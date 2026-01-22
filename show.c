#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

void main(){
  char *gs,*x,*yy,ser[21],buf[100],cmd[10000];
  FILE *fp;
  long lv2;
  
  gs=getenv("QUERY_STRING");
  if(gs==NULL){printf("Status: 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nmissing query"); exit(0); }
  *ser='\0';
  for(x=gs;*x!='\0';x++){
    if(strncmp(x,"ser=",4)==0){
      yy=ser; for(x+=4;*x!='\0' && *x!='&' && yy-ser<20;x++)*yy++=*x;
      *yy='\0';
      continue;
    }
  }

  sprintf(buf,"/run/display/%s.bin",ser);
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
