#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"

void point1(unsigned char *F,int x,int y){
  unsigned char *a;
  a=F+16+(x+y*64)*8;
  *a=255;
  *(a+2)=255;
  *(a+4)=255;
  return;
}

void line1(unsigned char *F,int x1,int y1,int x2,int y2){
  double a,b,len,dx,x,y;
  if(x1==x2){
    if(y1>y2){a=y1; y1=y2; y2=a;}
    for(y=y1;y<=y2;y++)point1(F,(int)x1,(int)y);
    return;
  }
  if(x1>x2){a=x1; x1=x2; x2=a; a=y1; y1=y2; y2=a;}
  a=(y1-y2)/(x1-x2);
  b=y1-a*x1;
  len=sqrt(pow(x1-x2,2)+pow(y1-y2,2));
  dx=(x2-x1)/len/2;
  for(x=x1;x<=x2;x+=dx){
    y=a*x+b;
    point1(F,(int)x,(int)y);
  }
  return;
}

int main(int argc,char **argv){
  unsigned char F[32784];
  FILE *fp;
  time_t now;
  struct tm *now_tm;
  double hr,mr;
  int hh,mm;

  // read ff file
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);

  now=time(NULL);
  now_tm=localtime(&now);
  hh=now_tm->tm_hour;
  mm=now_tm->tm_min;
  if(hh>11)hh-=12;
  hr=(90-30*hh)*2*M_PI/360;
  mr=(90-6+mm)*2*M_PI/360;
  line1(F,31,31,(int)floor(31+15*cos(hr)),(int)floor(31+15+sin(hr)));
  line1(F,31,31,(int)floor(31+25*cos(mr)),(int)floor(31+25+sin(mr)));
  
   // write ff file
  fp=fopen(argv[2],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
  
}
