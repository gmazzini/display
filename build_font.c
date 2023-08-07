#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (){
  unsigned char F[16],A[8],*H;
  FILE *fp;
  unsigned long x,y,v;
  int n,t;

  fp=fopen("my.ff","rb");
  fread(F,16,1,fp);
  if(memcmp(F,"farbfeld",8)!=0)exit(-1);
  x=F[8]<<24|F[9]<<16|F[10]<<8|F[11];
  y=F[12]<<24|F[13]<<16|F[14]<<8|F[15];
  t=x*y;
  H=malloc(t);
  for(n=0;n<t;n++){
    fread(A,8,1,fp);
    v=A[0]<<16|A[2]<<8|A[4];
    printf("%ld",v);
  }
  fclose(fp);
  

  printf("%ld %ld\n",x,y);

}
