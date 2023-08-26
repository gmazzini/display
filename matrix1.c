// arduino UNO R4 Minima

// OE=13 LAT=12 CLK=11
// A=10 B=9 C=8 D=7 E=6
// B2=5 B1=4 R2=3 R1=2 G2=1 G1=0

#define PORTBASE 0x40040000
#define PFS_P301PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (01 * 4))) // D0 / RxD (P301)
#define PFS_P302PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (02 * 4))) // D1 / TxD (P302) 
#define PFS_P105PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 5 * 4))) // D2
#define PFS_P104PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 4 * 4))) // D3
#define PFS_P103PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 3 * 4))) // D4
#define PFS_P102PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 2 * 4))) // D5
#define PFS_P106PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 6 * 4))) // D6
#define PFS_P107PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 7 * 4))) // D7
#define PFS_P304PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (04 * 4))) // D8
#define PFS_P303PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (03 * 4))) // D9
#define PFS_P112PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (12 * 4))) // D10 / CS
#define PFS_P109PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 9 * 4))) // D11 / MOSI
#define PFS_P110PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (10 * 4))) // D12 / MISO
#define PFS_P111PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (11 * 4))) // D13 / SCLK
#define D0on *PFS_P301PFS_BY = 0x05;
#define D0off *PFS_P301PFS_BY = 0x04;
#define D1on *PFS_P302PFS_BY = 0x05;
#define D1off *PFS_P302PFS_BY = 0x04;
#define D2on *PFS_P105PFS_BY = 0x05;
#define D2off *PFS_P105PFS_BY = 0x04;
#define D3on *PFS_P104PFS_BY = 0x05;
#define D3off *PFS_P104PFS_BY = 0x04;
#define D4on *PFS_P103PFS_BY = 0x05;
#define D4off *PFS_P103PFS_BY = 0x04;
#define D5on *PFS_P102PFS_BY = 0x05;
#define D5off *PFS_P102PFS_BY = 0x04;
#define D6on *PFS_P106PFS_BY = 0x05;
#define D6off *PFS_P106PFS_BY = 0x04;
#define D7on *PFS_P107PFS_BY = 0x05;
#define D7off *PFS_P107PFS_BY = 0x04;
#define D8on *PFS_P304PFS_BY = 0x05;
#define D8off *PFS_P304PFS_BY = 0x04;
#define D9on *PFS_P303PFS_BY = 0x05;
#define D9off *PFS_P303PFS_BY = 0x04;
#define D10on *PFS_P112PFS_BY = 0x05;
#define D10off *PFS_P112PFS_BY = 0x04;
#define D11on *PFS_P109PFS_BY = 0x05;
#define D11off *PFS_P109PFS_BY = 0x04;
#define D12on *PFS_P110PFS_BY = 0x05;
#define D12off *PFS_P110PFS_BY = 0x04;
#define D13on *PFS_P111PFS_BY = 0x05;
#define D13off *PFS_P111PFS_BY = 0x04;

#include "h1.h"
char m1[16]={1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};
char m2[16]={16,32,32,64,64,64,64,128,128,128,128,128,128,128,128};
int i,j,z=0;
char *r1,*r2,*g1,*g2,*b1,*b2,zm1,zm2,ar1,ar2,ag1,ag2,ab1,ab2;

char *MM;

void setup() {
  for(i=0;i<=13;i++)pinMode(i,OUTPUT);
}

void loop() {
  r1=MM;
  r2=MM+1024;
  g1=MM+2048;
  g2=MM+3072;
  b1=MM+4096;
  b2=MM+5120;
  zm1=*(m1+z);
  zm2=*(m2+z);
  
  for(j=0;j<32;j++){
    D13off

    if(j&0x10)D6on else D6off
    if(j&0x08)D7on else D7off
    if(j&0x04)D8on else D8off
    if(j&0x02)D9on else D9off
    if(j&0x01)D10on else D10off

    for(i=0;i<32;i++){
      ar1=*r1; ar2=*r2; r1++; r2++;
      ag1=*g1; ag2=*g2; g1++; g2++;
      ab1=*b1; ab2=*b2; b1++; b2++;

      if(ar1&zm1)D2on else D2off
      if(ar2&zm1)D3on else D3off
      if(ab1&zm1)D4on else D4off
      if(ab2&zm1)D5on else D5off
      if(ag1&zm1)D0on else D0off
      if(ag2&zm1)D1on else D1off
      D11off
      D11on

      if(ar1&zm2)D2on else D2off
      if(ar2&zm2)D3on else D3off
      if(ab1&zm2)D4on else D4off
      if(ab2&zm2)D5on else D5off
      if(ag1&zm2)D0on else D0off
      if(ag2&zm2)D1on else D1off
      D11off
      D11on

    }

    D12on
    D12off
    D13on
  }

  z++;
  if(z==16)z=0;
}
