#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc,char **argv){
  unsigned char F[32784];
  FILE *fp;
  int k,m,n,i,j,bit;
  unsigned char *aa,myqq,zz;
  unsigned long oo;
  
  // file.ff myqq out.h
  myqq=atoi(argv[2]);
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);

  fp=fopen(argv[3],"wb");
  fwrite(&myqq,1,1,fp);
  for(j=0;j<3;j++){
    aa=F+16+j*2;
    for(m=0;m<512;m++){
      oo=0;
      for(n=0;n<8;n++){
        zz=(*aa)>>4;
        aa+=8;
        oo|=zz;
        if(n<7)oo<<=4;
      }
      fwrite(&oo,4,1,fp);
    }
  }
  fclose(fp);
}
