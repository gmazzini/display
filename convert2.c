#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int tt[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};

int main(int argc,char **argv){
  unsigned char F[32784];
  FILE *fp;
  int k,m,n,zz,i,j,bit;
  unsigned char *aa,elm;
  unsigned long oo;
  
  // file.ff bit out.h
  bit=atoi(argv[2]);
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);

  elm=(1<<bit)-1;
  fp=fopen(argv[3],"wb");
  fwrite(&elm,1,1,fp);
  i=384*elm;
  for(k=0;k<elm;k++){
    for(j=0;j<3;j++){
      aa=F+16+j*2;
      for(m=0;m<128;m++){
        oo=0;
        for(n=0;n<32;n++){
          zz=(*aa)>>(8-bit); 
          aa+=8;
          if(zz&tt[k+1])oo|=1;
          if(n<31)oo<<=1;
        }
        fwrite(&oo,4,1,fp);
        i--;
      }
    }
  }
  fclose(fp);
}
