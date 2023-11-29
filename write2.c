#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "font_0.h"
#include "font_1.h"
#include "font_2.h"
#include "font_3.h"
#include "font_4.h"

unsigned int *mf[]={
  &font_0[0][0],
  &font_1[0][0],
  &font_2[0][0],
  &font_3[0][0],
  &font_4[0][0]
};

char hextable[] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1, 0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

int mask[]={0,0b10000000,0b11000000,0b11100000,0b11110000,0b11111000,0b11111100,0b11111110,0b11111111};

int main(int argc,char **argv){
  unsigned char F[32784],*a,t;
  char buf[100];
  FILE *fp;
  unsigned int y,n,m,r,g,b,l,k,v,w,rb,gb,bb,ml,ax,cc,*c,yy,ty,bit,mymask;
  int x,xx;
  
  // name.des bit out.ff
  memcpy(F,"farbfeld",8);
  memcpy(F+8,"\x00\x00\x00\x40",4);
  memcpy(F+12,"\x00\x00\x00\x40",4);
  bit=atoi(argv[2]);
  mymask=mask[bit];

  fp=fopen(argv[1],"rb"); 
  for(;;){
    fgets(buf,100,fp);
    if(feof(fp))break;
    t=*buf;
    switch(t){
      
      case '1':
        buf[4]='\0';
        x=atoi(buf+2);
        buf[7]='\0';
        y=atoi(buf+5);
        r=(hextable[*(buf+8)]<<4|hextable[*(buf+9)])&mymask;
        g=(hextable[*(buf+10)]<<4|hextable[*(buf+11)])&mymask;
        b=(hextable[*(buf+12)]<<4|hextable[*(buf+13)])&mymask;
        rb=(hextable[*(buf+15)]<<4|hextable[*(buf+16)])&mymask;
        gb=(hextable[*(buf+17)]<<4|hextable[*(buf+18)])&mymask;
        bb=(hextable[*(buf+19)]<<4|hextable[*(buf+20)])&mymask;
        buf[24]='\0';
        ty=atoi(buf+22);
        yy=*(mf[ty]+1);
        l=strlen(buf+25)-1;
        printf("%c %02d %02d %02x%02x%02x %02x%02x%02x %02d %02d\n",t,x,y,r,g,b,rb,gb,bb,ty,l);
      
        // justification
        if(x<0){
          ml=0;
          for(k=0;k<l;k++){
            n=(*(buf+25+k)-31)&0x7f;
            ml+=*(mf[ty]+n*(yy+1))+1;
          }
          if(x==-1)x=64-ml;
          else if(x==-2)x=(64-ml)/2;
        }

        // processing
        ax=0;
        for(k=0;k<l;k++){
          n=(*(buf+25+k)-31)&0x7f;
          c=mf[ty]+n*(yy+1);
          ml=*c;
          for(n=1;n<=yy;n++){
            cc=*(c+n);
            for(m=0;m<ml;m++){
              v=x+m+ax;
              w=y+n-1;
              if(v<64&&w<64){
                a=F+16+(w*64+v)*8;
                if(cc&0x8000){
                  a[0]=r; a[1]=0;
                  a[2]=g; a[3]=0;
                  a[4]=b; a[5]=0;
                  a[6]=255; a[7]=255;
                }
                else {
                  a[0]=rb; a[1]=0;
                  a[2]=gb; a[3]=0;
                  a[4]=bb; a[5]=0;
                  a[6]=255; a[7]=255;
                }
              }
              cc<<=1;
            }
          }
          ax+=(ml+1);
        }
        break;

      case '2':
        buf[4]='\0';
        x=atoi(buf+2);
        buf[7]='\0';
        xx=atoi(buf+5);
        if(xx<x){v=xx; xx=x; x=v;};
        buf[10]='\0';
        y=atoi(buf+8);
        buf[13]='\0';
        yy=atoi(buf+11);
        if(yy<y){v=yy; yy=y; y=v;};
        r=(hextable[*(buf+14)]<<4|hextable[*(buf+15)])&mymask;
        g=(hextable[*(buf+16)]<<4|hextable[*(buf+17)])&mymask;
        b=(hextable[*(buf+18)]<<4|hextable[*(buf+19)])&mymask;
        printf("%c %02d %02d %02d %02d %02x%02x%02x\n",t,x,xx,y,yy,r,g,b);
        for(w=y;w<=yy;w++){
          for(v=x;v<=xx;v++){
            a=F+16+(w*64+v)*8;
            a[0]=r; a[1]=0;
            a[2]=g; a[3]=0;
            a[4]=b; a[5]=0;
            a[6]=255; a[7]=255;
          }
        }
        break;
      
    }
  }
  fclose(fp);
  
  // write ff file
  fp=fopen(argv[3],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
}
