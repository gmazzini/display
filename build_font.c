#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (){
  char buf[100],*p;
  FILE *fp;
  int x,y;

  fp=fopen("my.pbm","rb");
  fgets(buf,100,fp);
  if(memcmp(buf,"P1",2)!=0)exit(-1);
  fgets(buf,100,fp);
  p=strtok(" ",buf);
  *p='\0';
  x=atoi(buf);
  y=atoi(p);
  fclose(fp);
  
  printf("%d %d\n",x,y);

}
