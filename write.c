#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "font_0.h"
#include "font_1.h"
#include "font_2.h"

unsigned int *mf[]={&font_0[0][0],&font_1[0][0],&font_2[0][0]};

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

int tt[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};
int mask[]={0,0b10000000,0b1100000,0b11100000,0b11110000,0b11111000,0b11111100,0b11111110,0b11111111};
void ww1(FILE *fp,char *name,unsigned char *a,int bit){
  int k,m,n,zz,elm;
  unsigned char *aa;
  elm=(1<<bit)-1;
  fprintf(fp,"unsigned long %s[%d][128]={",name,elm);
  for(k=0;k<elm;k++){
    aa=a;
    for(m=0;m<128;m++){
      fprintf(fp,"0b");
      for(n=0;n<32;n++){
        zz=(*aa)>>(8-bit); 
        aa+=8;
        if(zz&tt[k+1])fprintf(fp,"1");
        else fprintf(fp,"0");
      }
      if(k<elm-1||(k==elm-1&&m<127))fprintf(fp,",");
    }
  }
  fprintf(fp,"};\n");
}

int main(int argc,char **argv){
  unsigned char F[32784],*a,zz;
  char buf[100];
  FILE *fp,*fp2;
  unsigned int y,n,m,r,g,b,l,k,v,w,rb,gb,bb,ml,ax,cc,*c,yy,ty,bit,mymask;
  int x;
  
  // name.des bit out.ff
  memcpy(F,"farbfeld",8);
  memcpy(F+8,"\x00\x00\x00\x40",4);
  memcpy(F+12,"\x00\x00\x00\x40",4);
  bit=atoi(argv[2]);
  mymask=mask[bit];

  fp=fopen(argv[1],"rb");
  fgets(buf,100,fp);
  
  // background
  rb=(hextable[*(buf+0)]<<4|hextable[*(buf+1)])&mymask;
  gb=(hextable[*(buf+2)]<<4|hextable[*(buf+3)])&mymask;
  bb=(hextable[*(buf+4)]<<4|hextable[*(buf+5)])&mymask;
  a=F+16;
  for(n=0;n<4096;n++){
    *(a++)=rb; *(a++)=0x00;
    *(a++)=gb; *(a++)=0x00;
    *(a++)=bb; *(a++)=0x00;
    *(a++)=0xff; *(a++)=0xff;
  }
  printf("%02x%02x%02x\n",rb,gb,bb);
  
  for(;;){
    fgets(buf,100,fp);
    if(feof(fp))break;
    
    *(buf+2)='\0';
    x=atoi(buf);
    *(buf+5)='\0';
    y=atoi(buf+3);
    r=(hextable[*(buf+6)]<<4|hextable[*(buf+7)])&mymask;
    g=(hextable[*(buf+8)]<<4|hextable[*(buf+9)])&mymask;
    b=(hextable[*(buf+10)]<<4|hextable[*(buf+11)])&mymask;
    *(buf+15)='\0';
    ty=atoi(buf+13);
    yy=*(mf[ty]+1);
    l=strlen(buf+16)-1;
    
    printf("%02d %02d %02x%02x%02x %02d %02d\n",x,y,r,g,b,ty,l);

    // justification
    if(x<0){
      ml=0;
      for(k=0;k<l;k++){
        n=(*(buf+16+k)-31)&0x7f;
        ml+=*(mf[ty]+n*(yy+1))+1;
      }
      if(x==-1)x=64-ml;
      else if(x==-2)x=(64-ml)/2;
    }

    // processing
    ax=0;
    for(k=0;k<l;k++){
      n=(*(buf+16+k)-31)&0x7f;
      c=mf[ty]+n*(yy+1);
      ml=*c;
      
      for(n=1;n<=yy;n++){
        cc=*(c+n);
        for(m=0;m<ml;m++){
          if(cc&0x8000){
            v=x+m+ax;
            w=y+n-1;
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
  
  // write ff file
  fp=fopen(argv[2],"wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
}
