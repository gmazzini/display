#include "stdio.h"
#include "stdlib.h"
#include "string.h"

char fA[7]={0b00100000,0b01010000,0b10001000,0b00010000,0b11111000,0b10001000,0b10001000};
char fB[7]={0b11110000,0b01001000,0b01001000,0b01110000,0b01001000,0b01001000,0b11110000};
char fC[7]={0b01110000,0b10001000,0b10000000,0b10000000,0b10000000,0b10001000,0b01110000};
char fD[7]={0b11110000,0b01001000,0b01001000,0b01001000,0b01001000,0b01001000,0b11110000};
char fE[7]={0b11111000,0b10000000,0b10000000,0b11110000,0b10000000,0b10000000,0b11111000};
char fF[7]={0b11111000,0b10000000,0b10000000,0b11110000,0b10000000,0b10000000,0b10000000};
char fG[7]={0b01110000,0b10001000,0b10000000,0b10011000,0b10001000,0b10001000,0b01111000};
char fH[7]={0b10001000,0b10000000,0b10000000,0b11111000,0b10001000,0b10001000,0b10001000};
char fI[7]={0b01110000,0b00100000,0b00100000,0b00100000,0b00100000,0b00100000,0b01110000};
char fJ[7]={0b00111000,0b00010000,0b00010000,0b00010000,0b00010000,0b10010000,0b01100000};
char fK[7]={0b10001000,0b10010000,0b10100000,0b11000000,0b10100000,0b10010000,0b10001000};
char fL[7]={0b10000000,0b10000000,0b10000000,0b10000000,0b10000000,0b10000000,0b11111000};
char fM[7]={0b10001000,0b11011000,0b10101000,0b10101000,0b10001000,0b10001000,0b10001000};
char fN[7]={0b10001000,0b10001000,0b11001000,0b10101000,0b10011000,0b10001000,0b10001000};
char fO[7]={0b01110000,0b10001000,0b10001000,0b10001000,0b10001000,0b10001000,0b01110000};
char fP[7]={0b11110000,0b10001000,0b10001000,0b11110000,0b10000000,0b10000000,0b10000000};
char fQ[7]={0b01110000,0b10001000,0b10001000,0b10001000,0b10101000,0b10010000,0b01101000};
char fR[7]={0b11110000,0b10001000,0b10001000,0b11110000,0b10100000,0b10010000,0b10001000};
char fS[7]={0b01110000,0b10001000,0b10000000,0b01110000,0b00001000,0b10001000,0b01110000};
char fT[7]={0b11111000,0b00100000,0b00100000,0b00100000,0b00100000,0b00100000,0b00100000};
char fU[7]={0b10001000,0b10001000,0b10001000,0b10001000,0b10001000,0b10001000,0b01110000};
char fV[7]={0b10001000,0b10001000,0b10001000,0b10001000,0b10001000,0b01010000,0b00100000};
char fW[7]={0b10001000,0b10001000,0b10001000,0b10101000,0b10101000,0b10101000,0b01010000};
char fX[7]={0b10001000,0b10001000,0b01010000,0b00100000,0b01010000,0b10001000,0b10001000};
char fY[7]={0b10001000,0b10001000,0b10001000,0b01010000,0b00100000,0b00100000,0b00100000};
char fZ[7]={0b11111000,0b00001000,0b00010000,0b00100000,0b01000000,0b10000000,0b11111000};
char f0[7]={0b01110000,0b10001000,0b10011000,0b10101000,0b11001000,0b10001000,0b01110000};
char f1[7]={0b00100000,0b01100000,0b00100000,0b00100000,0b00100000,0b00100000,0b01110000};
char f2[7]={0b01110000,0b10001000,0b00001000,0b00110000,0b01000000,0b10000000,0b11111000};
char f3[7]={0b01110000,0b10001000,0b00001000,0b00110000,0b00001000,0b10001000,0b01110000};
char f4[7]={0b00010000,0b00110000,0b01010000,0b10010000,0b11111000,0b00010000,0b00010000};
char f5[7]={0b11111000,0b10000000,0b11110000,0b00001000,0b00001000,0b10001000,0b01110000};
char f6[7]={0b00110000,0b01000000,0b10000000,0b11110000,0b10001000,0b10001000,0b01110000};
char f7[7]={0b11111000,0b00001000,0b00010000,0b00100000,0b01000000,0b01000000,0b01000000};
char f8[7]={0b01110000,0b10001000,0b10001000,0b01110000,0b10001000,0b10001000,0b01110000};
char f9[7]={0b01110000,0b10001000,0b10001000,0b01111000,0b00001000,0b00010000,0b01100000};
char fSP[7]={0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000};

char *ff[128]={
  fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,\
  fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,\
  fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,fSP,\
  f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,fSP,fSP,fSP,fSP,fSP,fSP,\
  fSP,fA,fB,fC,fD,fE,fF,fG,fH,fI,fJ,fK,fL,fM,fN,fO,\
  fP,fQ,fR,fS,fT,fU,fV,fW,fX,fY,fZ,fSP,fSP,fSP,fSP,fSP,\
  fSP,fA,fB,fC,fD,fE,fF,fG,fH,fI,fJ,fK,fL,fM,fN,fO,\
  fP,fQ,fR,fS,fT,fU,fV,fW,fX,fY,fZ,fSP,fSP,fSP,fSP,fSP
};

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
int main(){
  unsigned char F[32784],*a,*c;
  char buf[100];
  FILE *fp;
  unsigned int x,y,n,m,r,g,b,l;

  memcpy(F,"farbfeld",8);
  memcpy(F+8,"\x00\x00\x00\x40",4);
  memcpy(F+12,"\x00\x00\x00\x40",4);

  a=F+16;
  for(n=0;n<4096;n++){
    *(a++)=0x00; *(a++)=0x00;
    *(a++)=0x00; *(a++)=0x00;
    *(a++)=0x00; *(a++)=0x00;
    *(a++)=0xff; *(a++)=0xff;
  }
  
  fp=fopen("gm6.des","rb");
  for(;;){
    fgets(buf,100,fp);
    if(feof(fp))break;
  
    *(buf+2)='\0';
    *(buf+5)='\0';
    *(buf+12)='\0';
    x=atoi(buf);
    y=atoi(buf+3);
    r=hextable[*(buf+6)]<<4|hextable[*(buf+7)];
    g=hextable[*(buf+8)]<<4|hextable[*(buf+9)];
    b=hextable[*(buf+10)]<<4|hextable[*(buf+11)];
    l=strlen(buf+13);
    
    printf("%d %d %d %d %d %d\n",x,y,r,g,b,l);
    
// fare check non overbound
    c=ff[*(buf+13)];
    for(n=0;n<7;n++){
      for(m=0;m<5;m++){
        a=F+16+((y+n)<<6+(x+m))<<3;
        *(a+0)=r;
        *(a+2)=g;
        *(a+4)=b;
      }
    }
    

    
  }
  fclose(fp);

  fp=fopen("gm6.ff","wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
}
