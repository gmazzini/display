// gcc display/create.c -o q1
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main (){
  unsigned char F[32784];
  unsigned char *a,r,g,b;
  FILE *fp,*fpR,*fpG,*fpB;
  int n;

  // read ff file
  fp=fopen("gm2.ff","rb");
  fread(F,32784,1,fp);
  fclose(fp);

  if(memcmp(F,"farbfeld",8)!=0)exit(-1);
  if(memcmp(F+8,"\x00\x00\x00\x40",4)!=0)exit(-1);
  if(memcmp(F+12,"\x00\x00\x00\x40",4)!=0)exit(-1);

  // change bits for color
  a=F+16;
  for(n=0;n<4096;n++){
    *(a+0)&=0xf0;
    *(a+1)=0x00;
    *(a+2)&=0xf0; 
    *(a+3)=0x00;
    *(a+4)&=0xf0;
    *(a+5)=0x00;
    a+=8;
  }
  
  // save ff file with new bits for color
  fp=fopen("gm3.ff","wb");
  fwrite(F,32784,1,fp);
  fclose(fp);

  // save ff file RGB components
  fpR=fopen("gm3_R.ff","wb");
  fpG=fopen("gm3_G.ff","wb");
  fpB=fopen("gm3_B.ff","wb");
  fwrite(F,16,1,fpR);
  fwrite(F,16,1,fpG);
  fwrite(F,16,1,fpB);
  a=F+16;
  for(n=0;n<4096;n++){
    fwrite(a,1,1,fpR);
    fwrite("\x00\x00\x00\x00\x00\xff\xff",7,1,fpR);
    fwrite("\x00\x00",2,1,fpG);
    fwrite(a+2,1,1,fpG);
    fwrite("\x00\x00\x00\xff\xff",5,1,fpG);
    fwrite("\x00\x00\x00\x00",4,1,fpB);
    fwrite(a+4,1,1,fpB);
    fwrite("\x00\xff\xff",3,1,fpB);
    a+=8;
  }
  fclose(fpR);
  fclose(fpG);
  fclose(fpB);
}
