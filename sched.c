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

#define PORT 5000
#define LEN  6144

static unsigned char f1[LEN];
static unsigned char f2[LEN];
static unsigned long interval_ms = 1000;

void *cl(void *p){
  int fd,one,i,got,r,sent;
  char ser[13],*buf;
  unsigned long t,now;
  struct timeval tv;

  fd = *(int *)p;
  free(p);
  one = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

  got = 0;
  while (got < SER) {
    r = recv(fd, ser + got, SER - got, 0);
    if (r <= 0) {close(fd); return ;}
    got += r;
  }
  ser[SER] = 0;
  printf("SER=%s\n", ser);

  gettimeofday(&tv, 0);
  t = (unsigned long)(tv.tv_sec * 1000UL + tv.tv_usec / 1000UL);

  i = 0;
  for (;;) {
    gettimeofday(&tv, 0);
    now = (unsigned long)(tv.tv_sec * 1000UL + tv.tv_usec / 1000UL);
    if ((long)(now - t) < 0) {usleep(1000); continue;}

    buf = (i & 1) ? f2 : f1;
    
    sent = 0;
    while (sent < LEN) {
      r = send(fd, (const char *)buf + sent, LEN - sent, 0);
      if (r <= 0) {close(fd); return 0;}
      sent += r;
    }

    i++;
    t += interval_ms;
  }
}

void main(){
  int s,one,c,*pc,prc;
  struct sockaddr_in a;
  size_t nread;
  pthread_t th;
  
  s = socket(AF_INET, SOCK_STREAM, 0);
  one = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_port = htons(PORT);
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(s, (struct sockaddr *)&a, sizeof(a));
  listen(s, 32);
  
  for (;;) {
    c = accept(s, 0, 0);
    if (c < 0) continue;
    pc = (int *)malloc(sizeof(int));
    if (pc == 0) {close(c); continue; }
    *pc = c;
    prc = pthread_create(&th, 0, cl, pc);
    if (prc != 0) {close(c); free(pc); continue;}
    pthread_detach(th);
  }
}

