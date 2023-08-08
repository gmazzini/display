#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "font_0.h"

// from https://fontstruct.com/fontstructions/show/847768/5x7_dot_matrix
char fA[7]={0b00100000,0b01010000,0b10001000,0b10001000,0b11111000,0b10001000,0b10001000};
char fB[7]={0b11110000,0b01001000,0b01001000,0b01110000,0b01001000,0b01001000,0b11110000};
char fC[7]={0b01110000,0b10001000,0b10000000,0b10000000,0b10000000,0b10001000,0b01110000};
char fD[7]={0b11110000,0b01001000,0b01001000,0b01001000,0b01001000,0b01001000,0b11110000};
char fE[7]={0b11111000,0b10000000,0b10000000,0b11110000,0b10000000,0b10000000,0b11111000};
char fF[7]={0b11111000,0b10000000,0b10000000,0b11110000,0b10000000,0b10000000,0b10000000};
char fG[7]={0b01110000,0b10001000,0b10000000,0b10011000,0b10001000,0b10001000,0b01111000};
char fH[7]={0b10001000,0b10001000,0b10001000,0b11111000,0b10001000,0b10001000,0b10001000};
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
char f32[7]={0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000};
char fa[7]={0b00000000,0b00000000,0b01110000,0b00001000,0b01111000,0b10001000,0b01111000};
char fb[7]={0b10000000,0b10000000,0b10110000,0b11001000,0b10001000,0b10001000,0b11110000};
char fc[7]={0b00000000,0b00000000,0b01110000,0b10000000,0b10000000,0b10001000,0b01110000};
char fd[7]={0b00001000,0b00001000,0b01101000,0b10011000,0b10001000,0b10001000,0b01111000};
char fe[7]={0b00000000,0b00000000,0b01110000,0b10001000,0b11111000,0b10000000,0b01110000};
char ff[7]={0b00110000,0b01001000,0b01000000,0b11100000,0b01000000,0b01000000,0b01000000};
char fg[7]={0b00000000,0b00000000,0b01111000,0b10001000,0b01111000,0b00001000,0b01110000};
char fh[7]={0b10000000,0b10000000,0b10110000,0b11001000,0b10001000,0b10001000,0b10001000};
char fi[7]={0b00100000,0b00000000,0b00100000,0b01100000,0b00100000,0b00100000,0b01110000};
char fj[7]={0b00010000,0b00000000,0b00110000,0b00010000,0b00010000,0b10010000,0b01100000};
char fk[7]={0b10000000,0b10000000,0b10010000,0b10100000,0b11000000,0b10100000,0b10010000};
char fl[7]={0b01100000,0b00100000,0b00100000,0b00100000,0b00100000,0b00100000,0b01110000};
char fm[7]={0b00000000,0b00000000,0b11010000,0b10101000,0b10101000,0b10101000,0b10101000};
char fn[7]={0b00000000,0b00000000,0b10110000,0b11001000,0b10001000,0b10001000,0b10001000};
char fo[7]={0b00000000,0b00000000,0b01110000,0b10001000,0b10001000,0b10001000,0b01110000};
char fp[7]={0b00000000,0b00000000,0b11110000,0b10001000,0b11110000,0b10000000,0b10000000};
char fq[7]={0b00000000,0b00000000,0b01101000,0b10011000,0b01111000,0b00001000,0b00001000};
char fr[7]={0b00000000,0b00000000,0b10110000,0b11001000,0b10000000,0b10000000,0b10000000};
char fs[7]={0b00000000,0b00000000,0b01110000,0b10000000,0b01110000,0b00001000,0b11110000};
char ft[7]={0b01000000,0b01000000,0b11100000,0b01000000,0b01000000,0b01001000,0b00110000};
char fu[7]={0b00000000,0b00000000,0b10001000,0b10001000,0b10001000,0b10011000,0b01101000};
char fv[7]={0b00000000,0b00000000,0b10001000,0b10001000,0b10001000,0b01010000,0b00100000};
char fw[7]={0b00000000,0b00000000,0b10001000,0b10001000,0b10101000,0b10101000,0b01010000};
char fx[7]={0b00000000,0b00000000,0b10001000,0b01010000,0b00100000,0b01010000,0b10001000};
char fy[7]={0b00000000,0b00000000,0b10001000,0b10001000,0b01111000,0b00001000,0b01110000};
char fz[7]={0b00000000,0b00000000,0b11111000,0b00010000,0b00100000,0b01000000,0b11111000};
char f33[7]={0b00100000,0b00100000,0b00100000,0b00100000,0b00100000,0b00000000,0b00100000};
char f34[7]={0b01010000,0b01010000,0b01010000,0b00000000,0b00000000,0b00000000,0b00000000};
char f35[7]={0b01010000,0b01010000,0b11111000,0b01010000,0b11111000,0b01010000,0b01010000};
char f36[7]={0b00100000,0b01111000,0b10100000,0b01110000,0b00101000,0b11110000,0b00100000};
char f37[7]={0b11000000,0b11001000,0b00010000,0b00100000,0b01000000,0b10011000,0b00011000};
char f38[7]={0b01100000,0b10010000,0b10100000,0b01000000,0b10101000,0b10010000,0b01101000};
char f39[7]={0b00100000,0b00100000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000};
char f40[7]={0b00010000,0b00100000,0b01000000,0b01000000,0b01000000,0b00100000,0b00010000};
char f41[7]={0b01000000,0b00100000,0b00010000,0b00010000,0b00010000,0b00100000,0b01000000};
char f42[7]={0b00000000,0b00100000,0b10101000,0b01110000,0b10101000,0b00100000,0b00000000};
char f43[7]={0b00000000,0b00100000,0b00100000,0b11111000,0b00100000,0b00100000,0b00000000};
char f44[7]={0b00000000,0b00000000,0b00000000,0b00000000,0b01100000,0b00100000,0b01000000};
char f45[7]={0b00000000,0b00000000,0b00000000,0b11111000,0b00000000,0b00000000,0b00000000};
char f46[7]={0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01100000,0b01100000};
char f47[7]={0b00000000,0b00001000,0b00010000,0b00100000,0b01000000,0b10000000,0b00000000};
char f58[7]={0b00000000,0b01100000,0b01100000,0b00000000,0b01100000,0b01100000,0b00000000};
char f59[7]={0b00000000,0b01100000,0b01100000,0b00000000,0b01100000,0b00100000,0b01000000};
char f60[7]={0b00010000,0b00100000,0b01000000,0b10000000,0b01000000,0b00100000,0b00010000};
char f61[7]={0b00000000,0b00000000,0b11111000,0b00000000,0b11111000,0b00000000,0b00000000};
char f62[7]={0b01000000,0b00100000,0b00010000,0b00001000,0b00010000,0b00100000,0b01000000};
char f63[7]={0b01110000,0b10001000,0b00001000,0b00010000,0b00100000,0b00000000,0b00100000};
char f64[7]={0b01110000,0b10001000,0b00001000,0b01101000,0b10101000,0b10101000,0b01110000};
char f91[7]={0b01110000,0b01000000,0b01000000,0b01000000,0b01000000,0b01000000,0b01110000};
char f92[7]={0b00000000,0b10000000,0b01000000,0b00100000,0b00010000,0b00001000,0b00000000};
char f93[7]={0b01110000,0b00010000,0b00010000,0b00010000,0b00010000,0b00010000,0b01110000};
char f94[7]={0b00100000,0b01010000,0b10001000,0b00000000,0b00000000,0b00000000,0b00000000};
char f95[7]={0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11111000};
char f96[7]={0b01000000,0b00100000,0b00010000,0b00000000,0b00000000,0b00000000,0b00000000};
char f123[7]={0b00010000,0b00100000,0b00100000,0b01000000,0b00100000,0b00100000,0b00010000};
char f124[7]={0b00100000,0b00100000,0b00100000,0b00100000,0b00100000,0b00100000,0b00100000};
char f125[7]={0b01000000,0b00100000,0b00100000,0b00010000,0b00100000,0b00100000,0b01000000};
char f126[7]={0b00000000,0b00000000,0b00000000,0b01101000,0b10010000,0b00000000,0b00000000};

char *mf[128]={
  f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,
  f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,f32,
  f32,f33,f34,f35,f36,f37,f38,f39,f40,f41,f42,f43,f44,f45,f46,f37,
  f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f58,f59,f60,f61,f62,f63,
  f64,fA,fB,fC,fD,fE,fF,fG,fH,fI,fJ,fK,fL,fM,fN,fO,
  fP,fQ,fR,fS,fT,fU,fV,fW,fX,fY,fZ,f91,f92,f93,f94,f95,
  f96,fa,fb,fc,fd,fe,ff,fg,fh,fi,fj,fk,fl,fm,fn,fo,
  fp,fq,fr,fs,ft,fu,fv,fw,fx,fy,fz,f123,f124,f125,f126,f32
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
  unsigned char F[32784],*a;
  char buf[100];
  unsigned int cc,*c;
  FILE *fp;
  unsigned int x,y,n,m,r,g,b,l,k,v,w,rb,gb,bb;

  memcpy(F,"farbfeld",8);
  memcpy(F+8,"\x00\x00\x00\x40",4);
  memcpy(F+12,"\x00\x00\x00\x40",4);

  fp=fopen("gm6.des","rb");
  fgets(buf,100,fp);
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
    *(buf+5)='\0';
    *(buf+12)='\0';
    x=atoi(buf);
    y=atoi(buf+3);
    r=hextable[*(buf+6)]<<4|hextable[*(buf+7)];
    g=hextable[*(buf+8)]<<4|hextable[*(buf+9)];
    b=hextable[*(buf+10)]<<4|hextable[*(buf+11)];
    l=strlen(buf+13)-1;
    
    printf("%d %d %d %d %d %d\n",x,y,r,g,b,l);
    
    for(k=0;k<l;k++){
      // c=mf[(*(buf+13+k))&0x7f];
      c=font_0[(*(buf+13+k)-31)&0x7f];
      for(n=0;n<7;n++){
        cc=c[n];
        for(m=0;m<5;m++){
          if(cc&0x8000){
       //   if(cc&0x80){
            v=x+m+k*6;
            w=y+n;
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
    }
  }
  fclose(fp);

  fp=fopen("gm6.ff","wb");
  fwrite(F,32784,1,fp);
  fclose(fp);
}
