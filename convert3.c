#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc,char **argv){
  unsigned char F[32784];
  FILE *fp;
  int m,j;
  unsigned char *aa,myqq,zz;
  
  // file.ff myqq out.h
  myqq=atoi(argv[2]);
  fp=fopen(argv[1],"rb");
  fread(F,32784,1,fp);
  fclose(fp);

  fp=fopen(argv[3],"wb");
  fwrite(&myqq,1,1,fp);
  for(j=0;j<3;j++){
    aa=F+16+j*2;
    for(m=0;m<2048;m++){
      zz=((*aa++)>>4)<<4;
      zz|=(*aa++)>>4;
      fwrite(&zz,1,1,fp);
    }
  }
  fclose(fp);
}
