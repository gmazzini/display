#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
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

int main(int argc,char **argv){
  unsigned char F[32784],F2[32784],*a,*a2;
  char buf[100],buf1[100];
  FILE *fp,*fp2;
  unsigned int y,n,m,r,g,b,t,l,k,v,w,rb,gb,bb,tb,ml,ax,cc,*c,yy,ty;
  int x,xx;
  double len,x1,x2,y1,y2,ad,bd,dd,yd,xd;
  
  // name.des out.ff
  memcpy(F,"farbfeld",8);
  memcpy(F+8,"\x00\x00\x00\x40",4);
  memcpy(F+12,"\x00\x00\x00\x40",4);
  for(w=0;w<4096;w++){
    a=F+16+w*8;
    a[0]=0; a[1]=0;
    a[2]=0; a[3]=0;
    a[4]=0; a[5]=0;
    a[6]=255; a[7]=255;
  }
  
  fp=fopen(argv[1],"rb"); 
  for(;;){
    fgets(buf,100,fp);
    if(feof(fp))break;
    switch(buf[0]){
      
      case '1':
        buf[4]='\0';
        x=atoi(buf+2);
        buf[7]='\0';
        y=atoi(buf+5);
        r=hextable[*(buf+8)]<<4|hextable[*(buf+9)];
        g=hextable[*(buf+10)]<<4|hextable[*(buf+11)];
        b=hextable[*(buf+12)]<<4|hextable[*(buf+13)];
        t=hextable[*(buf+14)]<<4|hextable[*(buf+15)];
        rb=hextable[*(buf+17)]<<4|hextable[*(buf+18)];
        gb=hextable[*(buf+19)]<<4|hextable[*(buf+20)];
        bb=hextable[*(buf+21)]<<4|hextable[*(buf+22)];
        tb=hextable[*(buf+23)]<<4|hextable[*(buf+24)];
        buf[28]='\0';
        ty=atoi(buf+26);
        yy=*(mf[ty]+1);
        l=strlen(buf+29)-1;  
        if(x<0){
          ml=0;
          for(k=0;k<l;k++){
            n=(*(buf+29+k)-31)&0x7f;
            ml+=*(mf[ty]+n*(yy+1))+1;
          }
          if(x==-1)x=64-ml;
          else if(x==-2)x=(64-ml)/2;
        }
        ax=0;
        for(k=0;k<l;k++){
          n=(*(buf+29+k)-31)&0x7f;
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
                  a[0]=(unsigned char)((unsigned int)r*t/255+(unsigned int)a[0]*(255-t)/255);
                  a[2]=(unsigned char)((unsigned int)g*t/255+(unsigned int)a[2]*(255-t)/255);
                  a[4]=(unsigned char)((unsigned int)b*t/255+(unsigned int)a[4]*(255-t)/255);
                }
                else {
                  a[0]=(unsigned char)((unsigned int)rb*tb/255+(unsigned int)a[0]*(255-tb)/255);
                  a[2]=(unsigned char)((unsigned int)gb*tb/255+(unsigned int)a[2]*(255-tb)/255);
                  a[4]=(unsigned char)((unsigned int)bb*tb/255+(unsigned int)a[4]*(255-tb)/255);
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
        y=atoi(buf+5);
        buf[10]='\0';
        xx=atoi(buf+8);
        buf[13]='\0';
        yy=atoi(buf+11);
        if(xx<x){v=xx; xx=x; x=v;};
        if(yy<y){v=yy; yy=y; y=v;};
        r=hextable[*(buf+14)]<<4|hextable[*(buf+15)];
        g=hextable[*(buf+16)]<<4|hextable[*(buf+17)];
        b=hextable[*(buf+18)]<<4|hextable[*(buf+19)];
        t=hextable[*(buf+20)]<<4|hextable[*(buf+21)];
        for(w=y;w<=yy;w++){
          for(v=x;v<=xx;v++){
            a=F+16+(w*64+v)*8;
            a[0]=(unsigned char)((unsigned int)r*t/255+(unsigned int)a[0]*(255-t)/255);
            a[2]=(unsigned char)((unsigned int)g*t/255+(unsigned int)a[2]*(255-t)/255);
            a[4]=(unsigned char)((unsigned int)b*t/255+(unsigned int)a[4]*(255-t)/255);
          }
        }
        break;

      case '3':
        buf[4]='\0';
        x=atoi(buf+2);
        buf[7]='\0';
        y=atoi(buf+5);
        buf[10]='\0';
        xx=atoi(buf+8);
        buf[13]='\0';
        yy=atoi(buf+11);
        r=hextable[*(buf+14)]<<4|hextable[*(buf+15)];
        g=hextable[*(buf+16)]<<4|hextable[*(buf+17)];
        b=hextable[*(buf+18)]<<4|hextable[*(buf+19)];
        t=hextable[*(buf+20)]<<4|hextable[*(buf+21)];
        x1=(double)x; x2=(double)xx; y1=(double)y; y2=(double)yy;
        len=sqrt(pow(x1-x2,2)+pow(y1-y2,2));
        if(fabs(x2-x1)<20){
          if(y1>y2){ad=y1; y1=y2; y2=ad; ad=x1; x1=x2; x2=ad;}
          ad=(x1-x2)/(y1-y2);
          bd=x1-ad*y1;
          dd=(y2-y1)/len/2;
          for(v=0,yd=y1;yd<=y2;yd+=dd,v++){
            if(v>1000)break;
            xd=ad*yd+bd;
            if(xd>63.0)xd=63.0; if(xd<0.0)xd=0.0; if(yd>63.0)yd=63.0; if(yd<0.0)yd=0.0;
            a=F+16+((int)xd+((int)yd)*64)*8;
            a[0]=(unsigned char)((unsigned int)r*t/255+(unsigned int)a[0]*(255-t)/255);
            a[2]=(unsigned char)((unsigned int)g*t/255+(unsigned int)a[2]*(255-t)/255);
            a[4]=(unsigned char)((unsigned int)b*t/255+(unsigned int)a[4]*(255-t)/255);
          }
        }
        else {
          if(x1>x2){ad=x1; x1=x2; x2=ad; ad=y1; y1=y2; y2=ad;}
          ad=(y1-y2)/(x1-x2);
          bd=y1-ad*x1;
          dd=(x2-x1)/len/2;
          for(v=0,xd=x1;xd<=x2;xd+=dd,v++){
            if(v>1000)break;
            yd=ad*xd+bd;
            if(xd>63.0)xd=63.0; if(xd<0.0)xd=0.0; if(yd>63.0)yd=63.0; if(yd<0.0)yd=0.0;
            a=F+16+((int)xd+((int)yd)*64)*8;
            a[0]=(unsigned char)((unsigned int)r*t/255+(unsigned int)a[0]*(255-t)/255);
            a[2]=(unsigned char)((unsigned int)g*t/255+(unsigned int)a[2]*(255-t)/255);
            a[4]=(unsigned char)((unsigned int)b*t/255+(unsigned int)a[4]*(255-t)/255);
          }
        }
        break;
      
      case '4':
        buf[4]='\0';
        x=atoi(buf+2);
        buf[7]='\0';
        y=atoi(buf+5);
        r=hextable[*(buf+8)]<<4|hextable[*(buf+9)];
        g=hextable[*(buf+10)]<<4|hextable[*(buf+11)];
        b=hextable[*(buf+12)]<<4|hextable[*(buf+13)];
        t=hextable[*(buf+14)]<<4|hextable[*(buf+15)];
        a=F+16+(y*64+x)*8;
        a[0]=(unsigned char)((unsigned int)r*t/255+(unsigned int)a[0]*(255-t)/255);
        a[2]=(unsigned char)((unsigned int)g*t/255+(unsigned int)a[2]*(255-t)/255);
        a[4]=(unsigned char)((unsigned int)b*t/255+(unsigned int)a[4]*(255-t)/255);
        break;
      
      case '5':
        buf[6]='\0';
        t=hextable[*(buf+7)]<<4|hextable[*(buf+8)];
        strcpy(buf1,"image/");
        strcat(buf1,buf+2);
        strcat(buf1,".ff");
        fp2=fopen(buf1,"rb");
        fread(F2,32784,1,fp2);
        fclose(fp2);
        for(w=0;w<4096;w++){
          a=F+16+w*8;
          a2=F2+16+w*8;
          a[0]=(unsigned char)((unsigned int)a2[0]*t/255+(unsigned int)a[0]*(255-t)/255);
          a[2]=(unsigned char)((unsigned int)a2[2]*t/255+(unsigned int)a[2]*(255-t)/255);
          a[4]=(unsigned char)((unsigned int)a2[4]*t/255+(unsigned int)a[4]*(255-t)/255);
        }
        break;
      
    }
  }
  fclose(fp);
  
  // write ff file
  fp=fopen(argv[2],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
}
