#include <WiFi.h>
#include <SPI.h>

// OE=D13=GPIO_NUM_48 LAT=D12=GPIO_NUM_47 CLK=11=GPIO_NUM_38
// A=D10=GPIO_NUM_21 B=D9=GPIO_NUM_18 C=D8=GPIO_NUM_17 D=D7=GPIO_NUM_10 E=D6=GPIO_NUM_9
// B2=D5=GPIO_NUM_8 B1=D4=GPIO_NUM_7
// R2=D3=GPIO_NUM_6 R1=D2=GPIO_NUM_5 
// G2=D20=A3=GPIO_NUM_4 G1=D19=A2=GPIO_NUM_3

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

int row,refresh,i,j,k1,k2,myqq,n;
unsigned long zr1,zr2,zg1,zg2,zb1,zb2;
unsigned long *pr1,*pr2,*pg1,*pg2,*pb1,*pb2,tt,ta,oo;
unsigned char c,buf[6144],*aa;
WiFiClient client;
IPAddress ip;
unsigned long MM[15][384];

unsigned char TTl[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};
unsigned char TTh[]={0,16,32,32,64,64,64,64,128,128,128,128,128,128,128,128};

int mywait(){
  unsigned long r=millis();
  while(client.available()==0 && millis()-tt<1000);
  if(client.available())return 0;
  else return 1;
}

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

  if(++refresh>=15){
    refresh=0;
    ta=millis();
    if(ta-tt>1000*myqq){
      tt=ta;
      ip=WiFi.localIP();
      client.stop();
      client.connect("matrix.lepida.it",80);
      client.print("GET /matrix.php?ip=");
      client.print(ip);
      client.println(" HTTP/1.1");
      client.println("Host: matrix.lepida.it");
      client.println("User-Agent: ArduinoWiFi/1.1");
      client.println("Connection: close");
      client.println();
      
      k1=0;
      for(;;){
        if(mywait())goto mybreak;
        c=client.read();
        Serial.write(c);
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
          buf[i]=client.read();
        }
        if(c>0)myqq=c;
        Serial.print("tt:"); Serial.println(tt);
        Serial.print("myqq:"); Serial.println(myqq);
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
            MM[k1][k2]=oo;
          }
        }
      }
      mybreak:
      ;
    }
  }
}

void setup() {
  Serial.begin(9600);
  refresh=0;
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

  WiFi.begin("EmiliaRomagnaWiFi wifiprivacy.it");
  delay(4000);
  for(;;){
    Serial.println("Connecting...");
    delay(1000);
    if(WiFi.status()==WL_CONNECTED)break;
  }
  tt=millis();
}
