// (<to> --> section with direct render ending with <to>
//   V <base> -->> Video reproduction set frame at position step+base
// [<to> --> section with write3 render ending with <to>
//   ! <stay> -->> set intevals of staying in ms for the section
//   @ <num> R <fmt> <from> <to> --> random generator
//   any "des" description processed by write3 with @<num>$ variables
//   reserved @0$ serial, @1$ IP (TBD), @2$ step, @3$ seq, @4$ hh, @5$ mm, @6$ ss

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

#define PORT 5000
#define LEN  6144
#define TOT  5000
#define MAX_THREADS 100

// Struttura per il monitoraggio esterno
typedef struct {
    volatile int active;
    char ser[16];
    char ip[16];
    volatile unsigned long step;
} ThreadMonitor;

static char **bin;
ThreadMonitor monitor[MAX_THREADS];
pthread_mutex_t mon_mutex = PTHREAD_MUTEX_INITIALIZER;

// Gestore del segnale per stampare lo stato (kill -USR1 PID)
void signal_print_status(int sig) {
    int i;
    printf("\n--- STATO THREAD ATTIVI ---\n");
    printf("%-3s | %-12s | %-15s | %-10s\n", "IDX", "SERIALE", "IP CLIENT", "STEP");
    pthread_mutex_lock(&mon_mutex);
    for(i = 0; i < MAX_THREADS; i++) {
        if(monitor[i].active) {
            printf("%03d | %-12s | %-15s | %lu\n", i, monitor[i].ser, monitor[i].ip, monitor[i].step);
        }
    }
    pthread_mutex_unlock(&mon_mutex);
    printf("---------------------------\n\n");
}

static void *client(void *p) {
  int fd, one, got, r, sent, eseq, tot, ln, a0, a1, a2, interval_ms, s, e, base_end, mm, my_idx;
  int start_seq[10000], end_seq[10000], base_seq[100];
  char *buf;
  char v[30][30];
  char seq[100][50];
  char aux[100];
  char *p1, *x;
  char fmt[20];
  char cmd[300];
  char xx;
  char desfile[100];
  char binfile[100];
  unsigned long t, now, step;
  struct timeval tv;
  FILE *fp;
  struct timespec ts;
  struct tm tmv;
  uint64_t seed;

  interval_ms = 1000;
  fd = *(int *)p;
  free(p);
  my_idx = -1;

  one = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

  for (got = 0; got < 16; got += r) {
    r = (int)recv(fd, aux + got, 16 - got, 0);
    if (r <= 0) { close(fd); return 0; }
  }

  snprintf(v[0], sizeof(v[0]), "%.12s", aux);
  snprintf(v[1], sizeof(v[1]), "%u.%u.%u.%u",
           (uint8_t)aux[12], (uint8_t)aux[13], (uint8_t)aux[14], (uint8_t)aux[15]);

  // Registrazione nel monitor
  pthread_mutex_lock(&mon_mutex);
  for(r = 0; r < MAX_THREADS; r++) {
      if(!monitor[r].active) {
          monitor[r].active = 1;
          strncpy(monitor[r].ser, v[0], 15);
          strncpy(monitor[r].ip, v[1], 15);
          monitor[r].step = 0;
          my_idx = r;
          break;
      }
  }
  pthread_mutex_unlock(&mon_mutex);

  snprintf(aux, sizeof(aux), "/home/www/display/pgr/%s.mat", v[0]);
  fp = fopen(aux, "rt");
  if (fp == 0) goto cleanup;
  if (fgets(v[3], (int)sizeof(v[3]), fp) == 0) { fclose(fp); goto cleanup; }
  fclose(fp);

  r = (int)strlen(v[3]);
  if (r > 0 && v[3][r - 1] == '\n') v[3][r - 1] = '\0';

  snprintf(aux, sizeof(aux), "/home/www/display/pgr/%s.seq", v[3]);
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

  printf("SER=%s IP=%s SEQ=%s ESEQ=%d tot=%d\n", v[0], v[1], v[3], eseq, tot);
  fflush(stdout);

  gettimeofday(&tv, 0);
  t = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;

  clock_gettime(CLOCK_REALTIME, &ts);
  seed = (uint64_t)ts.tv_sec ^ (uint64_t)ts.tv_nsec ^ (uint64_t)getpid();
  srand((unsigned)seed);

  snprintf(desfile, sizeof(desfile), "/run/display/%s.des", v[0]);
  snprintf(binfile, sizeof(binfile), "/run/display/%s.bin", v[0]);

  for (step = 0;;) {
    // Aggiornamento step nel monitor (veloce, senza mutex)
    if(my_idx != -1) monitor[my_idx].step = step;

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

      snprintf(v[4], sizeof(v[4]), "%02d", tmv.tm_hour);
      snprintf(v[5], sizeof(v[5]), "%02d", tmv.tm_min);
      snprintf(v[6], sizeof(v[6]), "%02d", tmv.tm_sec);
      snprintf(v[2], sizeof(v[2]), "%lu", step);

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
            } else {
              fmt[0] = '\0';
            }

            for (; *x == ' '; x++) {}
            for (a1 = 0; *x != ' ' && *x != '\0'; x++) a1 = a1 * 10 + (*x - '0');
            for (; *x == ' '; x++) {}
            for (a2 = 0; *x != ' ' && *x != '\0'; x++) a2 = a2 * 10 + (*x - '0');

            if (a0 >= 0 && a0 < 30 && a2 >= a1 && fmt[0] != '\0') {
              snprintf(v[a0], sizeof(v[a0]), fmt, a1 + rand() % (a2 - a1 + 1));
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

      snprintf(cmd, sizeof(cmd),
               "/home/www/display/write3 /run/display/%s.des /run/display/%s.ff /run/display/%s.bin",
               v[0], v[0], v[0]);
      system(cmd);

      fp = fopen(binfile, "rb");
      if (fp == 0) goto cleanup;
      fread(&xx, 1, 1, fp);
      fread(bin[TOT - 1], 1, LEN, fp);
      fclose(fp);

      buf = bin[TOT - 1];
    }

    if (buf == 0) goto cleanup;

    for (sent = 0; sent < LEN; sent += r) {
      r = (int)send(fd, buf + sent, LEN - sent, 0);
      if (r <= 0) goto cleanup;
    }

    step++;
    t += (unsigned long)interval_ms;
  }

cleanup:
  if(my_idx != -1) {
      pthread_mutex_lock(&mon_mutex);
      monitor[my_idx].active = 0;
      pthread_mutex_unlock(&mon_mutex);
  }
  close(fd);
  return 0;
}

int main(void) {
  int s, one, c, *pc, prc;
  struct sockaddr_in a;
  pthread_t th;
  FILE *fp;
  char aux[100];
  char x;

  signal(SIGPIPE, SIG_IGN);
  // Registra il comando kill -USR1 per la stampa
  signal(SIGUSR1, signal_print_status);

  memset(monitor, 0, sizeof(monitor));

  bin = (char **)malloc(TOT * sizeof(char *));
  if (bin == 0) return 1;

  for (s = 0; s < TOT; s++) {
    bin[s] = (char *)malloc(LEN * sizeof(char));
    if (bin[s] == 0) return 1;
  }

  for (s = 0; s < TOT; s++) {
    snprintf(aux, sizeof(aux), "/home/www/display/video/%05d.bin", s + 1);
    fp = fopen(aux, "rb");
    if (fp == 0) continue;
    fread(&x, 1, 1, fp);
    fread(bin[s], 1, LEN, fp);
    fclose(fp);
  }

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return 1;

  one = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));

  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_port = htons(PORT);
  a.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(s, (struct sockaddr *)&a, sizeof(a)) != 0) return 1;
  if (listen(s, 32) != 0) return 1;

  for (;;) {
    c = accept(s, 0, 0);
    if (c < 0) continue;

    pc = (int *)malloc(sizeof(int));
    if (pc == 0) { close(c); continue; }

    *pc = c;

    prc = pthread_create(&th, 0, client, pc);
    if (prc != 0) {
        free(pc);
        close(c);
    } else {
        pthread_detach(th);
    }
  }
}
