// (<to> --> section with direct render ending with <to>
//   V <base> -->> Video reproduction set frame at position step+base
// [<to> --> section with write3 render ending with <to>
//   ! <stay> -->> set intevals of staying in ms for the section
//   @ <num> R <fmt> <from> <to> --> random generator
//   @ <num> C <fmt> {<statement>} --> formula in RPN
//   any "des" description processed by write3 with @<num>$ variables
//   reserved @0$ serial, @1$ IP (TBD), @2$ step, @3$ seq, @4$ hh, @5$ mm, @6$ ss
// whois -h display.mazzini.org -p 5001 <passwd> <status>|<clear n>|<load from to>|<exit>

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
#define LEN  6144
#define TOT  5000
#define MAX_THREADS 100

// Struttura per i thread
struct myThread {
  volatile int active;
  char ser[13];
  char ip[16];
  uint16_t port;
  volatile unsigned long step;
  int fd;
  int8_t rssi;
  int mir;
  unsigned long t;
  char bin[LEN];
};

struct myThread mythr[MAX_THREADS];
static char **bin;
static time_t server_startup_time = 0;
pthread_mutex_t mon_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t bin_rwlock = PTHREAD_RWLOCK_INITIALIZER;

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

  opt = 1;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(5001);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return NULL;
  listen(server_fd, 5);

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
          send(client_fd, resp, (size_t)len, 0);
          ora = time(NULL);
          len = sprintf(resp, "SNAPSHOT: %s%-3s | %12s | %-15s:%-5s | %12s | %5s\n", ctime(&ora), "IDX", "SERIALE", "CLIENT  IP", "PORT", "STEP", "RSSI");
          send(client_fd, resp, (size_t)len, 0);
          pthread_mutex_lock(&mon_mutex);
          for (i = 0; i < MAX_THREADS; i++) {
            if (mythr[i].active) {
              if (strlen(mythr[i].mir) == -1) sprintf(aux, "%12lu", mythr[i].step);
              else strcpy(aux,mythr[mythr[i].mir].ser);
              len = sprintf(resp, "%03d | %12s | %-15s:%-5d | %12s | %5d\n", i, mythr[i].ser, mythr[i].ip, mythr[i].port, aux, (int)mythr[i].rssi);
              send(client_fd, resp, (size_t)len, 0);
            }
          }
          pthread_mutex_unlock(&mon_mutex);
        }
          
        else if (strcmp(cmd, "clear") == 0 && arg_val) {
          target_idx = atoi(arg_val);
          if (target_idx >= 0 && target_idx < MAX_THREADS) {
            pthread_mutex_lock(&mon_mutex);
            if (mythr[target_idx].active) {
              shutdown(mythr[target_idx].fd, SHUT_RDWR);
              close(mythr[target_idx].fd);
              sprintf(resp, "OK: Thread %d down.\n", target_idx);
            }
            else {
              sprintf(resp, "ERR: Thread %d inactive.\n", target_idx);
            }
            pthread_mutex_unlock(&mon_mutex);
            send(client_fd, resp, strlen(resp), 0);
          }
        }
        
        else if (strcmp(cmd, "load") == 0 && arg_val && arg_val2) {
          from = atoi(arg_val);
          to   = atoi(arg_val2);
          nloaded = load_bin_range(from, to);
          len = sprintf(resp, "OK: loaded %d frame(s) from %d to %d\n", nloaded, from, to);
          send(client_fd, resp, (size_t)len, 0);
        }
        
        else if (strcmp(cmd, "exit") == 0) {
          send(client_fd, "Shutdown triggered.\n", 20, 0);
          close(client_fd);
          exit(0);
        }
      }
      else {
        send(client_fd, "Invalid request.\n", 17, 0);
      }
    }
    close(client_fd);
  }
  return NULL;
}

static void *client(void *p) {
  int fd, one, got, r, sent, eseq, tot, ln, a0, a1, a2, interval_ms, s, e, base_end, mm, my_idx, mir_idx, pstack, stack[50];
  int start_seq[10000], end_seq[10000], base_seq[100];
  char *buf, v[30][30], seq[100][50], aux[100], *p1, *x, fmt[20], cmd[300], xx, desfile[100], binfile[100], peer_ip[16], mir_buf[LEN];
  unsigned long t, now, step, mir_last;
  struct timeval tv;
  FILE *fp;
  struct timespec ts;
  struct tm tmv;
  uint64_t seed;
  int8_t rssi;
  uint16_t peer_port;
  struct sockaddr_in peer;
  socklen_t peer_len = sizeof(peer);
 
  interval_ms = 1000;
  fd = *(int *)p;
  free(p);
  my_idx = -1;
  one = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

  if (getpeername(fd, (struct sockaddr *)&peer, &peer_len) == 0) {
    inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
    peer_port = ntohs(peer.sin_port);
  }

  for (got = 0; got < 16; got += r) {
    r = (int)recv(fd, aux + got, 16 - got, 0);
    if (r <= 0) { close(fd); return 0; }
  }

  snprintf(v[0], sizeof(v[0]), "%.12s", aux);
  snprintf(v[1], sizeof(v[1]), "%u.%u.%u.%u", (uint8_t)aux[12], (uint8_t)aux[13], (uint8_t)aux[14], (uint8_t)aux[15]);

  // Elementi the thread per monitor e mirror
  pthread_mutex_lock(&mon_mutex);
  for(r = 0; r < MAX_THREADS; r++) {
    if(!mythr[r].active) {
      mythr[r].active = 1;
      mythr[r].fd = fd;
      strncpy(mythr[r].ser, v[0], 12); mythr[r].ser[12] = 0;
      strncpy(mythr[r].ip, v[1], 15); mythr[r].ip[15] = 0;
      mythr[r].port = peer_port;
      mythr[r].step = 0;
      mythr[r].rssi = 0;
      mythr[r].mir = -1;
      mythr[r].t = 0;
      my_idx = r;
      break;
    }
  }
  pthread_mutex_unlock(&mon_mutex);

  sprintf(aux, "/home/www/display/pgr/%s.mat", v[0]);
  fp = fopen(aux, "rt");
  if (fp == 0) goto cleanup;
  if (fgets(v[3], (int)sizeof(v[3]), fp) == 0) { fclose(fp); goto cleanup; }
  fclose(fp);
  r = (int)strlen(v[3]);
  if (r > 0 && v[3][r - 1] == '\n') v[3][r - 1] = '\0';

  // mirroring
  if (strlen(v[3]) == 12) {
    mir_last = ULONG_MAX;
    pthread_mutex_lock(&mon_mutex);
    for (mir_idx = 0; mir_idx < MAX_THREADS; mir_idx++) {
      if (mythr[mir_idx].active && strncmp(mythr[mir_idx].ser, v[3], 12) == 0) break;
    }
    if (mir_idx < MAX_THREADS) mythr[r].mir = mir_idx;
    pthread_mutex_unlock(&mon_mutex);
    if (mir_idx == MAX_THREADS) goto cleanup;

    for (;;) {
      pthread_mutex_lock(&mon_mutex);
      if (!mythr[mir_idx].active || strncmp(mythr[mir_idx].ser, v[3], 12) != 0) { 
        pthread_mutex_unlock(&mon_mutex); 
        goto cleanup; 
      }
      t = mythr[mir_idx].t;
      if (t != 0 && t != mir_last) memcpy(mir_buf, mythr[mir_idx].bin, LEN);
      pthread_mutex_unlock(&mon_mutex);
      if (t == 0 || t == mir_last) { usleep(1000); continue; }
      for (sent = 0; sent < LEN; sent += r) {
        r = (int)send(fd, mir_buf + sent, LEN - sent, 0);
        if (r <= 0) goto cleanup;
      }
      for (;;) {
        r = (int)recv(fd, &rssi, 1, MSG_DONTWAIT);
        if (r != 1) break;
        if (my_idx != -1) mythr[my_idx].rssi = rssi;
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

  for (; eseq < 100; eseq++) {
    if (fgets(seq[eseq], (int)sizeof(seq[eseq]), fp) == 0) break;
    r = (int)strlen(seq[eseq]);
    if (r > 0 && seq[eseq][r - 1] == '\n') seq[eseq][r - 1] = '\0';

    if (seq[eseq][0] == '(' || seq[eseq][0] == '[') {
      for (x = seq[eseq] + 1; *x == ' '; x++) {}
      for (a0 = 0; *x != ' ' && *x != '\0'; x++) a0 = a0 * 10 + (*x - '0');
      base_seq[eseq] = tot;

      if (base_end >= 0) {
        for (r = base_end; r < tot; r++) end_seq[r] = eseq - 1;
      }

      base_end = tot;
      for (; tot <= a0 && tot < (int)(sizeof(start_seq)/sizeof(start_seq[0])); tot++) {
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
  seed = (uint64_t)ts.tv_sec ^ (uint64_t)ts.tv_nsec ^ (uint64_t)getpid();
  srand((unsigned)seed);

  sprintf(desfile, "/run/display/%s.des", v[0]);
  sprintf(binfile, "/run/display/%s.bin", v[0]);

  for (step = 0;;) {
    // Aggiornamento step nel monitor (veloce, senza mutex)
    if(my_idx != -1) mythr[my_idx].step = step;

    gettimeofday(&tv, 0);
    now = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;
    if (now < t) { usleep(1000); continue; }

    if (tot <= 0) goto cleanup;

    ln = (int)(step % (unsigned long)tot);
    s = start_seq[ln];
    e = end_seq[ln];

    buf = 0;

    if (seq[s][0] == '(') {
      for (r = s + 1; r <= e; r++) {
        if (seq[r][0] == 'V') {
          for (x = seq[r] + 1; *x == ' '; x++) {}
          for (a0 = 0; *x != ' ' && *x != '\0'; x++) a0 = a0 * 10 + (*x - '0');

          mm = ln - base_seq[s] + a0;
          if (mm >= 0 && mm < TOT) buf = bin[mm];
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

      for (r = s + 1; r <= e; r++) {
        if (seq[r][0] == '!') {
          for (x = seq[r] + 1; *x == ' '; x++) {}
          for (a0 = 0; *x != ' ' && *x != '\0'; x++) a0 = a0 * 10 + (*x - '0');
          interval_ms = a0;
          continue;
        }

        if (seq[r][0] == '@') {
          for (x = seq[r] + 1; *x == ' '; x++) {}
          for (a0 = 0; *x != ' '; x++) a0 = a0 * 10 + (*x - '0');
          for (; *x == ' '; x++) {}

          if (*x == 'R') {
            for (x++; *x == ' '; x++) {}
            p1 = x;
            for (; *x != ' ' && *x != '\0'; x++) {}
            if (x > p1) {
              strncpy(fmt, p1, (size_t)(x - p1));
              fmt[x - p1] = '\0';
            } 
            else fmt[0] = '\0';
            for (; *x == ' '; x++) {}
            for (a1 = 0; *x != ' ' && *x != '\0'; x++) a1 = a1 * 10 + (*x - '0');
            for (; *x == ' '; x++) {}
            for (a2 = 0; *x != ' ' && *x != '\0'; x++) a2 = a2 * 10 + (*x - '0');

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
              strncpy(fmt, p1, (size_t)(x - p1));
              fmt[x - p1] = '\0';
            } 
            else fmt[0] = '\0';

            for (pstack=0;*x != '\0'; x++) {
              for (; *x == ' '; x++) {}
              if (*x == '@') {
                for (a1 = 0, x++; *x != '$' && *x != '\0'; x++) a1 = a1 * 10 + (*x - '0');
                if (a1 >= 0 && a1 < 30) stack[pstack++] = atoi(v[a1]);
                continue;
              }
              if (isdigit(*x)) {
                for (a1 = 0; *x != '$' && *x != '\0'; x++) a1 = a1 * 10 + (*x - '0');
                stack[pstack++] = atoi(v[a1]);
                continue;
              }
              if (*x == '+') {
                if (pstack >= 2) {
                  stack[pstack-2] = stack[pstack-2] + stack[pstack-1];
                  pstack--;
                }
                continue;
              }
              if (*x == '-') {
                if (pstack >= 2) {
                  stack[pstack-2] = stack[pstack-2] - stack[pstack-1];
                  pstack--;
                }
                continue;
              }
              if (*x == '/') {
                if (pstack >= 2) {
                  stack[pstack-2] = stack[pstack-2] / stack[pstack-1];
                  pstack--;
                }
                continue;
              }
              if (*x == '*') {
                if (pstack >= 2) {
                  stack[pstack-2] = stack[pstack-2] * stack[pstack-1];
                  pstack--;
                }
                continue;
              }
            }
            if (a0 >= 0 && a0 < 30 && fmt[0] != '\0' && pstack > 0) {
              sprintf(v[a0], fmt, stack[pstack-1]);
            }
            continue;
          }

          continue;
        }

        for (x = seq[r]; *x != '\0'; x++) {
          if (*x == '@') {
            for (a0 = 0, x++; *x != '$' && *x != '\0'; x++) a0 = a0 * 10 + (*x - '0');
            if (a0 >= 0 && a0 < 30) fprintf(fp, "%s", v[a0]);
            continue;
          }
          fputc(*x, fp);
        }
        fputc('\n', fp);
      }

      fclose(fp);

      sprintf(cmd, "/home/www/display/write3 /run/display/%s.des /run/display/%s.ff /run/display/%s.bin", v[0], v[0], v[0]);
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
    for (sent = 0; sent < LEN; sent += r) {
      r = (int)send(fd, buf + sent, LEN - sent, 0);
      if (r <= 0) {
        pthread_rwlock_unlock(&bin_rwlock);
        goto cleanup;
      }
    }
    pthread_rwlock_unlock(&bin_rwlock);
    pthread_mutex_lock(&mon_mutex);
    memcpy(mythr[my_idx].bin, buf, LEN);
    mythr[my_idx].t = now;
    pthread_mutex_unlock(&mon_mutex);

    for (;;) {
      r = (int)recv(fd, &rssi, 1, MSG_DONTWAIT);
      if (r != 1) break;
      if (my_idx != -1) mythr[my_idx].rssi = rssi;
    }
    
    step++;
    t += (unsigned long)interval_ms;
  }
cleanup:
  if(my_idx != -1) {
      pthread_mutex_lock(&mon_mutex);
      mythr[my_idx].active = 0;
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

  // Alloca memoria per i buffer video (come nel codice originale)
  bin = (char **)malloc(TOT * sizeof(char *));
  for (int i = 0; i < TOT; i++) {
    bin[i] = (char *)malloc(LEN);
    memset(bin[i], 0, LEN);
  }
  load_bin_range(0, TOT - 2);

  // Avvia il thread dell'interfaccia Whois (Porta 5001)
  if (pthread_create(&monitor_tid, NULL, whois_interface, NULL) != 0) {
    perror("Failed to create monitor thread");
    return 1;
  }
  pthread_detach(monitor_tid);

  // Setup del Server principale (Porta 5000)
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) { perror("Socket failed"); return 1; }
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    return 1;
  }

  if (listen(server_fd, 10) < 0) {
    perror("Listen failed");
    return 1;
  }

  // Inizializzazione monitor
  pthread_mutex_lock(&mon_mutex);
  for (int i = 0; i < MAX_THREADS; i++) mythr[i].active = 0;
  pthread_mutex_unlock(&mon_mutex);

  printf("Server started. Port: %d, Whois Monitor: 5001\n", PORT);

  // Ciclo principale accettazione client
  for (;;) {
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
      if (errno == EINTR) continue;
      perror("Accept failed");
      continue;
    }

    p_fd = malloc(sizeof(int));
    *p_fd = client_fd;

    if (pthread_create(&tid, NULL, client, p_fd) != 0) {
      perror("Thread creation failed");
      close(client_fd);
      free(p_fd);
    } 
    else pthread_detach(tid);
  }

  return 0;
}
