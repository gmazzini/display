#include <WiFi.h>
#include <SPI.h>
#define myqq 10

// OE=D13=GPIO_NUM_48 LAT=D12=GPIO_NUM_47 CLK=11=GPIO_NUM_38
// A=D10=GPIO_NUM_21 B=D9=GPIO_NUM_18 C=D8=GPIO_NUM_17 D=D7=GPIO_NUM_10 E=D6=GPIO_NUM_9
// B2=D5=GPIO_NUM_8 B1=D4=GPIO_NUM_7
// R2=D3=GPIO_NUM_6 R1=D2=GPIO_NUM_5 
// G2=D1=GPIO_NUM_4 G1=D0=GPIO_NUM_3

#define pOE GPIO_NUM_48
#define pLAT GPIO_NUM_47
#define pCLK GPIO_NUM_38
#define pA GPIO_NUM_21
#define pB GPIO_NUM_18
#define pC GPIO_NUM_17
#define pD GPIO_NUM_10
#define pE GPIO_NUM_9
#define pB2 GPIO_NUM_8
#define pB1 GPIO_NUM_7
#define pR2 GPIO_NUM_6
#define pR1 GPIO_NUM_5
#define pG2 GPIO_NUM_4
#define pG1 GPIO_NUM_3

#define pCLKh GPIO.out1_w1ts.val = ((uint32_t)1 << (38-32));
#define pCLKl GPIO.out1_w1tc.val = ((uint32_t)1 << (38-32));
#define pLATh GPIO.out1_w1ts.val = ((uint32_t)1 << (47-32));
#define pLATl GPIO.out1_w1tc.val = ((uint32_t)1 << (47-32));
#define pOEh GPIO.out1_w1ts.val = ((uint32_t)1 << (48-32));
#define pOEl GPIO.out1_w1tc.val = ((uint32_t)1 << (48-32));
#define pAh GPIO.out_w1ts = ((uint32_t)1 << 21);
#define pAl GPIO.out_w1tc = ((uint32_t)1 << 21);
#define pBh GPIO.out_w1ts = ((uint32_t)1 << 18);
#define pBl GPIO.out_w1tc = ((uint32_t)1 << 18);
#define pCh GPIO.out_w1ts = ((uint32_t)1 << 17);
#define pCl GPIO.out_w1tc = ((uint32_t)1 << 17);
#define pDh GPIO.out_w1ts = ((uint32_t)1 << 10);
#define pDl GPIO.out_w1tc = ((uint32_t)1 << 10);
#define pEh GPIO.out_w1ts = ((uint32_t)1 << 9);
#define pEl GPIO.out_w1tc = ((uint32_t)1 << 9);
#define pB2h GPIO.out_w1ts = ((uint32_t)1 << 8);
#define pB2l GPIO.out_w1tc = ((uint32_t)1 << 8);
#define pB1h GPIO.out_w1ts = ((uint32_t)1 << 7);
#define pB1l GPIO.out_w1tc = ((uint32_t)1 << 7);
#define pR2h GPIO.out_w1ts = ((uint32_t)1 << 6);
#define pR2l GPIO.out_w1tc = ((uint32_t)1 << 6);
#define pR1h GPIO.out_w1ts = ((uint32_t)1 << 5);
#define pR1l GPIO.out_w1tc = ((uint32_t)1 << 5);
#define pG2h GPIO.out_w1ts = ((uint32_t)1 << 4);
#define pG2l GPIO.out_w1tc = ((uint32_t)1 << 4);
#define pG1h GPIO.out_w1ts = ((uint32_t)1 << 3);
#define pG1l GPIO.out_w1tc = ((uint32_t)1 << 3);

#include "h1.h"

int row,refresh,i,j,k1,k2,qq;
unsigned long zr1,zr2,zg1,zg2,zb1,zb2;
unsigned long *pr1,*pr2,*pg1,*pg2,*pb1,*pb2,tt,ta,*pp;
WiFiClient client;
unsigned char c,buf[4];

void loop(){
  pr1=MM[refresh];
  pr2=pr1+64;
  pg1=pr2+64; pg2=pg1+64;
  pb1=pg2+64; pb2=pb1+64;

  for(row=0;row<32;row++){
    pOEl
    for(j=0;j<2;j++){
      zr1=*pr1++; zr2=*pr2++;
      zg1=*pg1++; zg2=*pg2++;
      zb1=*pb1++; zb2=*pb2++;
  
      for(i=0;i<32;i++){
        if(zr1&0b10000000000000000000000000000000)pR1h else pR1l zr1<<=1;
        if(zr2&0b10000000000000000000000000000000)pR2h else pR2l zr2<<=1;
        if(zg1&0b10000000000000000000000000000000)pG1h else pG1l zg1<<=1;
        if(zg2&0b10000000000000000000000000000000)pG2h else pG2l zg2<<=1;
        if(zb1&0b10000000000000000000000000000000)pB1h else pB1l zb1<<=1;
        if(zb2&0b10000000000000000000000000000000)pB2h else pB2l zb2<<=1;
        pCLKl
        pCLKh
      }
    }

    if(row&0x10)pEh else pEl
    if(row&0x08)pDh else pDl
    if(row&0x04)pCh else pCl
    if(row&0x02)pBh else pBl
    if(row&0x01)pAh else pAl
    pLATh
    pLATl
    pOEh
  }
  if(++refresh>=elm){
    refresh=0;
    ta=millis();
    if(ta-tt>3000){
      tt=ta;
      Serial.print("qq:"); Serial.println(qq);
      if(qq==0){
        client.stop();
        client.connect("www.mazzini.org",80);
        client.println("GET /q1.bin HTTP/1.1");
        client.println("Host: www.mazzini.org");
        client.println("User-Agent: ArduinoWiFi/1.1");
        client.println("Connection: close");
        client.println();
      
        k1=0;
        for(;;){
          while(client.available()==0); c=client.read();
          Serial.write(c);
          if(k1==0 && c==0x0d){k1=1; continue;}
          if(k1==1){if(c==0x0a){k1=2; continue;} else {k1=0; continue;}}
          if(k1==2){if(c==0x0d){k1=3; continue;} else {k1=0; continue;}}
          if(k1==3){if(c==0x0a){break;} else {k1=0; continue;}}
        }
        if(k1==3){
          while(client.available()==0); c=client.read();
          if(c>0)elm=c;
          Serial.print("tt:"); Serial.println(tt);
          Serial.print("c:"); Serial.println(c);
          for(k1=0;k1<elm;k1++){
            for(k2=0;k2<384;k2++){
              while(client.available()==0); buf[0]=client.read();
              while(client.available()==0); buf[1]=client.read();
              while(client.available()==0); buf[2]=client.read();
              while(client.available()==0); buf[3]=client.read();
              pp=(unsigned long *)buf;
              MM[k1][k2]=*pp;
            }
          }
        }
      }
      qq++;
      if(qq>myqq)qq=0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  refresh=0;
  qq=1;
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
  WiFi.begin("gm10","xxxxxxx");
  for(;;){
    Serial.println("Connecting...");
    delay(10000);
    if(WiFi.status()==WL_CONNECTED)break;
  }
  tt=millis();
}
