#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"

void point1(unsigned char *F,double x,double y,unsigned char *ooo){
  unsigned char *a;
  if(x>63)x=63;
  if(x<0)x=0;
  if(y>63)y=63;
  if(y<0)y=0;
  a=F+16+((int)x+((int)y)*64)*8;

  *a=255-*ooo;;
  *(a+2)=255-*(ooo+1);
  *(a+4)=255-*(ooo+2);

  return;
}

void line1(unsigned char *F,unsigned char *ooo,double x1,double y1,double x2,double y2,int v){
  double a,b,len,dx,x,y;
  if(x1>x2){a=x1; x1=x2; x2=a; a=y1; y1=y2; y2=a;}
  a=(y1-y2)/(x1-x2);
  b=y1-a*x1;
  len=sqrt(pow(x1-x2,2)+pow(y1-y2,2));
  dx=(x2-x1)/len/2;
  for(x=x1;x<=x2;x+=dx){
    y=a*x+b;
    point1(F,x,y,ooo);
    if(v==1){
      point1(F,x+1,y,ooo);
      point1(F,x-1,y,ooo);
      point1(F,x,y+1,ooo);
      point1(F,x,y-1,ooo);
    }
  }
  return;
}

void read1(unsigned char *F,unsigned char *rr,double x,double y){
  unsigned char *a;
  if(x>63)x=63;
  if(x<0)x=0;
  if(y>63)y=63;
  if(y<0)y=0;
  a=F+16+((int)x+((int)y)*64)*8;
  rr[0]=a[0];
  rr[1]=a[2];
  rr[2]=a[4];
  return;
}

void ave1(unsigned char *F,unsigned char *ooo,double x1,double y1,double x2,double y2,int v){
  double a,b,len,dx,x,y,rrr[3];
  unsigned char rr[3];
  int cc;
  rrr[0]=rrr[1]=rrr[2]=0;
  cc=0;
  if(x1>x2){a=x1; x1=x2; x2=a; a=y1; y1=y2; y2=a;}
  a=(y1-y2)/(x1-x2);
  b=y1-a*x1;
  len=sqrt(pow(x1-x2,2)+pow(y1-y2,2));
  dx=(x2-x1)/len/2;
  for(x=x1;x<=x2;x+=dx){
    y=a*x+b;
    read1(F,rr,x,y);
    rrr[0]+=rr[0]; rrr[1]+=rr[1]; rrr[2]+=rr[2]; cc++;
    if(v==1){
      read1(F,rr,x+1,y);
      rrr[0]+=rr[0]; rrr[1]+=rr[1]; rrr[2]+=rr[2]; cc++;
      read1(F,rr,x-1,y);
      rrr[0]+=rr[0]; rrr[1]+=rr[1]; rrr[2]+=rr[2]; cc++;
      read1(F,rr,x,y+1);
      rrr[0]+=rr[0]; rrr[1]+=rr[1]; rrr[2]+=rr[2]; cc++;
      read1(F,rr,x,y-1);
      rrr[0]+=rr[0]; rrr[1]+=rr[1]; rrr[2]+=rr[2]; cc++;
    }
  }
  ooo[0]=(unsigned char)(rrr[0]/cc);
  ooo[1]=(unsigned char)(rrr[1]/cc);
  ooo[2]=(unsigned char)(rrr[2]/cc);
  return;
}

int main(int argc,char **argv){
  unsigned char F[32784],ooo[3];
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

  hr=(90.0-30.0*(hh+mm/60))*2.0*M_PI/360.0;
  mr=(90.0-6.0*mm)*2.0*M_PI/360.0;

  ave1(F,ooo,31,31,31+18*cos(hr),63-31-18*sin(hr),1);
  line1(F,ooo,31,31,31+18*cos(hr),63-31-18*sin(hr),1);
  ave1(F,ooo,31,31,31+28*cos(mr),63-31-28*sin(mr),2);
  line1(F,ooo,31,31,31+28*cos(mr),63-31-28*sin(mr),2);

  // write ff file
  fp=fopen(argv[2],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);

}
