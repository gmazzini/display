#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int tt[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};

int main(int argc,char **argv){
  unsigned char F[32784];
  FILE *fp;
  int k,m,n,zz,elm,i,j,bit;
  unsigned char *aa;
  
  // file.ff bit out.h
  bit=atoi(argv[2]);
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);

  elm=(1<<bit)-1;
  fp=fopen(argv[3],"wb");
  fprintf(fp,"unsigned int elm=%d;\n",elm);
  fprintf(fp,"unsigned long MM[%d][384]={",elm);
  i=384*3;
  for(k=0;k<elm;k++){
    for(j=0;j<3;j++){
      aa=F+16+j*2;
      for(m=0;m<128;m++){
        fprintf(fp,"0b");
        for(n=0;n<32;n++){
          zz=(*aa)>>(8-bit); 
          aa+=8;
          if(zz&tt[k+1])fprintf(fp,"1");
          else fprintf(fp,"0");
        }
        if(--i>0)fprintf(fp,",");
        else fprintf(fp,"};");
      }
    }
  }
  fclose(fp);
}
  
