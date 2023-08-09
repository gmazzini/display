#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "font_0.h"
#include "font_1.h"

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

unsigned int *mf[]={font_0,font_1};

int main(int argc,char **argv){
  unsigned char F[32784],*a;
  char buf[100];
  FILE *fp;
  unsigned int x,y,n,m,r,g,b,l,k,v,w,rb,gb,bb,ml,ax,cc,*c,yy,ty;

  // name.des out.ff
  
  memcpy(F,"farbfeld",8);
  memcpy(F+8,"\x00\x00\x00\x40",4);
  memcpy(F+12,"\x00\x00\x00\x40",4);

  fp=fopen(argv[1],"rb");
  fgets(buf,100,fp);

  // background
  rb=hextable[*(buf+0)]<<4|hextable[*(buf+1)];
  gb=hextable[*(buf+2)]<<4|hextable[*(buf+3)];
  bb=hextable[*(buf+4)]<<4|hextable[*(buf+5)];
  a=F+16;
  for(n=0;n<4096;n++){
    *(a++)=rb; *(a++)=0x00;
    *(a++)=gb; *(a++)=0x00;
    *(a++)=bb; *(a++)=0x00;
    *(a++)=0xff; *(a++)=0xff;
  }
  
  for(;;){
    fgets(buf,100,fp);
    if(feof(fp))break;
  
    *(buf+2)='\0';
    x=atoi(buf);
    *(buf+5)='\0';
    y=atoi(buf+3);
    r=hextable[*(buf+6)]<<4|hextable[*(buf+7)];
    g=hextable[*(buf+8)]<<4|hextable[*(buf+9)];
    b=hextable[*(buf+10)]<<4|hextable[*(buf+11)];
    *(buf+15)='\0';
    ty=atoi(buf+13);
    l=strlen(buf+16)-1;
    
    printf("%d %d %d %d %d %d %d\n",x,y,r,g,b,ty,l);
    
    yy=font_0[0][1];
    ax=0;
    for(k=0;k<l;k++){
      n=(*(buf+16+k)-31)&0x7f;
      c=font_0[n];
      ml=c[0];

      for(n=1;n<=yy;n++){
        cc=c[n];
        for(m=0;m<ml;m++){
          if(cc&0x8000){
            v=x+m+ax;
            w=y+n;
            if(v<64&&w<64){
              a=F+16+(w*64+v)*8;
              *(a+0)=r;
              *(a+2)=g;
              *(a+4)=b;
            }
          }
          cc<<=1;
        }
      }
      ax+=(ml+1);
    }
  }
  fclose(fp);

  fp=fopen(argv[2],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
}
