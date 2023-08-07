#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (){
  char H[16],*p;
  FILE *fp;
  unsigned long x,y;

  fp=fopen("my.ff","rb");
  fread(H,16,1,fp);
  if(memcmp(H,"farbfeld",8)!=0)exit(-1);
  x=F[8]<<24|F[9]<<16|F[10]<<8|F[11];
  y=F[12]<<24|F[13]<<16|F[14]<<8|F[15];
  fclose(fp);
  
  printf("%d %d\n",x,y);

}
