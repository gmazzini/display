#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void pp1(unsigned char *F,int x,int y){
  unsigned char *a;
  a=F+16+(x+y*64)*8;
  *a=255;
  *(a+2)=255;
  *(a+4)=255;
}

int main(int argc,char **argv){
  unsigned char F[32784],*a;
  FILE *fp;

   // read ff file
  fp=fopen(argv[1],"rb");
  fwrite(F,32784,1,fp);
  fclose(fp);

  pp1(F,10,10);

   // write ff file
  fp=fopen(argv[2],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
  
}
