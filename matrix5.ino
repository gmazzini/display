#include <WiFi.h>
#include <SPI.h>

// ESP32 register structs (for GPIO.out_w1ts, GPIO.out1_w1ts, etc.)
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

// Rel 20260113 by GM Copyright 2023-25
#define mySSID "EmiliaRomagnaWiFi wifiprivacy.it"
#define mySER "0108"
#define myWEB "display.mazzini.org"

// OE=D13=GPIO_NUM_48 LAT=D12=GPIO_NUM_47 CLK=11=GPIO_NUM_38
// A=D10=GPIO_NUM_21 B=D9=GPIO_NUM_18 C=D8=GPIO_NUM_17 D=D7=GPIO_NUM_10 E=D6=GPIO_NUM_9
// B2=D5=GPIO_NUM_8 B1=D4=GPIO_NUM_7
// R2=D3=GPIO_NUM_6 R1=D2=GPIO_NUM_5
// G2=D20=A3=GPIO_NUM_4 G1=D19=A2=GPIO_NUM_3

#define pOE  GPIO_NUM_48
#define pLAT GPIO_NUM_47
#define pCLK GPIO_NUM_38
#define pA   GPIO_NUM_21
#define pB   GPIO_NUM_18
#define pC   GPIO_NUM_17
#define pD   GPIO_NUM_10
#define pE   GPIO_NUM_9
#define pB2  GPIO_NUM_8
#define pB1  GPIO_NUM_7
#define pR2  GPIO_NUM_6
#define pR1  GPIO_NUM_5
#define pG2  GPIO_NUM_4
#define pG1  GPIO_NUM_3

// --- Fast GPIO set/clear macros (statement-safe, portable across Arduino-ESP32 builds) ---
#define GPIO1_BIT(gpio_num) ((uint32_t)1u << ((gpio_num) - 32))

#define SET_GPIO0(bit) do { GPIO.out_w1ts = (bit); } while(0)
#define CLR_GPIO0(bit) do { GPIO.out_w1tc = (bit); } while(0)
#define SET_GPIO1(bit) do { GPIO.out1_w1ts.val = (bit); } while(0)
#define CLR_GPIO1(bit) do { GPIO.out1_w1tc.val = (bit); } while(0)

// Precomputed bit masks for each signal (compile-time constants)
#define BIT_A   ((uint32_t)1u << 21)
#define BIT_B   ((uint32_t)1u << 18)
#define BIT_C   ((uint32_t)1u << 17)
#define BIT_D   ((uint32_t)1u << 10)
#define BIT_E   ((uint32_t)1u << 9)

#define BIT_B2  ((uint32_t)1u << 8)
#define BIT_B1  ((uint32_t)1u << 7)
#define BIT_R2  ((uint32_t)1u << 6)
#define BIT_R1  ((uint32_t)1u << 5)
#define BIT_G2  ((uint32_t)1u << 4)
#define BIT_G1  ((uint32_t)1u << 3)

// GPIO>32 live in out1_* registers:
#define BIT_CLK_1  GPIO1_BIT(38)
#define BIT_LAT_1  GPIO1_BIT(47)
#define BIT_OE_1   GPIO1_BIT(48)

// Single-operation macros (safe in if/else and safe as a line without ';')
#define pCLKh  do { SET_GPIO1(BIT_CLK_1); } while(0)
#define pCLKl  do { CLR_GPIO1(BIT_CLK_1); } while(0)
#define pLATh  do { SET_GPIO1(BIT_LAT_1); } while(0)
#define pLATl  do { CLR_GPIO1(BIT_LAT_1); } while(0)
#define pOEh   do { SET_GPIO1(BIT_OE_1); } while(0)
#define pOEl   do { CLR_GPIO1(BIT_OE_1); } while(0)

#define pAh    do { SET_GPIO0(BIT_A); } while(0)
#define pAl    do { CLR_GPIO0(BIT_A); } while(0)
#define pBh    do { SET_GPIO0(BIT_B); } while(0)
#define pBl    do { CLR_GPIO0(BIT_B); } while(0)
#define pCh    do { SET_GPIO0(BIT_C); } while(0)
#define pCl    do { CLR_GPIO0(BIT_C); } while(0)
#define pDh    do { SET_GPIO0(BIT_D); } while(0)
#define pDl    do { CLR_GPIO0(BIT_D); } while(0)
#define pEh    do { SET_GPIO0(BIT_E); } while(0)
#define pEl    do { CLR_GPIO0(BIT_E); } while(0)

#define pB2h   do { SET_GPIO0(BIT_B2); } while(0)
#define pB2l   do { CLR_GPIO0(BIT_B2); } while(0)
#define pB1h   do { SET_GPIO0(BIT_B1); } while(0)
#define pB1l   do { CLR_GPIO0(BIT_B1); } while(0)
#define pR2h   do { SET_GPIO0(BIT_R2); } while(0)
#define pR2l   do { CLR_GPIO0(BIT_R2); } while(0)
#define pR1h   do { SET_GPIO0(BIT_R1); } while(0)
#define pR1l   do { CLR_GPIO0(BIT_R1); } while(0)
#define pG2h   do { SET_GPIO0(BIT_G2); } while(0)
#define pG2l   do { CLR_GPIO0(BIT_G2); } while(0)
#define pG1h   do { SET_GPIO0(BIT_G1); } while(0)
#define pG1l   do { CLR_GPIO0(BIT_G1); } while(0)

#define MASK_ADDR (BIT_A|BIT_B|BIT_C|BIT_D|BIT_E)
#define IP_IS_ZERO(ip) ((ip)[0]==0 && (ip)[1]==0 && (ip)[2]==0 && (ip)[3]==0)

int row,refresh,i,j,k1,k2,myqq,n,valid;
unsigned long lastWifiTry=0;
unsigned long zr1,zr2,zg1,zg2,zb1,zb2;
volatile unsigned long *pr1,*pr2,*pg1,*pg2,*pb1,*pb2;
unsigned long tt,ta,oo;
unsigned char c,buf[6144],*aa;
WiFiClient client;
IPAddress ip;
unsigned long MM_A[15][384];
unsigned long MM_B[15][384];
volatile unsigned long (*front)[384]=MM_A;
volatile unsigned long (*back)[384]=MM_B;
volatile unsigned long (*swapTmp)[384];
unsigned char TTl[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};
unsigned char TTh[]={0,16,32,32,64,64,64,64,128,128,128,128,128,128,128,128};
uint32_t rowSet[32],rowClr[32];
IPAddress dispIP;
unsigned long lastDns=0;
const unsigned long DNS_TTL=300000UL;

int mywait(){
  unsigned long start=millis();
  while(client.available()==0 && (millis()-start) < 1000){ delay(1); }
  return client.available()? 0 : 1;
}

void loop(){
  int ok=0,v;
  static IPAddress tmp;
  uint32_t mask;

  pr1=front[refresh];
  pr2=pr1+64;
  pg1=pr2+64; pg2=pg1+64;
  pb1=pg2+64; pb2=pb1+64;

  for(row=0;row<32;row++){

    // ---- FIX1 GHOSTING (minimal):
    // Blank (OE disabled) while shifting + setting address + latching
    pOEh

    for(j=0;j<2;j++){
      zr1=*pr1++; zr2=*pr2++;
      zg1=*pg1++; zg2=*pg2++;
      zb1=*pb1++; zb2=*pb2++;
      if(valid){
        mask=0x80000000;
        for(i=0;i<32;i++){
          if(zr1 & mask) pR1h else pR1l;
          if(zr2 & mask) pR2h else pR2l;
          if(zg1 & mask) pG1h else pG1l;
          if(zg2 & mask) pG2h else pG2l;
          if(zb1 & mask) pB1h else pB1l;
          if(zb2 & mask) pB2h else pB2l;
          pCLKl
          pCLKh
          mask >>= 1;
        }
      }
      else {
        pR1l
        pR2l
        pG1l
        pG2l
        pB1l
        pB2l
        for(i=0;i<32;i++){
          pCLKl
          pCLKh
        }
      }
    }

    GPIO.out_w1tc=rowClr[row];
    GPIO.out_w1ts=rowSet[row];

    pLATh
    pLATl

    // Enable output only AFTER latch (OE is typically active-low on HUB75)
    pOEl
  }

  if(++refresh>=15){
    refresh=0;
    yield();
    ta=millis();
    if(ta-tt>1000*myqq){
      tt=ta;
      ok=0;
      if(WiFi.status()!=WL_CONNECTED){
        if(millis()-lastWifiTry>8000){
          lastWifiTry=millis();
          WiFi.disconnect(false);
          WiFi.begin(mySSID);
        }
        goto mybreak;
      }
      ip=WiFi.localIP();
      client.stop();
      client.setTimeout(1500);
      if(IP_IS_ZERO(dispIP) || (millis()-lastDns)>DNS_TTL){
        if(WiFi.hostByName(myWEB,tmp)){
          dispIP=tmp;
          lastDns=millis();
        }
        else if(IP_IS_ZERO(dispIP))goto mybreak;
      }
      if(!client.connect(dispIP,80))goto mybreak;
      client.print("GET /?ser=");
      client.print(mySER);
      client.print("&ip=");
      client.print(ip);
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(myWEB);
      client.println("User-Agent: ArduinoWiFi/1.1");
      client.println("Connection: close");
      client.println();

      k1=0;
      for(;;){
        if(mywait())goto mybreak;
        c=client.read();
        if(k1==0 && c==0x0d){k1=1; continue;}
        if(k1==1){if(c==0x0a){k1=2; continue;} else {k1=0; continue;}}
        if(k1==2){if(c==0x0d){k1=3; continue;} else {k1=0; continue;}}
        if(k1==3){if(c==0x0a){break;} else {k1=0; continue;}}
      }
      if(k1==3){
        if(mywait())goto mybreak;
        c=client.read();
        for(i=0;i<6144;i++){
          if(mywait())goto mybreak;
          v=client.read();
          if(v<0)goto mybreak;
          buf[i]=(unsigned char)v;
        }
        if(c>=1 && c<=20)myqq=c;
        for(k1=0;k1<15;k1++){
          aa=buf;
          for(k2=0;k2<384;k2++){
            oo=0;
            for(n=0;n<16;n++){
              if((*aa)&TTh[k1+1])oo|=1;
              oo<<=1;
              if((*aa)&TTl[k1+1])oo|=1;
              if(n<15)oo<<=1;
              aa++;
            }
            back[k1][k2]=oo;
          }
        }
        ok=1;
      }
mybreak:
      client.stop();
      if(ok){
        swapTmp=front;
        front=back;
        back=swapTmp;
        valid=1;
      }
    }
  }
}

void setup() {
  int r;
  uint32_t s;
  refresh=0;
  valid=0;
  myqq=2;
  setCpuFrequencyMhz(240);

  gpio_set_direction(pOE,GPIO_MODE_OUTPUT);
  gpio_set_direction(pLAT,GPIO_MODE_OUTPUT);
  gpio_set_direction(pCLK,GPIO_MODE_OUTPUT);
  gpio_set_direction(pA,GPIO_MODE_OUTPUT);
  gpio_set_direction(pB,GPIO_MODE_OUTPUT);
  gpio_set_direction(pC,GPIO_MODE_OUTPUT);
  gpio_set_direction(pD,GPIO_MODE_OUTPUT);
  gpio_set_direction(pE,GPIO_MODE_OUTPUT);
  gpio_set_direction(pR1,GPIO_MODE_OUTPUT);
  gpio_set_direction(pR2,GPIO_MODE_OUTPUT);
  gpio_set_direction(pG1,GPIO_MODE_OUTPUT);
  gpio_set_direction(pG2,GPIO_MODE_OUTPUT);
  gpio_set_direction(pB1,GPIO_MODE_OUTPUT);
  gpio_set_direction(pB2,GPIO_MODE_OUTPUT);

  for(r=0;r<32;r++){
    s=0;
    if(r & 0x01) s |= BIT_A;
    if(r & 0x02) s |= BIT_B;
    if(r & 0x04) s |= BIT_C;
    if(r & 0x08) s |= BIT_D;
    if(r & 0x10) s |= BIT_E;
    rowSet[r]=s;
    rowClr[r]=MASK_ADDR & ~s;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(mySSID);

  for(row=0;row<32;row++){
    pOEh
    for(j=0;j<2;j++){
      for(i=0;i<32;i++){
        pR1l
        pR2l
        pG1l
        pG2l
        pB1l
        pB2l
        pCLKl
        pCLKh
      }
    }
    GPIO.out_w1tc=rowClr[row];
    GPIO.out_w1ts=rowSet[row];
    pLATh
    pLATl
    pOEl
  }

  delay(4000);
  for(;;){
    if(WiFi.status()==WL_CONNECTED)break;
    delay(1000);
  }
  tt=millis();
}
