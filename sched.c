// (<to> --> section with direct render ending with <to>
//   V <base> -->> Video reproduction set frame at position step+base
// [<to> --> section with write3 render ending with <to>
//   ! <stay> -->> set intervals of staying in ms for the section
//   @ <num> R <fmt> <from> <to> --> random generator
//   @ <num> C <fmt> {<statement>} --> formula in RPN
//   any "des" description processed by write3 with @<num>$ variables
//   reserved @0$ serial, @1$ IP, @2$ step, @3$ seq, @4$ hh, @5$ mm, @6$ ss, @7$ DEPOCH
//
// whois -h display.mazzini.org -p 5001 <passwd> <status>|<clear n>|<load from to>|<exit>
//
// Session policy:
//   One physical display serial may have only one active TCP session.
//   If a new connection announces an already active serial, the newest
//   connection wins and all older sessions for that serial are shut down.
//
// Logical state policy:
//   The physical TCP session is not the logical display state.
//   The display serial owns a monotonic step counter for the lifetime of
//   this server process. If a display reconnects with the same serial, even
//   from a different IP or TCP port, the new session resumes from the last
//   known step instead of restarting from zero.
//   epoch is stored internally as the Unix timestamp of the last logical
//   relaunch/reconnection.
//   DEPOCH is printed/exported as seconds elapsed since that epoch.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include "/home/tools/sched_passwd.h"

#define PORT 5000
#define MONITOR_PORT 5001
#define LEN  6144
#define TOT  5000
#define MAX_THREADS 100

#define MAX_SEQ_LINES 100
#define MAX_STEPS     10000
#define RPN_STACK_MAX 50
#define MAX_STATES    (MAX_THREADS * 4)

struct myThread {
  volatile int active;
  char ser[13];
  char ip[16];
  uint16_t port;
  volatile unsigned long step;
  time_t epoch;
  int fd;
  int8_t rssi;
  int mir;
  unsigned long t;
  uint64_t gen;
  char bin[LEN];
};

struct clientState {
  int used;
  char ser[13];
  unsigned long step;
  time_t epoch;
};

struct myThread mythr[MAX_THREADS];
static struct clientState mystate[MAX_STATES];

static char **bin;
static time_t server_startup_time = 0;

/*
 * session_gen is incremented for every accepted physical session.
 * It prevents an old thread from clearing a monitor slot that has
 * already been reused by a newer session.
 */
static uint64_t session_gen = 0;

pthread_mutex_t mon_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t bin_rwlock = PTHREAD_RWLOCK_INITIALIZER;

static int send_all(int fd, const void *buf, size_t len) {
  const char *p = (const char *)buf;
  size_t sent = 0;

  while (sent < len) {
    ssize_t r = send(fd, p + sent, len - sent, 0);
    if (r <= 0) {
      if (r < 0 && errno == EINTR) continue;
      return -1;
    }
    sent += (size_t)r;
  }

  return 0;
}

static int read_full(int fd, void *buf, size_t len) {
  char *p = (char *)buf;
  size_t got = 0;

  while (got < len) {
    ssize_t r = recv(fd, p + got, len - got, 0);
    if (r <= 0) {
      if (r < 0 && errno == EINTR) continue;
      return -1;
    }
    got += (size_t)r;
  }

  return 0;
}

/*
 * This helper must be called with mon_mutex held.
 * It only shuts the socket down. The owner thread will close it.
 * This avoids double-close and file-descriptor reuse hazards.
 */
static void shutdown_session_locked(int idx, const char *reason) {
  if (idx < 0 || idx >= MAX_THREADS) return;
  if (!mythr[idx].active) return;

  printf("Session shutdown: reason=%s idx=%d serial=%s peer=%s:%u step=%lu epoch=%ld rssi=%d gen=%llu\n",
         reason ? reason : "unknown",
         idx,
         mythr[idx].ser,
         mythr[idx].ip,
         (unsigned)mythr[idx].port,
         mythr[idx].step,
         (long)mythr[idx].epoch,
         (int)mythr[idx].rssi,
         (unsigned long long)mythr[idx].gen);
  fflush(stdout);

  if (mythr[idx].fd >= 0) {
    shutdown(mythr[idx].fd, SHUT_RDWR);
  }

  /*
   * Mark inactive immediately so the monitor no longer reports the old
   * session and so a new connection can claim a slot.
   *
   * The owner thread may still be running for a short time, but all of its
   * monitor updates are protected by the generation check.
   */
  mythr[idx].active = 0;
  mythr[idx].mir = -1;
  mythr[idx].t = 0;
}

/*
 * This helper must be called with mon_mutex held.
 */
static int session_is_current_locked(int idx, uint64_t gen) {
  if (idx < 0 || idx >= MAX_THREADS) return 0;
  return mythr[idx].active && mythr[idx].gen == gen;
}

/*
 * These helpers must be called with mon_mutex held.
 * mystate[] is the logical per-serial state.
 * mythr[] is only the physical TCP session state.
 */
static int state_find_locked(const char *ser) {
  int i;

  for (i = 0; i < MAX_STATES; i++) {
    if (mystate[i].used && strncmp(mystate[i].ser, ser, 12) == 0) {
      return i;
    }
  }

  return -1;
}

static int state_get_or_create_locked(const char *ser) {
  int i;

  i = state_find_locked(ser);
  if (i >= 0) return i;

  for (i = 0; i < MAX_STATES; i++) {
    if (!mystate[i].used) {
      mystate[i].used = 1;
      strncpy(mystate[i].ser, ser, 12);
      mystate[i].ser[12] = 0;
      mystate[i].step = 0;
      mystate[i].epoch = 0;
      return i;
    }
  }

  return -1;
}

static void update_step_if_current(int idx, uint64_t gen, unsigned long step) {
  int si;

  pthread_mutex_lock(&mon_mutex);

  if (session_is_current_locked(idx, gen)) {
    mythr[idx].step = step;

    si = state_get_or_create_locked(mythr[idx].ser);
    if (si >= 0 && step > mystate[si].step) {
      mystate[si].step = step;
    }
  }

  pthread_mutex_unlock(&mon_mutex);
}

static void update_rssi_if_current(int idx, uint64_t gen, int8_t rssi) {
  pthread_mutex_lock(&mon_mutex);
  if (session_is_current_locked(idx, gen)) {
    mythr[idx].rssi = rssi;
  }
  pthread_mutex_unlock(&mon_mutex);
}

static void update_frame_if_current(int idx, uint64_t gen, const char *frame, unsigned long t) {
  pthread_mutex_lock(&mon_mutex);
  if (session_is_current_locked(idx, gen)) {
    memcpy(mythr[idx].bin, frame, LEN);
    mythr[idx].t = t;
  }
  pthread_mutex_unlock(&mon_mutex);
}

int load_bin_range(int from, int to) {
  int i, loaded;
  char path[100], x;
  FILE *fp;

  loaded = 0;

  if (from < 0 || to > TOT - 2 || from > to) return 0;

  for (i = from; i <= to; i++) {
    sprintf(path, "/home/www/display/video/%05d.bin", i);
    fp = fopen(path, "rb");
    if (fp == NULL) continue;

    fread(&x, 1, 1, fp);

    pthread_rwlock_wrlock(&bin_rwlock);
    fread(bin[i], 1, LEN, fp);
    pthread_rwlock_unlock(&bin_rwlock);

    fclose(fp);
    loaded++;
  }

  return loaded;
}

void *whois_interface(void *arg) {
  int server_fd, client_fd, opt, n, i, len, target_idx, from, to, nloaded;
  struct sockaddr_in addr;
  char cmd_buf[256], resp[1024], aux[100], *pwd, *cmd, *arg_val, *arg_val2;
  time_t ora;
  long depoch;

  (void)arg;

  opt = 1;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) return NULL;

  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(MONITOR_PORT);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(server_fd);
    return NULL;
  }

  if (listen(server_fd, 5) < 0) {
    close(server_fd);
    return NULL;
  }

  for (;;) {
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) continue;

    memset(cmd_buf, 0, sizeof(cmd_buf));
    n = (int)recv(client_fd, cmd_buf, sizeof(cmd_buf) - 1, 0);

    if (n > 0) {
      cmd_buf[strcspn(cmd_buf, "\r\n")] = 0;

      pwd = strtok(cmd_buf, " ");
      cmd = strtok(NULL, " ");
      arg_val = strtok(NULL, " ");
      arg_val2 = strtok(NULL, " ");

      if (pwd && strcmp(pwd, MONITOR_PWD) == 0 && cmd) {
        if (strcmp(cmd, "status") == 0) {
          len = sprintf(resp, "STARTUP: %s", ctime(&server_startup_time));
          send_all(client_fd, resp, (size_t)len);

          ora = time(NULL);
          len = sprintf(resp,
                        "SNAPSHOT: %s%-3s | %12s | %-15s:%-5s | %12s | %12s | %5s\n",
                        ctime(&ora),
                        "IDX",
                        "SERIALE",
                        "CLIENT  IP",
                        "PORT",
                        "STEP",
                        "DEPOCH",
                        "RSSI");
          send_all(client_fd, resp, (size_t)len);

          pthread_mutex_lock(&mon_mutex);
          for (i = 0; i < MAX_THREADS; i++) {
            if (mythr[i].active) {
              if (mythr[i].mir == -1) {
                sprintf(aux, "%12lu", mythr[i].step);
              }
              else if (mythr[i].mir >= 0 && mythr[i].mir < MAX_THREADS) {
                strcpy(aux, mythr[mythr[i].mir].ser);
              }
              else {
                strcpy(aux, "INVALID-MIR");
              }

              depoch = 0;
              if (mythr[i].epoch > 0 && ora >= mythr[i].epoch) {
                depoch = (long)(ora - mythr[i].epoch);
              }

              len = sprintf(resp,
                            "%03d | %12s | %-15s:%-5u | %12s | %12ld | %5d\n",
                            i,
                            mythr[i].ser,
                            mythr[i].ip,
                            (unsigned)mythr[i].port,
                            aux,
                            depoch,
                            (int)mythr[i].rssi);
              send_all(client_fd, resp, (size_t)len);
            }
          }
          pthread_mutex_unlock(&mon_mutex);
        }

        else if (strcmp(cmd, "clear") == 0 && arg_val) {
          target_idx = atoi(arg_val);

          if (target_idx >= 0 && target_idx < MAX_THREADS) {
            pthread_mutex_lock(&mon_mutex);

            if (mythr[target_idx].active) {
              shutdown_session_locked(target_idx, "manual-clear");
              sprintf(resp, "OK: Thread %d shutdown requested.\n", target_idx);
            }
            else {
              sprintf(resp, "ERR: Thread %d inactive.\n", target_idx);
            }

            pthread_mutex_unlock(&mon_mutex);
            send_all(client_fd, resp, strlen(resp));
          }
        }

        else if (strcmp(cmd, "load") == 0 && arg_val && arg_val2) {
          from = atoi(arg_val);
          to = atoi(arg_val2);
          nloaded = load_bin_range(from, to);
          len = sprintf(resp, "OK: loaded %d frame(s) from %d to %d\n", nloaded, from, to);
          send_all(client_fd, resp, (size_t)len);
        }

        else if (strcmp(cmd, "exit") == 0) {
          send_all(client_fd, "Shutdown triggered.\n", 20);
          close(client_fd);
          exit(0);
        }

        else {
          send_all(client_fd, "Invalid command.\n", 17);
        }
      }
      else {
        send_all(client_fd, "Invalid request.\n", 17);
      }
    }

    close(client_fd);
  }

  return NULL;
}

static void *client(void *p) {
  int fd, one, r, sent, eseq, tot, ln, a0, a1, a2;
  int interval_ms, s, e, base_end, mm, my_idx, mir_idx, pstack, stack[RPN_STACK_MAX];
  int state_idx;
  int start_seq[MAX_STEPS], end_seq[MAX_STEPS], base_seq[MAX_SEQ_LINES];
  char *buf;
  char v[30][30], seq[MAX_SEQ_LINES][50], aux[100], *p1, *x;
  char fmt[20], cmd[300], xx, desfile[100], binfile[100];
  char peer_ip[16], mir_buf[LEN];
  unsigned long t, now, step, resume_step, mir_last;
  struct timeval tv;
  FILE *fp;
  struct timespec ts;
  struct tm tmv;
  uint64_t seed, my_gen;
  time_t my_epoch;
  long depoch;
  int8_t rssi;
  uint16_t peer_port;
  struct sockaddr_in peer;
  socklen_t peer_len = sizeof(peer);

  fd = *(int *)p;
  free(p);

  my_idx = -1;
  my_gen = 0;
  my_epoch = 0;
  depoch = 0;
  interval_ms = 1000;
  peer_port = 0;
  state_idx = -1;
  resume_step = 0;
  strcpy(peer_ip, "0.0.0.0");

  one = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

  if (getpeername(fd, (struct sockaddr *)&peer, &peer_len) == 0) {
    inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
    peer_port = ntohs(peer.sin_port);
  }

  /*
   * Initial 16-byte client header:
   *   bytes 0..11  : serial
   *   bytes 12..15 : client-side IP bytes as reported by the device
   */
  if (read_full(fd, aux, 16) < 0) {
    close(fd);
    return 0;
  }

  snprintf(v[0], sizeof(v[0]), "%.12s", aux);
  snprintf(v[1], sizeof(v[1]),
           "%u.%u.%u.%u",
           (uint8_t)aux[12],
           (uint8_t)aux[13],
           (uint8_t)aux[14],
           (uint8_t)aux[15]);

  /*
   * Register this connection in the monitor.
   *
   * Policy:
   *   A serial can have only one active physical session.
   *   The newest connection wins.
   *
   * Logical state:
   *   step is owned by the serial, not by the socket.
   *   epoch is the Unix timestamp of the latest relaunch/reconnection.
   *
   * Important:
   *   Older sessions are only shutdown() here. They are not close()d here.
   *   The owner thread will eventually observe send/recv failure and close
   *   its own fd. This avoids double-close and fd-reuse races.
   */
  pthread_mutex_lock(&mon_mutex);

  state_idx = state_get_or_create_locked(v[0]);
  if (state_idx >= 0) {
    resume_step = mystate[state_idx].step;
    my_epoch = time(NULL);
    mystate[state_idx].epoch = my_epoch;
  }
  else {
    resume_step = 0;
    my_epoch = time(NULL);
  }

  for (r = 0; r < MAX_THREADS; r++) {
    if (mythr[r].active && strncmp(mythr[r].ser, v[0], 12) == 0) {
      if (mythr[r].step > resume_step) {
        resume_step = mythr[r].step;
      }

      shutdown_session_locked(r, "serial-reconnect");
    }
  }

  if (state_idx >= 0 && resume_step > mystate[state_idx].step) {
    mystate[state_idx].step = resume_step;
  }

  my_gen = ++session_gen;

  for (r = 0; r < MAX_THREADS; r++) {
    if (!mythr[r].active) {
      mythr[r].active = 1;
      mythr[r].fd = fd;
      mythr[r].gen = my_gen;

      strncpy(mythr[r].ser, v[0], 12);
      mythr[r].ser[12] = 0;

      /*
       * Keep the device-reported IP in the monitor, as in the original code.
       * peer_ip remains available for logs if needed.
       */
      strncpy(mythr[r].ip, v[1], 15);
      mythr[r].ip[15] = 0;

      mythr[r].port = peer_port;
      mythr[r].step = resume_step;
      mythr[r].epoch = my_epoch;
      mythr[r].rssi = 0;
      mythr[r].mir = -1;
      mythr[r].t = 0;

      my_idx = r;

      printf("New session: idx=%d serial=%s device_ip=%s peer=%s:%u step=%lu epoch=%ld gen=%llu\n",
             my_idx,
             mythr[r].ser,
             mythr[r].ip,
             peer_ip,
             (unsigned)peer_port,
             mythr[r].step,
             (long)mythr[r].epoch,
             (unsigned long long)my_gen);
      fflush(stdout);

      break;
    }
  }

  pthread_mutex_unlock(&mon_mutex);

  if (my_idx == -1) {
    printf("Too many sessions: rejecting serial=%s peer=%s:%u\n",
           v[0],
           peer_ip,
           (unsigned)peer_port);
    fflush(stdout);
    close(fd);
    return 0;
  }

  sprintf(aux, "/home/www/display/pgr/%s.mat", v[0]);
  fp = fopen(aux, "rt");
  if (fp == 0) goto cleanup;

  if (fgets(v[3], (int)sizeof(v[3]), fp) == 0) {
    fclose(fp);
    goto cleanup;
  }

  fclose(fp);

  r = (int)strlen(v[3]);
  if (r > 0 && v[3][r - 1] == '\n') v[3][r - 1] = '\0';

  /*
   * Mirroring mode.
   * If the .mat file contains a 12-char serial, this display mirrors another
   * display. The mirror target is still resolved by index, as in the original
   * implementation. If the target reconnects and moves to a new slot, this
   * mirror session will exit and the device can reconnect.
   */
  if (strlen(v[3]) == 12) {
    mir_last = ULONG_MAX;

    pthread_mutex_lock(&mon_mutex);

    for (mir_idx = 0; mir_idx < MAX_THREADS; mir_idx++) {
      if (mythr[mir_idx].active && strncmp(mythr[mir_idx].ser, v[3], 12) == 0) {
        break;
      }
    }

    if (mir_idx < MAX_THREADS && session_is_current_locked(my_idx, my_gen)) {
      mythr[my_idx].mir = mir_idx;
    }

    pthread_mutex_unlock(&mon_mutex);

    if (mir_idx == MAX_THREADS) goto cleanup;

    for (;;) {
      pthread_mutex_lock(&mon_mutex);

      if (!session_is_current_locked(my_idx, my_gen)) {
        pthread_mutex_unlock(&mon_mutex);
        goto cleanup;
      }

      if (!mythr[mir_idx].active || strncmp(mythr[mir_idx].ser, v[3], 12) != 0) {
        pthread_mutex_unlock(&mon_mutex);
        goto cleanup;
      }

      t = mythr[mir_idx].t;
      if (t != 0 && t != mir_last) {
        memcpy(mir_buf, mythr[mir_idx].bin, LEN);
      }

      pthread_mutex_unlock(&mon_mutex);

      if (t == 0 || t == mir_last) {
        usleep(1000);
        continue;
      }

      if (send_all(fd, mir_buf, LEN) < 0) goto cleanup;

      for (;;) {
        r = (int)recv(fd, &rssi, 1, MSG_DONTWAIT);
        if (r != 1) break;
        update_rssi_if_current(my_idx, my_gen, rssi);
      }

      mir_last = t;
    }
  }

  sprintf(aux, "/home/www/display/pgr/%s.seq", v[3]);
  fp = fopen(aux, "rt");
  if (fp == 0) goto cleanup;

  base_end = -1;
  tot = 0;
  eseq = 0;

  for (; eseq < MAX_SEQ_LINES; eseq++) {
    if (fgets(seq[eseq], (int)sizeof(seq[eseq]), fp) == 0) break;

    r = (int)strlen(seq[eseq]);
    if (r > 0 && seq[eseq][r - 1] == '\n') seq[eseq][r - 1] = '\0';

    if (seq[eseq][0] == '(' || seq[eseq][0] == '[') {
      for (x = seq[eseq] + 1; *x == ' '; x++) {}

      for (a0 = 0; *x != ' ' && *x != '\0'; x++) {
        if (!isdigit((unsigned char)*x)) break;
        a0 = a0 * 10 + (*x - '0');
      }

      base_seq[eseq] = tot;

      if (base_end >= 0) {
        for (r = base_end; r < tot; r++) end_seq[r] = eseq - 1;
      }

      base_end = tot;

      for (; tot <= a0 && tot < MAX_STEPS; tot++) {
        start_seq[tot] = eseq;
      }
    }
  }

  if (base_end >= 0) {
    for (r = base_end; r < tot; r++) end_seq[r] = eseq - 1;
  }

  fclose(fp);

  gettimeofday(&tv, 0);
  t = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;

  clock_gettime(CLOCK_REALTIME, &ts);
  seed = (uint64_t)ts.tv_sec ^ (uint64_t)ts.tv_nsec ^ (uint64_t)getpid() ^ my_gen;
  srand((unsigned)seed);

  sprintf(desfile, "/run/display/%s.des", v[0]);
  sprintf(binfile, "/run/display/%s.bin", v[0]);

  for (step = resume_step;;) {
    update_step_if_current(my_idx, my_gen, step);

    pthread_mutex_lock(&mon_mutex);
    if (!session_is_current_locked(my_idx, my_gen)) {
      pthread_mutex_unlock(&mon_mutex);
      goto cleanup;
    }
    pthread_mutex_unlock(&mon_mutex);

    gettimeofday(&tv, 0);
    now = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;

    if (now < t) {
      usleep(1000);
      continue;
    }

    if (tot <= 0) goto cleanup;

    ln = (int)(step % (unsigned long)tot);
    s = start_seq[ln];
    e = end_seq[ln];

    buf = 0;

    if (seq[s][0] == '(') {
      for (r = s + 1; r <= e; r++) {
        if (seq[r][0] == 'V') {
          for (x = seq[r] + 1; *x == ' '; x++) {}

          for (a0 = 0; *x != ' ' && *x != '\0'; x++) {
            if (!isdigit((unsigned char)*x)) break;
            a0 = a0 * 10 + (*x - '0');
          }

          mm = ln - base_seq[s] + a0;

          if (mm >= 0 && mm < TOT) {
            buf = bin[mm];
          }

          interval_ms = 40;
        }
      }
    }

    if (seq[s][0] == '[') {
      fp = fopen(desfile, "wt");
      if (fp == 0) goto cleanup;

      clock_gettime(CLOCK_REALTIME, &ts);
      localtime_r(&ts.tv_sec, &tmv);

      sprintf(v[4], "%02d", tmv.tm_hour);
      sprintf(v[5], "%02d", tmv.tm_min);
      sprintf(v[6], "%02d", tmv.tm_sec);
      sprintf(v[2], "%lu", step);

      depoch = 0;
      if (my_epoch > 0 && ts.tv_sec >= my_epoch) {
        depoch = (long)(ts.tv_sec - my_epoch);
      }
      sprintf(v[7], "%ld", depoch);

      for (r = s + 1; r <= e; r++) {
        if (seq[r][0] == '!') {
          for (x = seq[r] + 1; *x == ' '; x++) {}

          for (a0 = 0; *x != ' ' && *x != '\0'; x++) {
            if (!isdigit((unsigned char)*x)) break;
            a0 = a0 * 10 + (*x - '0');
          }

          if (a0 > 0) interval_ms = a0;
          continue;
        }

        if (seq[r][0] == '@') {
          for (x = seq[r] + 1; *x == ' '; x++) {}

          for (a0 = 0; *x != ' ' && *x != '\0'; x++) {
            if (!isdigit((unsigned char)*x)) break;
            a0 = a0 * 10 + (*x - '0');
          }

          for (; *x == ' '; x++) {}

          if (*x == 'R') {
            for (x++; *x == ' '; x++) {}

            p1 = x;
            for (; *x != ' ' && *x != '\0'; x++) {}

            if (x > p1) {
              size_t flen = (size_t)(x - p1);
              if (flen >= sizeof(fmt)) flen = sizeof(fmt) - 1;
              strncpy(fmt, p1, flen);
              fmt[flen] = '\0';
            }
            else {
              fmt[0] = '\0';
            }

            for (; *x == ' '; x++) {}

            for (a1 = 0; *x != ' ' && *x != '\0'; x++) {
              if (!isdigit((unsigned char)*x)) break;
              a1 = a1 * 10 + (*x - '0');
            }

            for (; *x == ' '; x++) {}

            for (a2 = 0; *x != ' ' && *x != '\0'; x++) {
              if (!isdigit((unsigned char)*x)) break;
              a2 = a2 * 10 + (*x - '0');
            }

            if (a0 >= 0 && a0 < 30 && a2 >= a1 && fmt[0] != '\0') {
              sprintf(v[a0], fmt, a1 + rand() % (a2 - a1 + 1));
            }

            continue;
          }

          if (*x == 'C') {
            for (x++; *x == ' '; x++) {}

            p1 = x;
            for (; *x != ' ' && *x != '\0'; x++) {}

            if (x > p1) {
              size_t flen = (size_t)(x - p1);
              if (flen >= sizeof(fmt)) flen = sizeof(fmt) - 1;
              strncpy(fmt, p1, flen);
              fmt[flen] = '\0';
            }
            else {
              fmt[0] = '\0';
            }

            pstack = 0;

            for (; *x != '\0'; x++) {
              for (; *x == ' '; x++) {}

              if (*x == '\0') break;

              if (*x == '@') {
                x++;
                for (a1 = 0; *x != '$' && *x != '\0'; x++) {
                  if (!isdigit((unsigned char)*x)) break;
                  a1 = a1 * 10 + (*x - '0');
                }

                if (a1 >= 0 && a1 < 30 && pstack < RPN_STACK_MAX) {
                  stack[pstack++] = atoi(v[a1]);
                }

                continue;
              }

              if (isdigit((unsigned char)*x)) {
                for (a1 = 0; isdigit((unsigned char)*x); x++) {
                  a1 = a1 * 10 + (*x - '0');
                }

                if (pstack < RPN_STACK_MAX) {
                  stack[pstack++] = a1;
                }

                if (*x == '\0') break;
              }

              if (*x == '+') {
                if (pstack >= 2) {
                  stack[pstack - 2] = stack[pstack - 2] + stack[pstack - 1];
                  pstack--;
                }
                continue;
              }

              if (*x == '-') {
                if (pstack >= 2) {
                  stack[pstack - 2] = stack[pstack - 2] - stack[pstack - 1];
                  pstack--;
                }
                continue;
              }

              if (*x == '/') {
                if (pstack >= 2) {
                  if (stack[pstack - 1] != 0) {
                    stack[pstack - 2] = stack[pstack - 2] / stack[pstack - 1];
                  }
                  pstack--;
                }
                continue;
              }

              if (*x == '*') {
                if (pstack >= 2) {
                  stack[pstack - 2] = stack[pstack - 2] * stack[pstack - 1];
                  pstack--;
                }
                continue;
              }
            }

            if (a0 >= 0 && a0 < 30 && fmt[0] != '\0' && pstack > 0) {
              sprintf(v[a0], fmt, stack[pstack - 1]);
            }

            continue;
          }

          continue;
        }

        for (x = seq[r]; *x != '\0'; x++) {
          if (*x == '@') {
            for (a0 = 0, x++; *x != '$' && *x != '\0'; x++) {
              if (!isdigit((unsigned char)*x)) break;
              a0 = a0 * 10 + (*x - '0');
            }

            if (a0 >= 0 && a0 < 30) {
              fprintf(fp, "%s", v[a0]);
            }

            if (*x == '\0') break;
            continue;
          }

          fputc(*x, fp);
        }

        fputc('\n', fp);
      }

      fclose(fp);

      sprintf(cmd,
              "/home/www/display/write3 /run/display/%s.des /run/display/%s.ff /run/display/%s.bin",
              v[0],
              v[0],
              v[0]);
      system(cmd);

      fp = fopen(binfile, "rb");
      if (fp == 0) goto cleanup;

      fread(&xx, 1, 1, fp);
      fread(bin[TOT - 1], 1, LEN, fp);
      fclose(fp);

      buf = bin[TOT - 1];
    }

    if (buf == 0) goto cleanup;

    pthread_rwlock_rdlock(&bin_rwlock);
    sent = send_all(fd, buf, LEN);
    pthread_rwlock_unlock(&bin_rwlock);

    if (sent < 0) goto cleanup;

    update_frame_if_current(my_idx, my_gen, buf, now);

    for (;;) {
      r = (int)recv(fd, &rssi, 1, MSG_DONTWAIT);
      if (r != 1) break;
      update_rssi_if_current(my_idx, my_gen, rssi);
    }

    step++;
    t += (unsigned long)interval_ms;
  }

cleanup:
  /*
   * Only the owner of the current generation may clear the slot.
   * If this thread was replaced by a newer connection, its generation
   * no longer matches and the newer slot is left untouched.
   *
   * The logical per-serial state in mystate[] is intentionally not cleared.
   * It keeps step and epoch for the next reconnection.
   */
  if (my_idx != -1) {
    pthread_mutex_lock(&mon_mutex);

    if (session_is_current_locked(my_idx, my_gen)) {
      mythr[my_idx].active = 0;
      mythr[my_idx].fd = -1;
      mythr[my_idx].mir = -1;
      mythr[my_idx].t = 0;

      printf("Session ended: idx=%d serial=%s step=%lu epoch=%ld gen=%llu\n",
             my_idx,
             mythr[my_idx].ser,
             mythr[my_idx].step,
             (long)mythr[my_idx].epoch,
             (unsigned long long)my_gen);
      fflush(stdout);
    }

    pthread_mutex_unlock(&mon_mutex);
  }

  close(fd);
  return 0;
}

int main() {
  int server_fd, client_fd, *p_fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);
  pthread_t tid, monitor_tid;
  int opt = 1;

  signal(SIGPIPE, SIG_IGN);
  server_startup_time = time(NULL);

  bin = (char **)malloc(TOT * sizeof(char *));
  if (bin == NULL) {
    perror("malloc bin");
    return 1;
  }

  for (int i = 0; i < TOT; i++) {
    bin[i] = (char *)malloc(LEN);
    if (bin[i] == NULL) {
      perror("malloc bin frame");
      return 1;
    }
    memset(bin[i], 0, LEN);
  }

  load_bin_range(0, TOT - 2);

  if (pthread_create(&monitor_tid, NULL, whois_interface, NULL) != 0) {
    perror("Failed to create monitor thread");
    return 1;
  }

  pthread_detach(monitor_tid);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("Socket failed");
    return 1;
  }

  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, 10) < 0) {
    perror("Listen failed");
    close(server_fd);
    return 1;
  }

  pthread_mutex_lock(&mon_mutex);

  for (int i = 0; i < MAX_THREADS; i++) {
    mythr[i].active = 0;
    mythr[i].ser[0] = 0;
    mythr[i].ip[0] = 0;
    mythr[i].port = 0;
    mythr[i].step = 0;
    mythr[i].epoch = 0;
    mythr[i].fd = -1;
    mythr[i].rssi = 0;
    mythr[i].mir = -1;
    mythr[i].t = 0;
    mythr[i].gen = 0;
  }

  for (int i = 0; i < MAX_STATES; i++) {
    mystate[i].used = 0;
    mystate[i].ser[0] = 0;
    mystate[i].step = 0;
    mystate[i].epoch = 0;
  }

  pthread_mutex_unlock(&mon_mutex);

  printf("Server started. Port: %d, Whois Monitor: %d\n", PORT, MONITOR_PORT);
  fflush(stdout);

  for (;;) {
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
      if (errno == EINTR) continue;
      perror("Accept failed");
      continue;
    }

    p_fd = malloc(sizeof(int));
    if (p_fd == NULL) {
      perror("malloc client fd");
      close(client_fd);
      continue;
    }

    *p_fd = client_fd;

    if (pthread_create(&tid, NULL, client, p_fd) != 0) {
      perror("Thread creation failed");
      close(client_fd);
      free(p_fd);
    }
    else {
      pthread_detach(tid);
    }
  }

  return 0;
}
