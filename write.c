#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "font_0.h"
#include "font_1.h"
#include "font_2.h"

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

unsigned int *mf[]={&font_0[0][0],&font_1[0][0],&font_2[0][0]};

int main(int argc,char **argv){
  unsigned char F[32784],*a,zz;
  char buf[100];
  FILE *fp,*fp2;
  unsigned int y,n,m,r,g,b,l,k,v,w,rb,gb,bb,ml,ax,cc,*c,yy,ty;
  int x;
  
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
    yy=*(mf[ty]+1);
    l=strlen(buf+16)-1;
    
    printf("%d %d %d %d %d %d %d\n",x,y,r,g,b,ty,l);

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

    // processing 12 bits
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
            w=y+n;
            if(v<64&&w<64){
              a=F+16+(w*64+v)*8;
              *(a+0)=r&0xf0;
              *(a+2)=g&0xf0;
              *(a+4)=b&0xf0;
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

  // write gm file
  fp2=fopen("hh.qq","wb");
  fprintf(fp2,"char MM[6164]={\n");
  fp=fopen(argv[3],"wb");
  a=F+16;
  for(n=0;n<2048;){
    zz=(*a)>>4;
    a+=8;
    zz|=(*a);
    a+=8;
    fwrite(&zz,1,1,fp);
    fprintf(fp2,"0x%02x,",zz);
    n++;
    if(n%16==0)fprintf(fp2,"\n");
  }
  a=F+18;
  for(n=0;n<2048;){
    zz=(*a)>>4;
    a+=8;
    zz|=(*a);
    a+=8;
    fwrite(&zz,1,1,fp);
    fprintf(fp2,"0x%02x,",zz);
    n++;
    if(n%16==0)fprintf(fp2,"\n");
  }
  a=F+20;
  for(n=0;n<2048;){
    zz=(*a)>>4;
    a+=8;
    zz|=(*a);
    a+=8;
    fwrite(&zz,1,1,fp);
    if(n<2047)fprintf(fp2,"0x%02x,",zz);
    else fprintf(fp2,"0x%02x};\n",zz);
    n++;
    if(n%16==0)fprintf(fp2,"\n");
  }
  fclose(fp);
  fclose(fp2);
}
