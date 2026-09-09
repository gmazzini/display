// Gianluca Mazzini @2023- Version 4.50
// ESP32-S3 firmware for the 64x64 HUB75 network display.

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_private/gdma.h"
#include "esp_private/periph_ctrl.h"
#include "esp_rom_gpio.h"
#include "hal/dma_types.h"
#include "soc/gdma_struct.h"
#include "soc/gpio_sig_map.h"
#include "soc/lcd_cam_struct.h"
#include "soc/lldesc.h"
#include "soc/periph_defs.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "nvs_flash.h"
#include "soc/gpio_struct.h"

#define WIFI_SSID "EmiliaRomagnaWiFi wifiprivacy.it"
#define SERVER_HOST "displayd.mazzini.org"
#define SERVER_PORT 5002

#define COLOR_BITS 8
#if COLOR_BITS < 1 || COLOR_BITS > 8
#error COLOR_BITS must be between 1 and 8
#endif
#define COLOR_SHIFT (8 - COLOR_BITS)
#define PANEL_WIDTH 64
#define ROWS 32
#define CHANNEL_BYTES (PANEL_WIDTH * ROWS * 2)
#define FRAME_SIZE (CHANNEL_BYTES * 3)
#define DMA_BASE_EXPOSURE_US (1U << COLOR_SHIFT)

#define NET_PUMP_US 3000U
#define DNS_TTL 300000U
#define NET_FAILS_BEFORE_WIFI_RECOVER 5
#define WIFI_RECOVER_MIN_GAP 15000U
#define FRAME_SOFT_TIMEOUT 30000U
#define FRAME_HARD_TIMEOUT 180000U
#define WIFI_BOOT_TIMEOUT 60000U
#define FRAME_RECV_TIMEOUT 20000U
#define NET_BACKOFF_MIN 500U
#define NET_BACKOFF_MAX 8000U
#define TCP_CONNECT_TIMEOUT 3000U
#define NET_STACK_SIZE 8192U
#define DISPLAY_STACK_SIZE 4096U

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

#define pCLKh GPIO.out1_w1ts.val = ((uint32_t)1 << (38 - 32));
#define pCLKl GPIO.out1_w1tc.val = ((uint32_t)1 << (38 - 32));
#define pLATh GPIO.out1_w1ts.val = ((uint32_t)1 << (47 - 32));
#define pLATl GPIO.out1_w1tc.val = ((uint32_t)1 << (47 - 32));
#define pOEh  GPIO.out1_w1ts.val = ((uint32_t)1 << (48 - 32));
#define pOEl  GPIO.out1_w1tc.val = ((uint32_t)1 << (48 - 32));

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

#define MASK_A    (1U << 21)
#define MASK_B    (1U << 18)
#define MASK_C    (1U << 17)
#define MASK_D    (1U << 10)
#define MASK_E    (1U << 9)
#define MASK_ADDR (MASK_A | MASK_B | MASK_C | MASK_D | MASK_E)

static int row, i, j;
static volatile int valid;
static volatile int wifi_connected, wifi_auto_reconnect;

static unsigned char buf[FRAME_SIZE];
DMA_ATTR static unsigned char dma_a[COLOR_BITS][ROWS][PANEL_WIDTH];
DMA_ATTR static unsigned char dma_b[COLOR_BITS][ROWS][PANEL_WIDTH];
static unsigned char (* volatile dma_front)[ROWS][PANEL_WIDTH];
static unsigned char (* volatile dma_back)[ROWS][PANEL_WIDTH];

static const char hex[] = "0123456789ABCDEF";

static const unsigned char gamma_lut[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6,
  6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12,
  12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
  20, 20, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28, 29,
  30, 30, 31, 32, 33, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41,
  42, 43, 43, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 53, 54, 55,
  56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
  73, 74, 75, 76, 77, 78, 79, 81, 82, 83, 84, 85, 87, 88, 89, 90,
  91, 93, 94, 95, 97, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 111,
  113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135,
  137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 153, 154, 156, 158, 159, 161,
  163, 165, 166, 168, 170, 172, 173, 175, 177, 179, 181, 182, 184, 186, 188, 190,
  192, 194, 196, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221,
  223, 225, 227, 229, 231, 234, 236, 238, 240, 242, 244, 246, 248, 251, 253, 255
};

static uint32_t row_set[ROWS], row_clr[ROWS];
static unsigned char macip[16];
static volatile unsigned char wifi_ip[4];
static struct in_addr server_addr;
static int sock;

static portMUX_TYPE swap_mux = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE display_mux = portMUX_INITIALIZER_UNLOCKED;
static StaticTask_t net_task_buffer, display_task_buffer;
static StackType_t net_stack[NET_STACK_SIZE / sizeof(StackType_t)];
static StackType_t display_stack[DISPLAY_STACK_SIZE / sizeof(StackType_t)];

enum net_state {
  NET_IDLE,
  NET_DNS,
  NET_CONNECT,
  NET_SENDSER,
  NET_RECV,
  NET_DECODE,
  NET_SWAP
};

static enum net_state net_state;
static uint32_t last_net_ok, net_t0, backoff_ms, last_frame_ms, last_wifi_recover, last_dns;
static int net_fail_count, force_net_restart, payload_pos, dec_pixel;

static uint32_t now_us(void) {
  return (uint32_t)esp_timer_get_time();
}

static uint32_t now_ms(void) {
  return (uint32_t)(esp_timer_get_time() / 1000);
}

static unsigned char (*get_dma_front(void))[ROWS][PANEL_WIDTH] {
  unsigned char (*p)[ROWS][PANEL_WIDTH];

  portENTER_CRITICAL(&swap_mux);
  p = dma_front;
  portEXIT_CRITICAL(&swap_mux);
  return p;
}

static int budget_expired(uint32_t start, uint32_t budget) {
  return (uint32_t)(now_us() - start) >= budget;
}

static void dma_decode_pair(int pixel) {
  unsigned int r1, g1, b1, r2, g2, b2, top, bottom;
  unsigned char v;
  int plane, row_local, x;

  row_local = pixel / PANEL_WIDTH;
  x = pixel % PANEL_WIDTH;
  top = (unsigned int)pixel;
  bottom = top + ROWS * PANEL_WIDTH;

  r1 = gamma_lut[buf[top]] >> COLOR_SHIFT;
  r2 = gamma_lut[buf[bottom]] >> COLOR_SHIFT;
  g1 = gamma_lut[buf[CHANNEL_BYTES + top]] >> COLOR_SHIFT;
  g2 = gamma_lut[buf[CHANNEL_BYTES + bottom]] >> COLOR_SHIFT;
  b1 = gamma_lut[buf[CHANNEL_BYTES * 2 + top]] >> COLOR_SHIFT;
  b2 = gamma_lut[buf[CHANNEL_BYTES * 2 + bottom]] >> COLOR_SHIFT;

  for (plane = 0; plane < COLOR_BITS; plane++) {
    v = (unsigned char)(((r1 >> plane) & 1U) |
                        (((g1 >> plane) & 1U) << 1) |
                        (((b1 >> plane) & 1U) << 2) |
                        (((r2 >> plane) & 1U) << 3) |
                        (((g2 >> plane) & 1U) << 4) |
                        (((b2 >> plane) & 1U) << 5));
    dma_back[plane][row_local][x] = v;
  }
}

static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
  ip_event_got_ip_t *event;

  (void)arg;

  if (base == WIFI_EVENT) {
    if (id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
      return;
    }

    if (id == WIFI_EVENT_STA_DISCONNECTED) {
      wifi_connected = 0;
      wifi_ip[0] = 0;
      wifi_ip[1] = 0;
      wifi_ip[2] = 0;
      wifi_ip[3] = 0;
      if (wifi_auto_reconnect) esp_wifi_connect();
      return;
    }
  }

  if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
    event = (ip_event_got_ip_t *)data;
    wifi_ip[0] = esp_ip4_addr1(&event->ip_info.ip);
    wifi_ip[1] = esp_ip4_addr2(&event->ip_info.ip);
    wifi_ip[2] = esp_ip4_addr3(&event->ip_info.ip);
    wifi_ip[3] = esp_ip4_addr4(&event->ip_info.ip);
    wifi_connected = 1;
  }
}

static int wifi_start(void) {
  esp_err_t err;
  esp_netif_t *netif;
  wifi_init_config_t init;
  wifi_config_t sta;

  init = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    if (nvs_flash_erase() != ESP_OK) return -1;
    err = nvs_flash_init();
  }
  if (err != ESP_OK) return -1;

  if (esp_netif_init() != ESP_OK) return -1;
  if (esp_event_loop_create_default() != ESP_OK) return -1;

  netif = esp_netif_create_default_wifi_sta();
  if (netif == 0) return -1;

  if (esp_wifi_init(&init) != ESP_OK) return -1;
  if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) return -1;

  if (esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event, 0) != ESP_OK) return -1;
  if (esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event, 0) != ESP_OK) return -1;

  memset(&sta, 0, sizeof(sta));
  memcpy(sta.sta.ssid, WIFI_SSID, sizeof(WIFI_SSID) - 1);

  if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) return -1;
  if (esp_wifi_set_config(WIFI_IF_STA, &sta) != ESP_OK) return -1;
  if (esp_wifi_set_ps(WIFI_PS_NONE) != ESP_OK) return -1;

  wifi_auto_reconnect = 1;
  if (esp_wifi_start() != ESP_OK) return -1;
  return 0;
}

static int wifi_rssi(void) {
  wifi_ap_record_t ap;

  if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) return -127;
  return ap.rssi;
}

static void net_tcp_reset(void);

static void wifi_recover(void) {
  wifi_config_t sta;
  uint32_t now;

  now = now_ms();
  if ((uint32_t)(now - last_wifi_recover) < WIFI_RECOVER_MIN_GAP) return;

  last_wifi_recover = now;
  net_tcp_reset();
  server_addr.s_addr = 0;
  last_dns = 0;
  wifi_auto_reconnect = 0;
  wifi_connected = 0;
  esp_wifi_disconnect();
  vTaskDelay(pdMS_TO_TICKS(200));

  memset(&sta, 0, sizeof(sta));
  memcpy(sta.sta.ssid, WIFI_SSID, sizeof(WIFI_SSID) - 1);
  if (esp_wifi_set_config(WIFI_IF_STA, &sta) != ESP_OK) {
    esp_restart();
  }

  wifi_auto_reconnect = 1;
  if (esp_wifi_connect() != ESP_OK) {
    esp_restart();
  }
  net_fail_count = 0;
  backoff_ms = 1000U;
  net_t0 = now_ms();
}

static int resolve_server(void) {
  struct addrinfo hints, *res;
  struct sockaddr_in *addr;
  int r;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  res = 0;

  r = getaddrinfo(SERVER_HOST, 0, &hints, &res);
  if (r != 0 || res == 0) return -1;

  addr = (struct sockaddr_in *)res->ai_addr;
  server_addr = addr->sin_addr;
  freeaddrinfo(res);
  return 0;
}

static void socket_close(void) {
  unsigned char drop[64];
  int r;

  if (sock < 0) return;

  for (;;) {
    r = (int)recv(sock, drop, sizeof(drop), MSG_DONTWAIT);
    if (r <= 0) break;
  }

  shutdown(sock, SHUT_RDWR);
  close(sock);
  sock = -1;
}

static int socket_connect(void) {
  struct sockaddr_in addr;
  struct timeval tv;
  fd_set set;
  socklen_t len;
  int flags, one, r, error;

  socket_close();
  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) return -1;

  flags = fcntl(sock, F_GETFL, 0);
  if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) != 0) {
    socket_close();
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SERVER_PORT);
  addr.sin_addr = server_addr;

  r = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
  if (r != 0 && errno != EINPROGRESS) {
    socket_close();
    return -1;
  }

  if (r != 0) {
    FD_ZERO(&set);
    FD_SET(sock, &set);
    tv.tv_sec = TCP_CONNECT_TIMEOUT / 1000U;
    tv.tv_usec = (TCP_CONNECT_TIMEOUT % 1000U) * 1000U;

    r = select(sock + 1, 0, &set, 0, &tv);
    if (r <= 0) {
      socket_close();
      return -1;
    }

    error = 0;
    len = sizeof(error);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) != 0 || error != 0) {
      socket_close();
      return -1;
    }
  }

  if (fcntl(sock, F_SETFL, flags) != 0) {
    socket_close();
    return -1;
  }

  tv.tv_sec = TCP_CONNECT_TIMEOUT / 1000U;
  tv.tv_usec = (TCP_CONNECT_TIMEOUT % 1000U) * 1000U;
  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0 ||
      setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
    socket_close();
    return -1;
  }

  one = 1;
  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) != 0) {
    socket_close();
    return -1;
  }
  return 0;
}

static int socket_write(const unsigned char *p, int len) {
  int done, r;

  done = 0;
  for (; done < len; ) {
    r = (int)send(sock, p + done, (size_t)(len - done), 0);
    if (r > 0) {
      done += r;
      continue;
    }
    if (r < 0 && errno == EINTR) continue;
    return -1;
  }
  return done;
}

static void net_tcp_reset(void) {
  socket_close();
  net_state = NET_IDLE;
  payload_pos = 0;
  dec_pixel = 0;
}

static void net_fail(void) {
  net_tcp_reset();
  net_fail_count++;

  if (backoff_ms < NET_BACKOFF_MAX) backoff_ms <<= 1;
  if (net_fail_count >= NET_FAILS_BEFORE_WIFI_RECOVER) {
    server_addr.s_addr = 0;
    last_dns = 0;
    wifi_recover();
  }

  net_t0 = now_ms();
}

static void macip_update_ip(void) {
  macip[12] = wifi_ip[0];
  macip[13] = wifi_ip[1];
  macip[14] = wifi_ip[2];
  macip[15] = wifi_ip[3];
}

static void net_pump(uint32_t budget) {
  uint32_t start;
  unsigned char (*swap)[ROWS][PANEL_WIDTH];
  signed char rssi;
  int need, r;

  start = now_us();

  if (force_net_restart) {
    force_net_restart = 0;
    net_fail();
    return;
  }

  for (;;) {
    if (budget_expired(start, budget)) return;

    switch (net_state) {
    case NET_IDLE:
      if ((uint32_t)(now_ms() - net_t0) < backoff_ms) return;
      net_state = NET_DNS;
      continue;

    case NET_DNS:
      if (!wifi_connected) {
        net_fail();
        return;
      }

      if (server_addr.s_addr == 0 || (uint32_t)(now_ms() - last_dns) > DNS_TTL) {
        if (resolve_server() != 0) {
          net_fail();
          return;
        }
        last_dns = now_ms();
      }

      net_state = NET_CONNECT;
      continue;

    case NET_CONNECT:
      if (socket_connect() != 0) {
        net_fail();
        return;
      }

      backoff_ms = NET_BACKOFF_MIN;
      net_state = NET_SENDSER;
      continue;

    case NET_SENDSER:
      macip_update_ip();
      if (socket_write((const unsigned char *)macip, 16) != 16) {
        net_fail();
        return;
      }

      payload_pos = 0;
      net_t0 = now_ms();
      net_state = NET_RECV;
      continue;

    case NET_RECV:
      need = FRAME_SIZE - payload_pos;
      r = (int)recv(sock, buf + payload_pos, (size_t)need, MSG_DONTWAIT);

      if (r > 0) payload_pos += r;
      else if (r == 0) {
        net_fail();
        return;
      } else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
        net_fail();
        return;
      }

      if (payload_pos >= FRAME_SIZE) {
        dec_pixel = 0;
        net_state = NET_DECODE;
        continue;
      }

      if ((uint32_t)(now_ms() - net_t0) > FRAME_RECV_TIMEOUT) {
        net_fail();
        return;
      }
      return;

    case NET_DECODE:
      for (;;) {
        if (budget_expired(start, budget)) return;

        if (dec_pixel >= ROWS * PANEL_WIDTH) {
          net_state = NET_SWAP;
          break;
        }

        dma_decode_pair(dec_pixel++);
      }
      continue;

    case NET_SWAP:
      portENTER_CRITICAL(&swap_mux);
      swap = dma_front;
      dma_front = dma_back;
      dma_back = swap;
      valid = 1;
      portEXIT_CRITICAL(&swap_mux);

      last_net_ok = now_ms();
      last_frame_ms = last_net_ok;
      net_fail_count = 0;
      backoff_ms = NET_BACKOFF_MIN;

      rssi = (signed char)wifi_rssi();
      if (socket_write((const unsigned char *)&rssi, 1) != 1) {
        net_fail();
        return;
      }

      payload_pos = 0;
      net_t0 = now_ms();
      net_state = NET_RECV;
      return;
    }
  }
}

static void net_task(void *arg) {
  uint32_t last_wd, now;

  (void)arg;
  last_wd = 0;

  for (;;) {
    net_pump(NET_PUMP_US);
    now = now_ms();

    if ((uint32_t)(now - last_wd) > 500U) {
      last_wd = now;

      if (valid && last_frame_ms && (uint32_t)(now - last_frame_ms) > FRAME_SOFT_TIMEOUT) {
        force_net_restart = 1;
        last_frame_ms = now;
      }

      if (last_net_ok && (uint32_t)(now - last_net_ok) > FRAME_HARD_TIMEOUT) esp_restart();

      if (!wifi_connected) {
        net_fail_count++;
        if (net_fail_count >= NET_FAILS_BEFORE_WIFI_RECOVER) {
          server_addr.s_addr = 0;
          last_dns = 0;
          wifi_recover();
        }
      }
    }

    vTaskDelay(1);
  }
}


DMA_ATTR static lldesc_t raw_dma_desc_a[COLOR_BITS][ROWS];
DMA_ATTR static lldesc_t raw_dma_desc_b[COLOR_BITS][ROWS];
static gdma_channel_handle_t raw_dma_handle;
static int raw_dma_channel;

static void raw_dma_reset_pins(void) {
  gpio_reset_pin(pCLK);
  gpio_reset_pin(pR1);
  gpio_reset_pin(pG1);
  gpio_reset_pin(pB1);
  gpio_reset_pin(pR2);
  gpio_reset_pin(pG2);
  gpio_reset_pin(pB2);
}

static int raw_dma_wait(void) {
  uint32_t start;

  start = now_us();
  for (;;) {
    if (GDMA.channel[raw_dma_channel].out.int_raw.out_total_eof) break;
    if ((uint32_t)(now_us() - start) > 100000U) return -1;
  }

  start = now_us();
  for (;;) {
    if (!LCD_CAM.lcd_user.lcd_start) return 0;
    if ((uint32_t)(now_us() - start) > 100000U) return -1;
  }
}

static void raw_dma_start(lldesc_t *desc) {
  desc->owner = 1;
  GDMA.channel[raw_dma_channel].out.int_clr.val = 0xFFFFFFFFU;
  GDMA.channel[raw_dma_channel].out.link.addr = ((uint32_t)desc) & 0xFFFFFU;
  GDMA.channel[raw_dma_channel].out.link.start = 1;

  LCD_CAM.lcd_user.lcd_update = 1;
  LCD_CAM.lcd_user.lcd_start = 1;
}

static int raw_dma_init(void) {
  gdma_channel_alloc_config_t alloc;
  int plane, r;

  raw_dma_handle = 0;
  raw_dma_channel = -1;
  memset(&alloc, 0, sizeof(alloc));
  alloc.direction = GDMA_CHANNEL_DIRECTION_TX;

  if (gdma_new_ahb_channel(&alloc, &raw_dma_handle) != ESP_OK) return -1;
  if (gdma_get_channel_id(raw_dma_handle, &raw_dma_channel) != ESP_OK || raw_dma_channel < 0) return -1;

  memset(raw_dma_desc_a, 0, sizeof(raw_dma_desc_a));
  memset(raw_dma_desc_b, 0, sizeof(raw_dma_desc_b));
  for (plane = 0; plane < COLOR_BITS; plane++) {
    for (r = 0; r < ROWS; r++) {
      raw_dma_desc_a[plane][r].size = PANEL_WIDTH;
      raw_dma_desc_a[plane][r].length = PANEL_WIDTH;
      raw_dma_desc_a[plane][r].eof = 1;
      raw_dma_desc_a[plane][r].owner = 1;
      raw_dma_desc_a[plane][r].buf = dma_a[plane][r];
      raw_dma_desc_a[plane][r].qe.stqe_next = 0;
      raw_dma_desc_b[plane][r].size = PANEL_WIDTH;
      raw_dma_desc_b[plane][r].length = PANEL_WIDTH;
      raw_dma_desc_b[plane][r].eof = 1;
      raw_dma_desc_b[plane][r].owner = 1;
      raw_dma_desc_b[plane][r].buf = dma_b[plane][r];
      raw_dma_desc_b[plane][r].qe.stqe_next = 0;
    }
  }

  gpio_set_level(pOE, 1);
  gpio_set_level(pLAT, 0);

  raw_dma_reset_pins();
  gpio_set_direction(pCLK, GPIO_MODE_OUTPUT);
  gpio_set_direction(pR1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pG1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pB1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pR2, GPIO_MODE_OUTPUT);
  gpio_set_direction(pG2, GPIO_MODE_OUTPUT);
  gpio_set_direction(pB2, GPIO_MODE_OUTPUT);

  esp_rom_gpio_connect_out_signal(pR1, LCD_DATA_OUT0_IDX, false, false);
  esp_rom_gpio_connect_out_signal(pG1, LCD_DATA_OUT1_IDX, false, false);
  esp_rom_gpio_connect_out_signal(pB1, LCD_DATA_OUT2_IDX, false, false);
  esp_rom_gpio_connect_out_signal(pR2, LCD_DATA_OUT3_IDX, false, false);
  esp_rom_gpio_connect_out_signal(pG2, LCD_DATA_OUT4_IDX, false, false);
  esp_rom_gpio_connect_out_signal(pB2, LCD_DATA_OUT5_IDX, false, false);
  esp_rom_gpio_connect_out_signal(pCLK, LCD_PCLK_IDX, false, false);

  periph_module_enable(PERIPH_LCD_CAM_MODULE);
  periph_module_reset(PERIPH_LCD_CAM_MODULE);

  LCD_CAM.lcd_clock.val = 0;
  LCD_CAM.lcd_clock.clk_en = 1;
  LCD_CAM.lcd_clock.lcd_clk_sel = 2;
  LCD_CAM.lcd_clock.lcd_clkm_div_num = 1;
  LCD_CAM.lcd_clock.lcd_clkcnt_n = 19;
  LCD_CAM.lcd_clock.lcd_ck_idle_edge = 1;
  LCD_CAM.lcd_clock.lcd_ck_out_edge = 0;

  LCD_CAM.lcd_misc.val = 0;
  LCD_CAM.lcd_ctrl.lcd_rgb_mode_en = 0;
  LCD_CAM.lcd_misc.lcd_vfk_cyclelen = 0;
  LCD_CAM.lcd_misc.lcd_vbk_cyclelen = 0;
  LCD_CAM.lcd_misc.lcd_next_frame_en = 0;
  LCD_CAM.lcd_misc.lcd_bk_en = 0;

  LCD_CAM.lcd_user.val = 0;
  LCD_CAM.lcd_user.lcd_dout_cyclelen = PANEL_WIDTH - 1;
  LCD_CAM.lcd_user.lcd_always_out_en = 0;
  LCD_CAM.lcd_user.lcd_2byte_en = 0;
  LCD_CAM.lcd_user.lcd_dout = 1;
  LCD_CAM.lcd_user.lcd_dummy = 1;
  LCD_CAM.lcd_user.lcd_dummy_cyclelen = 1;

  GDMA.channel[raw_dma_channel].out.conf0.val = 0;
  GDMA.channel[raw_dma_channel].out.conf1.val = 0;
  GDMA.channel[raw_dma_channel].out.peri_sel.sel = 5;
  return 0;
}

static void panel_prepare(void) {
  uint32_t s;
  int r;

  gpio_set_direction(pOE, GPIO_MODE_OUTPUT);
  gpio_set_direction(pLAT, GPIO_MODE_OUTPUT);
  gpio_set_direction(pCLK, GPIO_MODE_OUTPUT);
  gpio_set_direction(pA, GPIO_MODE_OUTPUT);
  gpio_set_direction(pB, GPIO_MODE_OUTPUT);
  gpio_set_direction(pC, GPIO_MODE_OUTPUT);
  gpio_set_direction(pD, GPIO_MODE_OUTPUT);
  gpio_set_direction(pE, GPIO_MODE_OUTPUT);
  gpio_set_direction(pR1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pR2, GPIO_MODE_OUTPUT);
  gpio_set_direction(pG1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pG2, GPIO_MODE_OUTPUT);
  gpio_set_direction(pB1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pB2, GPIO_MODE_OUTPUT);

  for (r = 0; r < ROWS; r++) {
    s = 0;
    if (r & 0x01) s |= MASK_A;
    if (r & 0x02) s |= MASK_B;
    if (r & 0x04) s |= MASK_C;
    if (r & 0x08) s |= MASK_D;
    if (r & 0x10) s |= MASK_E;
    row_set[r] = s;
    row_clr[r] = MASK_ADDR & ~s;
  }
}

static void panel_clear(void) {
  for (row = 0; row < ROWS; row++) {
    pOEl

    for (j = 0; j < 2; j++) {
      for (i = 0; i < 32; i++) {
        pR1l pR2l pG1l pG2l pB1l pB2l
        pCLKl
        pCLKh
      }
    }

    GPIO.out_w1tc = row_clr[row];
    GPIO.out_w1ts = row_set[row];
    pLATh
    pLATl
    pOEh
  }
}

static void display_task(void *arg) {
  unsigned char (*front_local)[ROWS][PANEL_WIDTH];
  lldesc_t (*desc)[ROWS];
  uint32_t exposure;
  int plane, r;

  (void)arg;

  for (;;) {
    front_local = get_dma_front();
    desc = front_local == dma_a ? raw_dma_desc_a : raw_dma_desc_b;

    for (plane = 0; plane < COLOR_BITS; plane++) {
      exposure = DMA_BASE_EXPOSURE_US << plane;

      for (r = 0; r < ROWS; r++) {
        pOEh
        raw_dma_start(&desc[plane][r]);
        if (raw_dma_wait() != 0) esp_restart();

        GPIO.out_w1tc = row_clr[r];
        GPIO.out_w1ts = row_set[r];
        asm volatile("nop; nop; nop; nop;");
        pLATh
        asm volatile("nop; nop; nop; nop;");
        pLATl

        if (valid) {
          portENTER_CRITICAL(&display_mux);
          pOEl
          esp_rom_delay_us(exposure);
          pOEh
          portEXIT_CRITICAL(&display_mux);
        }
      }
    }

    taskYIELD();
  }
}

void app_main(void) {
  uint8_t factory[6];
  TaskHandle_t task;
  int k;

  dma_front = dma_a;
  dma_back = dma_b;
  sock = -1;
  backoff_ms = NET_BACKOFF_MIN;

  panel_prepare();
  if (wifi_start() != 0) esp_restart();
  panel_clear();
  if (raw_dma_init() != 0) esp_restart();

  if (esp_efuse_mac_get_default(factory) != ESP_OK) {
    esp_restart();
  }

  for (k = 0; k < 6; k++) {
    macip[k * 2] = hex[(factory[k] >> 4) & 0x0F];
    macip[k * 2 + 1] = hex[factory[k] & 0x0F];
  }

  vTaskDelay(pdMS_TO_TICKS(4000));

  last_net_ok = now_ms();
  last_frame_ms = last_net_ok;

  for (;;) {
    if (wifi_connected) break;
    if ((uint32_t)(now_ms() - last_net_ok) > WIFI_BOOT_TIMEOUT) esp_restart();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  last_net_ok = now_ms();
  last_frame_ms = last_net_ok;
  net_t0 = now_ms();
  net_state = NET_IDLE;

  task = xTaskCreateStaticPinnedToCore(net_task, "net_task", NET_STACK_SIZE, 0, 1, net_stack, &net_task_buffer, 0);
  if (task == 0) esp_restart();

  task = xTaskCreateStaticPinnedToCore(display_task, "display", DISPLAY_STACK_SIZE, 0, 1, display_stack, &display_task_buffer, 1);
  if (task == 0) esp_restart();

  vTaskDelete(0);
}
