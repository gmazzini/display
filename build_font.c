#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (){
  char F[16],*p;
  FILE *fp;
  unsigned long x,y,v,n,a,rx,ry,w;

  fp=fopen("my.ff","rb");
  fread(F,16,1,fp);
  if(memcmp(F,"farbfeld",8)!=0)exit(-1);
  x=F[8]<<24|F[9]<<16|F[10]<<8|F[11];
  y=F[12]<<24|F[13]<<16|F[14]<<8|F[15];
  v=x*y;
  p=malloc(v);
  for(n=0;n<v;n++){
    fread(F,8,1,fp);
    a=F[0]<<16|F[2]<<8|F[4];
    p[n]=(a==0)?1:0;
  }
  fclose(fp);

  for(rx=0;rx<x;rx++){
    w=0;
    for(ry=0;ry<y;ry++)if(p[ry*y+rx]==0)w++;
    if(w==0)printf("%ld\n",rx);
  }


  
  printf("%ld %ld\n",x,y);

}
