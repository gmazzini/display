#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// v[0]=n v[1]=hh v[2]=mm v[3]=ss
void main(int argc,char *argv[]){
  FILE *fp,*fp1;
  char buf[100],seq[100],*p1,*p2,*q1,*q2,*q3,*q4,*x;
  int l,tot,ln,go,min,max,i;
  long v[100];
  time_t now;
  struct tm tmv;
  struct timeval tv;
  
  gettimeofday(&tv,NULL);
  srand((unsigned)(tv.tv_sec ^ tv.tv_usec ^ getpid()));
  now=time(NULL);
  localtime_r(&now,&tmv);
  v[1]=tmv.tm_hour;
  v[2]=tmv.tm_min;
  v[3]=tmv.tm_sec;

  sprintf(buf,"/run/display/%s.step",argv[1]);
  fp=fopen(buf,"rt");
  fgets(buf,100,fp);
  v[0]=atol(buf);
  fclose(fp);
  sprintf(buf,"/home/www/display/pgr/%s.mat",argv[1]);
  fp=fopen(buf,"rt");
  fgets(seq,100,fp);
  l=strlen(seq); seq[l-1]='\0';
  fclose(fp);

  sprintf(buf,"/run/display/%s.des",argv[1]);
  fp1=fopen(buf,"wt");
  sprintf(buf,"/home/www/display/pgr/%s.seq",seq);
  fp=fopen(buf,"rt");
  fgets(buf,100,fp);
  tot=atoi(buf);
  ln=v[0]%tot;
  go=0;
  for(;;){
    fgets(buf,100,fp);
    if(feof(fp))break;
    l=strlen(buf); buf[l-1]='\0';

    if(go==0 && buf[0]=='['){
      p1=strtok(buf+1," ");
      p2=strtok(NULL," ]\n");
      if(ln>=atoi(p1) && ln<=atoi(p2))go=1;
      continue;
    }

    if(go==1 && *buf=='[')break;

    if(go==0)continue;

    if(buf[0]=='@'){
      q1=strtok(buf+1," ");
      q2=strtok(NULL," ");
      if(strcmp(q2,"RAND")==0){
        q3=strtok(NULL," "); min=atoi(q3);
        q4=strtok(NULL," \n"); max=atoi(q4);
        v[atoi(q1)]=min+rand()%(max-min+1);
      }
      continue;
    }

    for(x=buf;*x!='\0';x++){
      if(*x=='@'){
        q1=x+1; for(x=q1;*x!='$';x++); *x='\0';
        q2=x+1; for(x=q2;*x!='$';x++); *x='\0';
        fprintf(fp1,q1,v[atoi(q2)]);
        continue;
      }
      fprintf(fp1,"%c",*x);
    }
    fprintf(fp1,"\n");

  }
  fclose(fp);
  fclose(fp1);
}
