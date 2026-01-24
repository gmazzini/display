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

// tot --> steps as first row
// (<from> <to> <base>) --> atomic video section range with format %06d.bin (TBD)
// [<fron> <to>] --> activation section range until other [] activation
// @<num> RAND <fmt> <from> <to> --> random generator
// !<num> -->> set intevals in ms
// any "des" description processed by write3 with @<num>$ variables
// reserved @0$ serial, @1$ IP (TBD), @2$ step, @3$ seq, @4$ hh, @5$ mm, @6$ ss

#define PORT 5000
#define LEN  6144
#define TOT  3000
#define SER  12

char **bin;

void *client(void *p){
  int fd,one,got,r,sent,eseq,tot,ln,go,a0,a1,a2,interval_ms;
  char *buf,v[30][30],seq[100][50],aux[100],*p1,*x,fmt[20],cmd[300],xx;
  unsigned long t,now,i;
  struct timeval tv;
  FILE *fp;
  struct timespec ts;
  struct tm tmv;
  uint64_t seed;
  
  interval_ms=1000;
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
  for(eseq=0;eseq<100;eseq++){
    fgets(seq[eseq],50,fp); r=strlen(seq[eseq]); seq[eseq][r-1]='\0';
    if(feof(fp))break;
  }
  fclose(fp);
  tot=atoi(seq[0]);
  
  printf("SER=%s SEQ=%s ESEQ=%d\n",v[0],v[3],eseq);
  gettimeofday(&tv,0);
  t=tv.tv_sec*1000UL+tv.tv_usec/1000UL;

  clock_gettime(CLOCK_REALTIME,&ts);
  seed=(uint64_t)ts.tv_sec ^ (uint64_t)ts.tv_nsec ^ (uint64_t)getpid();
  srand((unsigned)seed);

  for(i=0;;){
    gettimeofday(&tv,0);
    now=tv.tv_sec*1000UL+tv.tv_usec/1000UL;
    if(now<t){usleep(1000); continue;}

    clock_gettime(CLOCK_REALTIME,&ts);
    localtime_r(&ts.tv_sec,&tmv);
    sprintf(v[4],"%02d",tmv.tm_hour);
    sprintf(v[5],"%02d",tmv.tm_min);
    sprintf(v[6],"%02d",tmv.tm_sec);
    sprintf(v[2],"%lu",i);

    go=0; ln=i%tot;
    sprintf(aux,"/run/display/%s.des",v[0]);
    fp=fopen(aux,"wt");
    for(r=0;r<eseq;r++){
      if(go==0 && seq[r][0]=='('){
        for(x=seq[r]+1;*x==' ';x++);
        for(a0=0;*x!=' ';x++)a0=a0*10+(*x-'0');
        for(;*x==' ';x++);
        for(a1=0;*x!=' ';x++)a1=a1*10+(*x-'0');
        for(;*x==' ';x++);
        for(a2=0;*x!=' ' && *x!=')';x++)a2=a2*10+(*x-'0');
        if(ln>=a0 && ln<=a1){
          buf=bin(ln+a2);
          interval_ms=40;
          goto video;
        }
        continue;
      }
      if(go==0 && seq[r][0]=='['){
        for(x=seq[r]+1;*x==' ';x++);
        for(a0=0;*x!=' ';x++)a0=a0*10+(*x-'0');
        for(;*x==' ';x++);
        for(a1=0;*x!=' ' && *x!=']';x++)a1=a1*10+(*x-'0');
        if(ln>=a0 && ln<=a1)go=1;
        continue;
      }
      if(go==1 && seq[r][0]=='[')break;
      if(go==0)continue;
      if(seq[r][0]=='!'){
        for(a0=0,x=seq[r]+1;*x!=' ' && *x!='\0';x++)a0=a0*10+(*x-'0');
        interval_ms=a0;
        continue;
      }
      if(seq[r][0]=='@'){
        for(a0=0,x=seq[r]+1;*x!=' ';x++)a0=a0*10+(*x-'0');
        for(;*x==' ';x++);
        p1=x;
        if(strncmp(p1,"RAND",4)==0){
          for(x+=4;*x==' ';x++);
          p1=x; for(x++;*x!=' ';x++); strncpy(fmt,p1,x-p1); fmt[x-p1]='\0';
          for(;*x==' ';x++);
          for(a1=0;*x!=' ';x++)a1=a1*10+(*x-'0');
          for(;*x==' ';x++);
          for(a2=0;*x!=' ' && *x!='\0';x++)a2=a2*10+(*x-'0');
          sprintf(v[a0],fmt,a1+rand()%(a2-a1+1));
        }    
        continue;
      }
      for(x=seq[r];*x!='\0';x++){
        if(*x=='@'){
          for(a0=0,x++;*x!='$';x++)a0=a0*10+(*x-'0');
          fprintf(fp,"%s",v[a0]);
          continue;
        }
        fprintf(fp,"%c",*x);
      }
      fprintf(fp,"\n");
    }
    fclose(fp);

    sprintf(cmd,"/home/www/display/write3 /run/display/%s.des /run/display/%s.ff /run/display/%s.bin",v[0],v[0],v[0]);
    system(cmd);
    sprintf(aux,"/run/display/%s.bin",v[0]);
    fp=fopen(aux,"rb");
    fread(&xx,1,1,fp);
    fread(bin[2999],1,LEN,fp);
    fclose(fp);
    buf=bin[2999];
    
video:    
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

