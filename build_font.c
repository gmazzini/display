#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (){
  char F[16],*p;
  FILE *fp;
  unsigned long x,y,v,n,a;

  fp=fopen("my.ff","rb");
  fread(F,16,1,fp);
  if(memcmp(F,"farbfeld",8)!=0)exit(-1);
  x=F[8]<<24|F[9]<<16|F[10]<<8|F[11];
  y=F[12]<<24|F[13]<<16|F[14]<<8|F[15];
  v=x*y;
  p=malloc(v);
  for(n=0;n<v;){
    fread(F,8,1,fp);
    a=F[0]<<16|F[2]<<8|F[4];
    if(a==0)printf("0");else printf("1");
    n++;
    if(n%x==0)printf("\n");
  }
  fclose(fp);
  
  printf("%ld %ld\n",x,y);

}
