#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define BITREFRESH 2

int tt[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};

void ww1(FILE *fp,char *name,unsigned char *a){
  int k,m,n,zz,elm;
  unsigned char *aa;
  elm=(1<<BITREFRESH)-1;
  fprintf(fp,"unsigned long %s[%d][128]={",name,elm);
  for(k=0;k<elm;k++){
    aa=a;
    for(m=0;m<128;m++){
      fprintf(fp,"0b");
      for(n=0;n<32;n++){
        zz=(*aa)>>(8-BITREFRESH); 
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
  unsigned int y,n,m,r,g,b,l,k,v,w,rb,gb,bb,ml,ax,cc,*c,yy,ty;
  int x;
  
  // file.ff out.h
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);
  
  fp=fopen(argv[2],"wb");
  fprintf(fp,"unsigned int elm=%d;\n",(1<<BITREFRESH)-1);
  ww1(fp,"mr",F+16,BITREFRESH);
  ww1(fp,"mg",F+18,BITREFRESH);
  ww1(fp,"mb",F+20,BITREFRESH);
  fclose(fp);
}
  
