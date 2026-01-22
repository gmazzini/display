#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void main(void){
  const char *qs;
  char qbuf[1024], ip[256], ser[256];
  char bin[512], des[512], ff[512];
  char cmd[1400];
  char *tok, *eq;
  FILE *f;
  long n;

  char ip[21],ser[21],*gs,*xx,*yy;

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

  snprintf(bin, sizeof(bin), "/run/display/%s.bin", ser);
  snprintf(des, sizeof(des), "/run/display/%s.des", ser);
  snprintf(ff,  sizeof(ff),  "/run/display/%s.ff",  ser);

  snprintf(cmd, sizeof(cmd),
           "/home/www/display/prog %s %s; /home/www/display/write3 %s %s %s",
           ser, ip, des, ff, bin);
  system(cmd);

  f = fopen(bin, "rb");
  if (!f) { puts("Status: 500\r\nContent-Type: text/plain\r\n\r\nmissing bin"); return 0; }

  fseek(f, 0, SEEK_END);
  n = ftell(f);
  fseek(f, 0, SEEK_SET);

  printf("Content-Encoding: identity\r\n");
  printf("Content-Type: application/octet-stream\r\n");
  printf("Content-Length: %ld\r\n\r\n", n);

  while ((n = fread(cmd, 1, sizeof(cmd), f)) > 0) fwrite(cmd, 1, (size_t)n, stdout);
  fclose(f);

  return 0;
}
