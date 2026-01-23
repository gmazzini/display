/* gcc -std=c89 -O2 -pthread server.c -o server */
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
#define SER 12
#define LEN 6144

static unsigned char f1[LEN], f2[LEN];
static unsigned long interval_ms = 40;

static int ra(int fd, void *b, int n){ int g=0,r; while(g<n){ r=recv(fd,(char*)b+g,n-g,0); if(r<=0) return r; g+=r; } return g; }
static int sa(int fd, const void *b, int n){ int s=0,r; while(s<n){ r=send(fd,(char*)b+s,n-s,0); if(r<=0) return r; s+=r; } return s; }
static unsigned long ms(void){ struct timeval tv; gettimeofday(&tv,0); return (unsigned long)(tv.tv_sec*1000UL + tv.tv_usec/1000UL); }

static void *cl(void *p){
  int fd=*(int*)p, one=1, i=0; unsigned char ser[SER+1]; unsigned long t=ms();
  free(p);
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
  ra(fd, ser, SER); ser[SER]=0; printf("SER=%s\n", ser);
  for(;;){
    unsigned long now=ms();
    if((long)(now-t)>=0){
      sa(fd, (i&1)?f2:f1, LEN);
      i++; t += interval_ms;
    } else usleep(1000);
  }
  return 0;
}
int main(int argc, char **argv){
  int s, one=1; struct sockaddr_in a;
  FILE *f; unsigned char tmp[1+LEN];

  if(argc>1) interval_ms=(unsigned long)atoi(argv[1]);

  f=fopen("1.bin","rb"); fread(tmp,1,1+LEN,f); fclose(f); memcpy(f1,tmp+1,LEN);
  f=fopen("2.bin","rb"); fread(tmp,1,1+LEN,f); fclose(f); memcpy(f2,tmp+1,LEN);

  s=socket(AF_INET,SOCK_STREAM,0);
  setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(one));
  memset(&a,0,sizeof(a)); a.sin_family=AF_INET; a.sin_port=htons(PORT); a.sin_addr.s_addr=htonl(INADDR_ANY);
  bind(s,(struct sockaddr*)&a,sizeof(a));
  listen(s,32);
  printf("listening :%d interval=%lums\n", PORT, interval_ms);

  for(;;){
    int c=accept(s,0,0), *pc=(int*)malloc(sizeof(int)); pthread_t th;
    *pc=c; pthread_create(&th,0,cl,pc); pthread_detach(th);
  }
  return 0;
}
