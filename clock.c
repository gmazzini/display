#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void point1(unsigned char *F,int x,int y){
  unsigned char *a;
  a=F+16+(x+y*64)*8;
  *a=255;
  *(a+2)=255;
  *(a+4)=255;
}

void line1(unsigned char *F,int x1,int y1,int x2,int y2){
  double a,b,len,dx,x,y;
  // x1<x2 x1=x2
  a=(y1-y2)/(x1-x2);
  b=y1-a*x1;
  len=sqrt(pow(x1-x2,2)+pow(y1-y2,2));
  dx=(x2-x1)/len/2;
  for(x=x1;x<=x2;x+=dx){
    y=a*x+b;
    point1(F,(int)x,(int)y);
  }
}

int main(int argc,char **argv){
  unsigned char F[32784];
  FILE *fp;

   // read ff file
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);

 line1(F,10,10,40,50);

   // write ff file
  fp=fopen(argv[2],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
  
}
