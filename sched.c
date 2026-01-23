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
#define TOT  3000
#define SER  12

char **bin;
long interval_ms = 1000;

void *cl(void *p){
  int fd,one,i,got,r,sent;
  char ser[SER+1],*buf;
  unsigned long t,now;
  struct timeval tv;

  fd = *(int *)p;
  free(p);
  one = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

  got = 0;
  while (got < SER) {
    r = recv(fd, ser + got, SER - got, 0);
    if (r <= 0) {close(fd); return 0;}
    got += r;
  }
  ser[SER] = 0;
  printf("SER=%s\n", ser);

  gettimeofday(&tv, 0);
  t = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;

  i = 0;
  for (;;) {
    gettimeofday(&tv, 0);
    now = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;
    if (now < t) {usleep(1000); continue;}

    buf = bin[i%2];
    sent = 0;
    while (sent < LEN) {
      r = send(fd, buf + sent, LEN - sent, 0);
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
  pthread_t th;
  FILE *fp;
  char aux[100],x;

  bin=(char **)malloc(TOT*sizeof(char *));
  for(s=0; s<TOT; s++)bin[s]=(char *)malloc(LEN*sizeof(char));

  for(s=0;s<2;s++){
    sprintf(aux,"/root/%d.bin",s);
    fp=fopen(aux,"rb");
    fread(&x,1,1,fp);
    fread(bin[s],1,LEN,fp);
    fclose(fp);
  }

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

