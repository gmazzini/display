#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define BITREFRESH 2

int tt[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};

void mywrite(FILE *fp,char *name,unsigned char *a){
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
  unsigned char F[32784];
  FILE *fp;
  
  // file.ff out.h
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);
  
  fp=fopen(argv[2],"wb");
  fprintf(fp,"unsigned int elm=%d;\n",(1<<BITREFRESH)-1);
  mywrite(fp,"mr",F+16);
  mywrite(fp,"mg",F+18);
  mywrite(fp,"mb",F+20);
  fclose(fp);
}
  
