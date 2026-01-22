#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

// v[0]=mat v[1]=ip v[2]=step v[3]=seq
// v[4]=hh(%02d) v[5]=mm(%02d) v[6]=ss(%02d)
// RAND <fmt> <start> <end>
// LCG <fmt> <len> <mod> <base>
void main(int argc,char *argv[]){
  FILE *fp,*fp1,*fp2;
  char buf[100],*p1,*p2,*q,*q1,*x,*yy,fmt[20],*gs,cmd[10000];
  int l,tot,ln,go,a0,a1,a2,a3,a4;
  char v[100][100];
  struct timespec ts;
  struct tm tmv;
  uint64_t seed;
  long lv2;

  gs=getenv("QUERY_STRING");
  if(gs==NULL){printf("Status: 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nmissing query"); exit(0); }
  *v[0]=*v[1]='\0';
  for(x=gs;*x!='\0';x++){
    if(strncmp(x,"ip=",3)==0){
      yy=v[1]; for(x+=3;*x!='\0' && *x!='&' && yy-v[1]<20;x++)*yy++=*x;
      *yy='\0';
      continue;
    }
    if(strncmp(x,"ser=",4)==0){
      yy=v[0]; for(x+=4;*x!='\0' && *x!='&' && yy-v[0]<20;x++)*yy++=*x;
      *yy='\0';
      continue;
    }
  }
  
  clock_gettime(CLOCK_REALTIME, &ts);
  seed = (uint64_t)ts.tv_sec ^ (uint64_t)ts.tv_nsec ^ (uint64_t)getpid();
  srand((unsigned)seed);
  localtime_r(&ts.tv_sec, &tmv);
  sprintf(v[4],"%02d",tmv.tm_hour);
  sprintf(v[5],"%02d",tmv.tm_min);
  sprintf(v[6],"%02d",tmv.tm_sec);
  
  sprintf(buf,"/run/display/%s.step",v[0]); 
  fp=fopen(buf,"r+");
  fgets(v[2],100,fp); l=strlen(v[2]); v[2][l-1]='\0';
  rewind(fp);
  lv2=atol(v[2]);
  fprintf(fp,"%ld\n",lv2+1);
  fclose(fp);

  sprintf(buf,"/run/display/%s.mat",v[0]);
  fp=fopen(buf,"rt");
  fgets(v[3],100,fp); l=strlen(v[3]); v[3][l-1]='\0';
  fclose(fp);

  sprintf(buf,"/run/display/%s.des",v[0]);
  fp1=fopen(buf,"wt");
  sprintf(buf,"/run/display/%s.seq",v[3]);
  fp=fopen(buf,"rt");
  fgets(buf,100,fp);
  tot=atoi(buf);
  ln=lv2%tot;
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
      q=strtok(buf+1," "); a0=atoi(q);
      q=strtok(NULL," ");
      
      if(strcmp(q,"RAND")==0){
        q=strtok(NULL," "); strcpy(fmt,q);
        q=strtok(NULL," "); a1=atoi(q);
        q=strtok(NULL," \n"); a2=atoi(q);
        sprintf(v[a0],fmt,a1+rand()%(a2-a1+1));
      }

      else if(strcmp(q,"LCG")==0){
        q=strtok(NULL," "); strcpy(fmt,q);
        q=strtok(NULL," "); a1=atoi(q);
        q=strtok(NULL," "); a2=atoi(q);
        q=strtok(NULL," \n"); a3=atoi(q);
        sprintf(buf,"/run/display/lcg/%d-%d.lcg",a1,lv2%a2);
        fp2=fopen(buf,"rt");
        fgets(buf,100,fp2);
        a4=atoi(buf);
        fclose(fp2);
        sprintf(v[a0],fmt,a3+a4);
      }
      
      continue;
    }

    for(x=buf;*x!='\0';x++){
      if(*x=='@'){
        q1=x+1; for(x=q1;*x!='$';x++); *x='\0';
        fprintf(fp1,"%s",v[atoi(q1)]);
        continue;
      }
      fprintf(fp1,"%c",*x);
    }
    fprintf(fp1,"\n");

  }
  fclose(fp);
  fclose(fp1);

  sprintf(cmd,"/home/www/display/write3 /run/display/%s.des /run/display/%s.ff /run/display/%s.bin",v[0],v[0],v[0]);
  system(cmd);
  sprintf(buf,"/run/display/%s.bin",v[0]);
  fp=fopen(buf,"rb");
  fseek(fp,0,SEEK_END);
  lv2=ftell(fp);
  fseek(fp,0,SEEK_SET);
  printf("Content-Type: application/octet-stream\r\n");
  printf("Content-Encoding: identity\r\n");
  printf("Content-Length: %ld\r\n",lv2);
  printf("\r\n");
  fflush(stdout);
  fread(cmd,1,lv2,fp);
  fwrite(cmd,1,lv2,stdout);
  fclose(fp);
}
