// OE=13 LAT=12 CLK=11
// A=10 B=9 C=8 D=7 E=6
// B2=5 B1=4 R2=3 R1=2 G2=1 G1=0

// manual 19.2.5
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

int row,refresh,i,j,q1,q2;
unsigned long zr1,zr2,zg1,zg2,zb1,zb2;
unsigned long *pr1,*pr2,*pg1,*pg2,*pb1,*pb2,*myMM;

void setup() {
  refresh=0;
  q1=1000;
  q2=2;
  for(i=0;i<=13;i++)pinMode(i,OUTPUT);
}

void loop() {
  if(--q1==0){
    q1=1000;
    if(--q2<0)q2=2;
  }
  if(q2==2)pr1=MM1[refresh];
  else if(q2==1)pr1=MM2[refresh];
  else if(q2==0)pr1=MM3[refresh];
  
  pr2=pr1+64;
  pg1=pr2+64; pg2=pg1+64;
  pb1=pg2+64; pb2=pb1+64;

  for(row=0;row<32;row++){
    D13off
    for(j=0;j<2;j++){
      zr1=*pr1++; zr2=*pr2++;
      zg1=*pg1++; zg2=*pg2++;
      zb1=*pb1++; zb2=*pb2++;
  
      for(i=0;i<32;i++){
        if(zr1&0b10000000000000000000000000000000)D2on else D2off zr1<<=1;
        if(zr2&0b10000000000000000000000000000000)D3on else D3off zr2<<=1;
        if(zg1&0b10000000000000000000000000000000)D0on else D0off zg1<<=1;
        if(zg2&0b10000000000000000000000000000000)D1on else D1off zg2<<=1;
        if(zb1&0b10000000000000000000000000000000)D4on else D4off zb1<<=1;
        if(zb2&0b10000000000000000000000000000000)D5on else D5off zb2<<=1;
        D11off
        D11on
      }
    }

    if(row&0x10)D6on else D6off
    if(row&0x08)D7on else D7off
    if(row&0x04)D8on else D8off
    if(row&0x02)D9on else D9off
    if(row&0x01)D10on else D10off
    D12on
    D12off
    D13on
  }
  if(++refresh==elm)refresh=0;
}
