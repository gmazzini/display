// Display on ESP32S3 Rel 20260115 by GM Copyright 2023-25

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include "soc/gpio_struct.h"
#include "soc/gpio_periph.h"
#include "driver/gpio.h"
#include <Preferences.h>
#include "esp_system.h" 

#define mySSID "EmiliaRomagnaWiFi wifiprivacy.it"
#define myPUMP  650
#define myWEB  "display.mazzini.org"
#define myPGR  ""
// #define myWEB  "matrix.lepida.it"
// #define myPGR  "matrix.php"

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

#define pCLKh GPIO.out1_w1ts.val = ((uint32_t)1 << (38-32));
#define pCLKl GPIO.out1_w1tc.val = ((uint32_t)1 << (38-32));
#define pLATh GPIO.out1_w1ts.val = ((uint32_t)1 << (47-32));
#define pLATl GPIO.out1_w1tc.val = ((uint32_t)1 << (47-32));
#define pOEh  GPIO.out1_w1ts.val = ((uint32_t)1 << (48-32));
#define pOEl  GPIO.out1_w1tc.val = ((uint32_t)1 << (48-32));

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

#define MASK_A (1u<<21)
#define MASK_B (1u<<18)
#define MASK_C (1u<<17)
#define MASK_D (1u<<10)
#define MASK_E (1u<<9)
#define MASK_ADDR (MASK_A|MASK_B|MASK_C|MASK_D|MASK_E)

#define IP_IS_ZERO(ip) ((ip)[0]==0 && (ip)[1]==0 && (ip)[2]==0 && (ip)[3]==0)

Preferences prefs;
int row,refresh,i,j,k1,k2,myqq,n,valid;
unsigned long zr1,zr2,zg1,zg2,zb1,zb2;
volatile unsigned long *pr1,*pr2,*pg1,*pg2,*pb1,*pb2;
unsigned long oo;
unsigned char buf[6144],*aa;

unsigned long MM_A[15][384];
unsigned long MM_B[15][384];
volatile unsigned long (*front)[384]=MM_A;
volatile unsigned long (*back)[384]=MM_B;
volatile unsigned long (*swapTmp)[384];

unsigned char TTl[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};
unsigned char TTh[]={0,16,32,32,64,64,64,64,128,128,128,128,128,128,128,128};

uint32_t rowSet[32],rowClr[32];
char ser[9];

WiFiClient client;
IPAddress dispIP, ip, tmp;
unsigned long lastDns=0;
const unsigned long DNS_TTL=300000UL;
unsigned long lastWifiTry=0;
unsigned long tt=0;
unsigned long netT0=0;
enum NetState { NET_IDLE, NET_DNS, NET_CONNECT, NET_SEND, NET_HDR, NET_QBYTE, NET_PAYLOAD, NET_DECODE, NET_SWAP, NET_FAIL };
NetState netState = NET_IDLE;
int hdrState=0;
int payloadPos=0;
unsigned char qByte=0;
int dec_k1=0;
int dec_k2=0;
unsigned char *dec_aa=nullptr;
unsigned long dec_oo=0;
int decodeWords=0;
volatile int wantClose=0;
unsigned long pump_t0=0;
int v=0;
unsigned char ch=0;
int localBudget=0;

// -------------------- DUAL CORE: lock + task --------------------
static portMUX_TYPE swapMux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t netTaskHandle = nullptr;

// Copia atomica del puntatore front (evita leggere mentre l’altro core swappa)
static inline volatile unsigned long (*getFrontPtr())[384] {
  volatile unsigned long (*p)[384];
  portENTER_CRITICAL(&swapMux);
  p = front;
  portEXIT_CRITICAL(&swapMux);
  return p;
}
// ----------------------------------------------------------------

static void randomString(char *out, size_t len){
  const char charset[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
  size_t i;
  size_t charset_len;
  uint32_t r;
  charset_len = sizeof(charset) - 1;
  for (i = 0; i < len; i++) {
    r = esp_random();
    out[i] = charset[r % charset_len];
  }
  out[len] = '\0';
}

static inline bool budget_expired(unsigned long t0, unsigned long budget_us){
  return (unsigned long)(micros() - t0) >= budget_us;
}

static inline void drainClient(){
  while(client.available()) client.read();
}

void netResetCycle(){
  hdrState=0;
  payloadPos=0;
  qByte=0;
  netT0 = millis();
}

void netStartCycle(){
  netResetCycle();
  netState = NET_DNS;
}

void netFail(){
  client.stop();
  wantClose=0;
  netState = NET_IDLE;
}

void netPump(unsigned long budget_us){
  pump_t0 = micros();
  if(wantClose){
    client.stop();
    wantClose=0;
  }

  for(;;){
    if(budget_expired(pump_t0, budget_us)) return;

    switch(netState){

      case NET_IDLE:
        if(millis() - tt < 100UL * (unsigned long)myqq) return;
        tt = millis();
        netStartCycle();
        return;

      case NET_DNS:
        if(WiFi.status()!=WL_CONNECTED){
          if(millis()-lastWifiTry>8000){
            lastWifiTry=millis();
            WiFi.disconnect(false);
            WiFi.begin(mySSID);
          }
          netFail();
          return;
        }

        if(IP_IS_ZERO(dispIP) || (millis()-lastDns)>DNS_TTL){
          if(WiFi.hostByName(myWEB,tmp)){
            dispIP=tmp;
            lastDns=millis();
          } else {
            netFail();
            return;
          }
        }

        netState = NET_CONNECT;
        continue;

      case NET_CONNECT:
        client.setTimeout(0);
        client.setNoDelay(true);

        if(!client.connected()){
          client.stop();
          if(!client.connect(dispIP,80)){
            netFail();
            return;
          }
        }

        drainClient();
        netState = NET_SEND;
        continue;

      case NET_SEND:
        client.print("GET /");
        client.print(myPGR);
        client.print("?ser=");
        client.print(ser);
        client.print("&ip=");
        client.print(WiFi.localIP());
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(myWEB);
        client.println("User-Agent: ArduinoWiFi/1.1");
        client.println("Connection: keep-alive");
        client.println("Keep-Alive: timeout=10, max=1000");
        client.println();

        netResetCycle();
        netState = NET_HDR;
        return;

      case NET_HDR:
        if(!client.connected() && client.available()==0){ netFail(); return; }

        localBudget = 256;
        while(client.available() && localBudget--){
          v = client.read();
          if(v<0) break;
          ch = (unsigned char)v;

          if(hdrState==0 && ch==0x0d){hdrState=1; continue;}
          if(hdrState==1){ if(ch==0x0a){hdrState=2; continue;} hdrState=0; continue; }
          if(hdrState==2){ if(ch==0x0d){hdrState=3; continue;} hdrState=0; continue; }
          if(hdrState==3){
            if(ch==0x0a){
              netState = NET_QBYTE;
              break;
            }
            hdrState=0;
            continue;
          }
        }

        if(netState != NET_QBYTE){
          if(millis()-netT0 > 2500UL){ netFail(); }
          return;
        }
        continue;

      case NET_QBYTE:
        if(!client.connected() && client.available()==0){ netFail(); return; }

        if(client.available()){
          v = client.read();
          if(v<0){ netFail(); return; }
          qByte = (unsigned char)v;
          if(qByte>=1 && qByte<=200) myqq = qByte;
          if(qByte==255) {
            prefs.begin("conf", false);
            randomString(ser, 8);
            prefs.putBytes("ser", (const void *)ser, sizeof(ser));
            prefs.end();
          }
          payloadPos = 0;
          netState = NET_PAYLOAD;
          continue;
        }

        if(millis()-netT0 > 3000UL){ netFail(); }
        return;

      case NET_PAYLOAD:
        if(!client.connected() && client.available()==0){ netFail(); return; }

        localBudget = 256;
        while(client.available() && payloadPos < 6144 && localBudget--){
          v = client.read();
          if(v<0){ netFail(); return; }
          buf[payloadPos++] = (unsigned char)v;
        }

        if(payloadPos >= 6144){
          dec_k1 = 0;
          dec_k2 = 0;
          dec_aa = buf;
          netState = NET_DECODE;
          continue;
        }

        if(millis()-netT0 > 3500UL){ netFail(); }
        return;

      case NET_DECODE:
        decodeWords = 16; // 16 word per pump
        while(decodeWords-- > 0){

          if(dec_k1 >= 15){
            netState = NET_SWAP;
            break;
          }

          dec_oo = 0;
          for(n=0;n<16;n++){
            if((*dec_aa)&TTh[dec_k1+1]) dec_oo |= 1;
            dec_oo <<= 1;
            if((*dec_aa)&TTl[dec_k1+1]) dec_oo |= 1;
            if(n<15) dec_oo <<= 1;
            dec_aa++;
          }

          back[dec_k1][dec_k2] = dec_oo;

          dec_k2++;
          if(dec_k2 >= 384){
            dec_k2 = 0;
            dec_k1++;
            dec_aa = buf;
          }
        }
        return;

      case NET_SWAP:
        // SWAP ATOMICO (perché display legge front dall’altro core)
        portENTER_CRITICAL(&swapMux);
        swapTmp=front; front=back; back=swapTmp;
        valid=1;
        portEXIT_CRITICAL(&swapMux);

        netState = NET_IDLE;
        return;

      case NET_FAIL:
        netFail();
        return;
    }
  }
}

// Task rete su core 0
void netTask(void *pv){
  // Questo task gira sempre e pompa la rete. Il display resta nel loop() su core 1.
  for(;;){
    netPump(myPUMP);
    vTaskDelay(1); // 1 tick: evita di “mangiarsi” tutto il core; puoi provare 0/1/2
  }
}

void setup() {
  int r;
  uint32_t s;

  refresh=0;
  valid=0;
  myqq=20;

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
    if(r & 0x01) s |= MASK_A;
    if(r & 0x02) s |= MASK_B;
    if(r & 0x04) s |= MASK_C;
    if(r & 0x08) s |= MASK_D;
    if(r & 0x10) s |= MASK_E;
    rowSet[r]=s;
    rowClr[r]=MASK_ADDR & ~s;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(mySSID);

  // init pannello “nero stabile”
  for(row=0;row<32;row++){
    pOEl
    for(j=0;j<2;j++){
      for(i=0;i<32;i++){
        pR1l pR2l pG1l pG2l pB1l pB2l
        pCLKl
        pCLKh
      }
    }
    GPIO.out_w1tc=rowClr[row];
    GPIO.out_w1ts=rowSet[row];
    pLATh
    pLATl
    pOEh
  }
  
  prefs.begin("conf", false);
  has_ser = prefs.isKey("ser");
  if (!has_ser) {
    randomString(ser, 8);
    prefs.putBytes("ser", (const void *)ser, sizeof(ser));
  } 
  else prefs.getBytes("ser", (void *)ser, sizeof(ser));
  prefs.end();
  
  delay(4000);
  for(;;){
    if(WiFi.status()==WL_CONNECTED)break;
    delay(1000);
  }

  tt = millis();
  netState = NET_IDLE;

  // Avvia task rete su core 0 (PRO CPU)
  xTaskCreatePinnedToCore(
    netTask,            // function
    "netTask",          // name
    6144,               // stack (decode + buf: meglio non troppo basso)
    nullptr,            // param
    1,                  // priority (bassa; il display deve restare fluido)
    &netTaskHandle,     // handle
    0                   // core 0
  );
}

void loop() {
  uint32_t mask;

  // leggi puntatore front in modo atomico (evita race durante swap)
  volatile unsigned long (*frontLocal)[384] = getFrontPtr();

  pr1=frontLocal[refresh];
  pr2=pr1+64;
  pg1=pr2+64; pg2=pg1+64;
  pb1=pg2+64; pb2=pb1+64;

  for(row=0;row<32;row++){
    pOEl
    pR1l pR2l pG1l pG2l pB1l pB2l

    for(j=0;j<2;j++){
      zr1=*pr1++; zr2=*pr2++;
      zg1=*pg1++; zg2=*pg2++;
      zb1=*pb1++; zb2=*pb2++;

      if(valid){
        mask=0x80000000;
        for(i=0;i<32;i++){
          if(zr1 & mask) { pR1h } else { pR1l }
          if(zr2 & mask) { pR2h } else { pR2l }
          if(zg1 & mask) { pG1h } else { pG1l }
          if(zg2 & mask) { pG2h } else { pG2l }
          if(zb1 & mask) { pB1h } else { pB1l }
          if(zb2 & mask) { pB2h } else { pB2l }
          pCLKl
          pCLKh
          mask >>= 1;
        }
      } else {
        pR1l pR2l pG1l pG2l pB1l pB2l
        for(i=0;i<32;i++){
          pCLKl
          pCLKh
        }
      }
    }

    pR1l pR2l pG1l pG2l pB1l pB2l
    pCLKl

    GPIO.out_w1tc=rowClr[row];
    GPIO.out_w1ts=rowSet[row];

    if(row==0) asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
    else       asm volatile("nop; nop; nop; nop;");

    pLATh
    pLATl

    if(row==0) asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
    else       asm volatile("nop; nop; nop; nop;");

    pOEh
  }

  if(++refresh>=15){
    refresh=0;
    // IMPORTANT: qui NON chiamiamo più netPump(), perché la rete gira nel task su core 0
    // yield() qui va bene per cooperare con Arduino/RTOS sul core del display
    yield();
  }
}

