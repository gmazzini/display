// Display on ESP32S3 Rel 20260125 by GM Copyright 2023-26

#include <Arduino.h>
#include <WiFi.h>
#include "soc/gpio_struct.h"
#include "soc/gpio_periph.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_mac.h"

#define mySSID "EmiliaRomagnaWiFi wifiprivacy.it"
#define myPUMP  3000

#define myWEB  "display.mazzini.org"
#define DISP_PORT 5000

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

int row,refresh,i,j,myqq,valid;
unsigned long zr1,zr2,zg1,zg2,zb1,zb2;
volatile unsigned long *pr1,*pr2,*pg1,*pg2,*pb1,*pb2;
unsigned long oo;

unsigned char buf[6144];

unsigned long MM_A[15][384];
unsigned long MM_B[15][384];
volatile unsigned long (*front)[384]=MM_A;
volatile unsigned long (*back)[384]=MM_B;
volatile unsigned long (*swapTmp)[384];

unsigned char TTl[]={0,1,2,2,4,4,4,4,8,8,8,8,8,8,8,8};
unsigned char TTh[]={0,16,32,32,64,64,64,64,128,128,128,128,128,128,128,128};
const char hex[] = "0123456789ABCDEF";

uint32_t rowSet[32],rowClr[32];
char macip[16];

WiFiClient client;
IPAddress dispIP, tmp;
unsigned long lastDns=0;
const unsigned long DNS_TTL=300000UL;

/* -------------------- DUAL CORE: lock + task -------------------- */
static portMUX_TYPE swapMux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t netTaskHandle = 0;

static inline volatile unsigned long (*getFrontPtr())[384] {
  volatile unsigned long (*p)[384];
  portENTER_CRITICAL(&swapMux);
  p = front;
  portEXIT_CRITICAL(&swapMux);
  return p;
}
/* ---------------------------------------------------------------- */

static inline int budget_expired(unsigned long t0, unsigned long budget_us){
  return ((unsigned long)(micros() - t0) >= budget_us);
}

static inline void drainClient(){
  while(client.available()) client.read();
}

/* ====================== NETWORK + STREAM STATE ====================== */
enum NetState { NET_IDLE, NET_DNS, NET_CONNECT, NET_SENDSER, NET_RECV, NET_DECODE, NET_SWAP };
static volatile enum NetState netState = NET_IDLE;

static volatile unsigned long lastNetOk = 0;
static volatile int forceNetRestart = 0;

static unsigned long netT0 = 0;
static unsigned long backoffMs = 500;
static unsigned long nextSerMs = 0;
static unsigned long lastFrameMs = 0;

static int payloadPos = 0;

/* decode cursor */
static int dec_k1=0;
static int dec_k2=0;
static unsigned char *dec_p = 0;

/* LUT: [bitplane 0..14][byte 0..255] -> 2 bits (hi,lo) packed as 0..3 */
static unsigned char lut[15][256];

/* build LUT once */
static void buildLUT(void){
  int plane;
  int b;
  unsigned char hiMask, loMask;
  unsigned char v;
  for(plane=0; plane<15; plane++){
    hiMask = TTh[plane+1];
    loMask = TTl[plane+1];
    for(b=0; b<256; b++){
      v = 0;
      if(((unsigned char)b) & hiMask) v |= 2;
      if(((unsigned char)b) & loMask) v |= 1;
      lut[plane][b] = v;
    }
  }
}

static void netFailHard(void){
  client.stop();
  netState = NET_IDLE;
  payloadPos = 0;
  dec_k1 = 0;
  dec_k2 = 0;
  dec_p = 0;
}

static void netPump(unsigned long budget_us){
  unsigned long pump_t0;
  int avail, v, localBudget;
  pump_t0 = micros();

  if(forceNetRestart){
    forceNetRestart = 0;
    netFailHard();
    netT0 = millis();
    return;
  }

  for(;;){
    if(budget_expired(pump_t0, budget_us)) return;

    switch(netState){

      case NET_IDLE:
        if((unsigned long)(millis() - netT0) < backoffMs) return;
        netState = NET_DNS;
        continue;

      case NET_DNS:
        if(WiFi.status()!=WL_CONNECTED){
          netFailHard();
          netT0 = millis();
          if(backoffMs < 8000UL) backoffMs <<= 1;
          return;
        }

        if(IP_IS_ZERO(dispIP) || (millis()-lastDns)>DNS_TTL){
          if(WiFi.hostByName(myWEB,tmp)){
            dispIP = tmp;
            lastDns = millis();
          } else {
            netFailHard();
            netT0 = millis();
            if(backoffMs < 8000UL) backoffMs <<= 1;
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
          if(!client.connect(dispIP, DISP_PORT)){
            netFailHard();
            netT0 = millis();
            if(backoffMs < 8000UL) backoffMs <<= 1;
            return;
          }
        }

        drainClient();
        backoffMs = 500UL;

        netState = NET_SENDSER;
        continue;

      case NET_SENDSER:
        if(client.write((const unsigned char*)macip, 16) != 16){
          netFailHard();
          netT0 = millis();
          return;
        }
        nextSerMs = millis() + 30000UL;
        payloadPos = 0;
        netT0 = millis();
        netState = NET_RECV;
        continue;

      case NET_RECV:
        if(!client.connected() && client.available()==0){
          netFailHard();
          netT0 = millis();
          return;
        }

        /* keepalive applicativo SER */
        if((long)(millis() - nextSerMs) >= 0){
          if(client.write((const unsigned char*)macip, 16) != 16){
            netFailHard();
            netT0 = millis();
            return;
          }
          nextSerMs = millis() + 30000UL;
        }

        int need, r;
need = 6144 - payloadPos;
if(need > 0){
  r = client.read((unsigned char*)buf + payloadPos, need);
  if(r > 0) payloadPos += r;
}

        if(payloadPos >= 6144){
          /* frame completo -> decode in back */
          dec_k1 = 0;
          dec_k2 = 0;
          dec_p  = buf; /* inizio buffer per il plane 0 */
          netState = NET_DECODE;
          continue;
        }

        /* timeout frame */
        if((unsigned long)(millis() - netT0) > 20000UL){
          netFailHard();
          netT0 = millis();
          return;
        }

        return;

      case NET_DECODE:
        /* decode “chunked” per rispettare budget */
        for(;;){
          unsigned long oo;
          unsigned char *p;
          unsigned char *L;
          int t;

          if(budget_expired(pump_t0, budget_us)) return;

          if(dec_k1 >= 15){
            netState = NET_SWAP;
            break;
          }

          /* pointer al LUT del plane corrente */
          L = (unsigned char*)lut[dec_k1];

          /* pointer ai 16 byte della word corrente (k2) */
          p = dec_p;

          /* costruisci 32 bit: 16 volte (<<2 | lut[byte]) */
          oo = 0;
          for(t=0; t<16; t++){
            oo = (oo << 2) | (unsigned long)L[*p++];
          }

          back[dec_k1][dec_k2] = oo;

          /* avanti di 16 byte nel buf */
          dec_p += 16;

          dec_k2++;
          if(dec_k2 >= 384){
            dec_k2 = 0;
            dec_k1++;
            dec_p = buf; /* reset: ogni plane scorre lo stesso buf */
          }
        }
        continue;

      case NET_SWAP:
        portENTER_CRITICAL(&swapMux);
        swapTmp = front; front = back; back = swapTmp;
        valid = 1;
        portEXIT_CRITICAL(&swapMux);

        lastNetOk = millis();
        lastFrameMs = lastNetOk;

        /* pronti per frame successivo (connessione resta aperta) */
        payloadPos = 0;
        netT0 = millis();
        netState = NET_RECV;
        return;
    }
  }
}

/* Task rete su core 0 */
void netTask(void *pv){
  static unsigned long lastWd=0;

  for(;;){
    netPump(myPUMP);

    if(millis()-lastWd > 500UL){
      lastWd = millis();
      /* se siamo vivi ma non arrivano frame per troppo, reset */
      if(valid && lastFrameMs && (millis()-lastFrameMs > 30000UL)){
        forceNetRestart = 1;
        lastFrameMs = millis();
      }
    }

    vTaskDelay(1);
  }
}

/* =============================== SETUP =============================== */
void setup() {
  int r;
  uint32_t s;
  uint8_t factory[6];
  esp_netif_ip_info_t ipinfo;


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

  /* init pannello nero stabile */
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

  esp_efuse_mac_get_default(factory);
  for (r = 0; r < 6; r++) {
    macip[r*2]     = hex[(factory[r] >> 4) & 0x0F];
    macip[r*2 + 1] = hex[ factory[r]       & 0x0F];
  }
  esp_netif_get_ip_info(netif, &ipinfo);
  memcpy(&macip[12], &ipinfo.ip.addr, 4);

  /* LUT decode */
  buildLUT();

  delay(4000);
  for(;;){
    if(WiFi.status()==WL_CONNECTED)break;
    delay(1000);
  }

  netT0 = millis();
  netState = NET_IDLE;

  /* Avvia task rete su core 0 */
  xTaskCreatePinnedToCore(
    netTask,
    "netTask",
    8192,          /* un po' più stack: decode + WiFi */
    0,
    1,
    &netTaskHandle,
    0
  );
}

/* =============================== LOOP DISPLAY (core1) =============================== */
void loop() {
  uint32_t mask;

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
    yield();
  }
}
