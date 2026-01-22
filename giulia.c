#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main(){
  char *gs,*x,*yy,cmd[10001];
  FILE *fp;
  long lv2;
  
  gs=getenv("QUERY_STRING");
  if(gs==NULL){printf("Status: 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nmissing query"); exit(0); }
  *cmd='\0';
  for(x=gs;*x!='\0';x++){
    if(strncmp(x,"des=",4)==0){
      yy=cmd; for(x+=4;*x!='\0' && *x!='&' && yy-cmd<10000;x++)*yy++=*x;
      *yy='\0';
      continue;
    }
  }

  fp=fopen("/run/display/giulia.des","wt");
  for(x=cmd;*x!='\0';x++){
    if(*x=='\\')*x='\n';
    fputc(*x,fp);
  }
  fclose(fp);

  sprintf(cmd,"/home/www/display/write3 /run/display/giulia.des /run/display/giulia.ff /run/display/giulia.bin");
  system(cmd);

  fp=fopen("/run/display/giulia.bin","rb");
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
