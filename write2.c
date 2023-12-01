#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
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

int myparse(char *ss,int n,...){
  char *a,*b,buf[50];
  int i,ty;  
  int *pi;
  unsigned int *pui;
  unsigned char *puc;
  strcpy(buf,ss);
  va_list args;
  va_start(args,n);
  a=buf;
  for(i=0;i<n;i++){
    ty=va_arg(args,int);
    switch(ty){
      case 1:
        for(;*a==' ';a++){};
        for(b=a;*a!=' '&&*a!='\0';a++){}; *a='\0'; a++;
        pi=va_arg(args,int *);
        *pi=atoi(b);
        break;
     case 2:
        for(;*a==' ';a++){};
        for(b=a;*a!=' '&&*a!='\0';a++){}; *a='\0'; a++;
        pui=va_arg(args,unsigned int *);
        *pui=atoi(b);
        break;
     case 3:
        for(;*a==' ';a++){};
        for(b=a;*a!=' '&&*a!='\0';a++){}; *a='\0'; a++;
        pui=va_arg(args,unsigned int *);
        *pui=hextable[*(b+0)]<<4|hextable[*(b+1)];
        pui=va_arg(args,unsigned int *);
        *pui=hextable[*(b+2)]<<4|hextable[*(b+3)];
        pui=va_arg(args,unsigned int *);
        *pui=hextable[*(b+4)]<<4|hextable[*(b+5)];
        pui=va_arg(args,unsigned int *);
        *pui=hextable[*(b+6)]<<4|hextable[*(b+7)];
        break;
      case 4:
        for(;*a==' ';a++){};
        for(b=a;*a!=' '&&*a!='\0';a++){}; *a='\0'; a++;
        puc=va_arg(args,unsigned char *);
        strcpy(puc,b);
        break;
      case 5:
        for(;*a==' ';a++){};
        for(b=a;*a!=' '&&*a!='\0';a++){}; *a='\0'; a++;
        pui=va_arg(args,unsigned int *);
        *pui=hextable[*(b+0)]<<4|hextable[*(b+1)];
        break;
    }
  }
  va_end(args);
  return a-buf;
}

int main(int argc,char **argv){
  unsigned char F[32784],F2[32784],*a,*a2,*ss,img[10];
  char buf[100],buf1[100];
  FILE *fp,*fp2;
  unsigned int y,n,m,r,g,b,t,l,k,v,vv,w,ww,rb,gb,bb,tb,ml,ax,cc,*c,yy,ty,xx,xxx,yyy;
  int x;
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
        v=myparse(buf+1,5,1,&x,2,&y,3,&r,&g,&b,&t,3,&rb,&gb,&bb,&tb,2,&ty);
        ss=buf+1+v;
        yy=*(mf[ty]+1);
        l=strlen(ss)-1;  
        if(x<0){
          ml=0;
          for(k=0;k<l;k++){
            n=(*(ss+k)-31)&0x7f;
            ml+=*(mf[ty]+n*(yy+1))+1;
          }
          if(x==-1)x=64-ml;
          else if(x==-2)x=(64-ml)/2;
        }
        ax=0;
        for(k=0;k<l;k++){
          n=(*(ss+k)-31)&0x7f;
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
        myparse(buf+1,5,1,&x,2,&y,2,&xx,2,&yy,3,&r,&g,&b,&t);
        if(xx<x){v=xx; xx=x; x=v;};
        if(yy<y){v=yy; yy=y; y=v;};
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
        myparse(buf+1,5,1,&x,2,&y,2,&xx,2,&yy,3,&r,&g,&b,&t);
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
        myparse(buf+1,3,1,&x,2,&y,3,&r,&g,&b,&t);
        a=F+16+(y*64+x)*8;
        a[0]=(unsigned char)((unsigned int)r*t/255+(unsigned int)a[0]*(255-t)/255);
        a[2]=(unsigned char)((unsigned int)g*t/255+(unsigned int)a[2]*(255-t)/255);
        a[4]=(unsigned char)((unsigned int)b*t/255+(unsigned int)a[4]*(255-t)/255);
        break;
      
      case '5':
        myparse(buf+1,2,4,img,5,&t);
        strcpy(buf1,"image/");
        strcat(buf1,img);
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

      case '6':
        myparse(buf+1,8,4,img,5,&t,1,&x,2,&y,2,&xx,2,&yy,2,&xxx,2,&yyy);
        strcpy(buf1,"image/");
        strcat(buf1,img);
        strcat(buf1,".ff");
        fp2=fopen(buf1,"rb");
        fread(F2,32784,1,fp2);
        fclose(fp2);
        for(w=y;w<=yy;w++){
          for(v=x;v<=xx;v++){
            ww=yyy+w-y; vv=xxx+v-x;
            if(ww>63 || vv>63)continue; 
            a2=F2+16+(w*64+v)*8;
            a=F+16+(ww*64+vv)*8;
            a[0]=(unsigned char)((unsigned int)a2[0]*t/255+(unsigned int)a[0]*(255-t)/255);
            a[2]=(unsigned char)((unsigned int)a2[2]*t/255+(unsigned int)a[2]*(255-t)/255);
            a[4]=(unsigned char)((unsigned int)a2[4]*t/255+(unsigned int)a[4]*(255-t)/255);
          }
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
