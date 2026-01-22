#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void main(void){
  char ip[21],ser[21],*gs,*xx,*yy;
  char bin[512],des[512],ff[512],cmd[512],buf[10000];
  FILE *f;
  long n;
  
  gs=getenv("QUERY_STRING");
  if(gs==NULL){
    printf("Status: 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nmissing query");
    exit(0);
  }
  *ip=*ser='\0';
  for(xx=gs;*xx!='\0';xx++){
    if(strncmp(xx,"ip=",3)==0){
      yy=ip; for(xx+=3;*xx!='\0' && *xx!='&' && yy-ip<20;xx++)*yy++=*xx;
      *yy='\0';
      continue;
    }
    if(strncmp(xx,"ser=",4)==0){
      yy=ser; for(xx+=4;*xx!='\0' && *xx!='&' && yy-ser<20;xx++)*yy++=*xx;
      *yy='\0';
      continue;
    }
  }

  mkdir("/run/display", 0777);
  sprintf(bin,"/run/display/%s.bin",ser);
  sprintf(des,"/run/display/%s.des",ser);
  sprintf(ff,"/run/display/%s.ff", ser);
  sprintf(cmd,"/home/www/display/prog %s %s; /home/www/display/write3 %s %s %s",ser, ip, des, ff, bin);
  system(cmd);

  f = fopen(bin, "rb");
  fseek(f, 0, SEEK_END);
  n = ftell(f);
  fseek(f, 0, SEEK_SET);

printf("Status: 200 OK\r\n");
printf("Content-Type: application/octet-stream\r\n");
printf("Content-Encoding: identity\r\n");
printf("Content-Length: 6145\r\n");
printf("\r\n");
fflush(stdout);

  fread(buf, 1, n, f);
  fwrite(buf, 1, n, stdout);
  fclose(f);

}
