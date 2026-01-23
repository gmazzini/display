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
long interval_ms = 5000;

void *client(void *p){
  int fd,one,got,r,sent,eseq,tot,ln,go,a0,a1,a2;
  char *buf,v[30][30],seq[100][50],aux[100],*p1,*p2,*q,*q1,*x,fmt[20];
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
  for(eseq=0;eseq<100;eseq++){
    fgets(seq[eseq],50,fp); r=strlen(seq[eseq]); seq[eseq][r-1]='\0';
    if(feof(fp))break;
  }
  fclose(fp);
  tot=atoi(seq[0]);
  
  printf("SER=%s SEQ=%s ESEQ=%d\n",v[0],v[3],eseq);
  gettimeofday(&tv,0);
  t=tv.tv_sec*1000UL+tv.tv_usec/1000UL;

  for(i=0;;){
    gettimeofday(&tv,0);
    now=tv.tv_sec*1000UL+tv.tv_usec/1000UL;
    if(now<t){usleep(1000); continue;}

    printf("%ld\n",i);

    go=0; ln=i%tot;
    sprintf(aux,"/run/display/%s.des",v[0]);
    fp=fopen(aux,"wt");
    for(r=0;r<eseq;r++){
      printf("r=%d\n",r);
      if(go==0 && seq[r][0]=='['){
        p1=strtok(seq[r]+1," ");
        p2=strtok(NULL," ]");
        if(ln>=atoi(p1) && ln<=atoi(p2))go=1;
        continue;
      }
      if(go==1 && seq[r][0]=='[')break;
      if(go==0)continue;
      if(seq[r][0]=='@'){
        q=strtok(seq[r]+1," "); a0=atoi(q);
        q=strtok(NULL," ");
        if(strcmp(q,"RAND")==0){
          q=strtok(NULL," "); strcpy(fmt,q);
          q=strtok(NULL," "); a1=atoi(q);
          q=strtok(NULL," \n"); a2=atoi(q);
          sprintf(v[a0],fmt,a1+rand()%(a2-a1+1));
        }
        continue;
      }
      for(x=seq[r];*x!='\0';x++){
        if(*x=='@'){
          q1=x+1; for(x=q1;*x!='$';x++); *x='\0';
          fprintf(fp,"%s",v[atoi(q1)]);
          continue;
        }
        fprintf(fp,"%c",*x);
      }
    }
    fclose(fp);

    
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

