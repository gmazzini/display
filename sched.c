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
long interval_ms = 40;

void *client(void *p){
  int fd,one,got,r,sent,eseq;
  char *buf,v[30][30],seq[50][30],aux[100];
  unsigned long t,now;
  struct timeval tv;
  FILE *fp;
  unsigned long long i;

  fd=*(int *)p;
  free(p);
  one=1;
  setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&one,sizeof(one));
  for(got=0;got<SER;got+=r){
    r=recv(fd,v[0]+got,SER-got,0);
    if(r<=0){close(fd); return 0;}
  }
  v[0][SER]=0;
  sprintf(aux,"/run/display/%s.mat",v[0]);
  fp=fopen(aux,"rt");
  fgets(v[3],30,fp); r=strlen(v[3]); v[3][r-1]='\0';
  fclose(fp);
  sprintf(aux,"/run/display/%s.seq",v[3]);
  fp=fopen(aux,"rt");
  for(eseq=0;eseq<50;eseq++){
    fgets(seq[eseq],30,fp);
    if(feof(fp))break;
  }
  fclose(fp);
  
  printf("SER=%s SEQ=%s ESEQ=%d\n",v[0],v[3],eeq);
  gettimeofday(&tv,0);
  t=tv.tv_sec*1000UL+tv.tv_usec/1000UL;

  for(i=0;;){
    gettimeofday(&tv,0);
    now=tv.tv_sec*1000UL+tv.tv_usec/1000UL;
    if(now<t){usleep(1000); continue;}

    buf=bin[i%2445];
    
    for(sent=0;sent<LEN;sent+=r){
      r=send(fd,buf+sent,LEN-sent,0);
      if(r<=0){close(fd); return 0;}
    }

    i++;
    t+=interval_ms;
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

  for(s=0;s<2445;s++){
    sprintf(aux,"/root/prova2/%04d.bin",s+1);
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
    prc = pthread_create(&th, 0, client, pc);
    if (prc != 0) {close(c); free(pc); continue;}
    pthread_detach(th);
  }
}

