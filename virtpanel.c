// Gianluca Mazzini @2026- Version 2.02
// SDL2 virtual implementation of the 64x64 RGB888 display.

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <SDL.h>

#define SERVER_HOST "displayd.mazzini.org"
#define SERVER_PORT 5002
#define WIDTH 64
#define HEIGHT 64
#define PIXELS (WIDTH * HEIGHT)
#define FRAME_SIZE (PIXELS * 3)
#define DEFAULT_SCALE 10
#define VIRTUAL_RSSI -50

static int recv_full(int fd, void *buf, int len) {
  unsigned char *p;
  int done, r;

  p = (unsigned char *)buf;
  done = 0;
  for (; done < len; ) {
    r = (int)recv(fd, p + done, (size_t)(len - done), 0);
    if (r > 0) {
      done += r;
      continue;
    }
    if (r < 0 && errno == EINTR) continue;
    return -1;
  }
  return done;
}

static int send_full(int fd, const void *buf, int len) {
  const unsigned char *p;
  int done, r;

  p = (const unsigned char *)buf;
  done = 0;
  for (; done < len; ) {
    r = (int)send(fd, p + done, (size_t)(len - done), 0);
    if (r > 0) {
      done += r;
      continue;
    }
    if (r < 0 && errno == EINTR) continue;
    return -1;
  }
  return done;
}

static int connect_server(void) {
  struct addrinfo hints, *res, *ai;
  char port[16];
  int fd, one, r;

  fd = -1;
  res = 0;
  snprintf(port, sizeof(port), "%d", SERVER_PORT);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  r = getaddrinfo(SERVER_HOST, port, &hints, &res);
  if (r != 0) return -1;

  for (ai = res; ai != 0; ai = ai->ai_next) {
    fd = (int)socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (fd < 0) continue;

    one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));
    if (connect(fd, ai->ai_addr, (socklen_t)ai->ai_addrlen) == 0) break;

    close(fd);
    fd = -1;
  }

  freeaddrinfo(res);
  return fd;
}

static int hex_value(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static int parse_code(const char *text, char code[2]) {
  static const char hex[] = "0123456789ABCDEF";
  int hi, lo;

  if (text == 0) {
    code[0] = '0';
    code[1] = '1';
    return 0;
  }
  if (strlen(text) != 2) return -1;

  hi = hex_value((unsigned char)text[0]);
  lo = hex_value((unsigned char)text[1]);
  if (hi < 0 || lo < 0) return -1;

  code[0] = hex[hi];
  code[1] = hex[lo];
  return 0;
}

static int parse_scale(const char *text, int *scale) {
  char *end;
  long value;

  if (text == 0) {
    *scale = DEFAULT_SCALE;
    return 0;
  }

  errno = 0;
  end = 0;
  value = strtol(text, &end, 10);
  if (errno != 0 || end == text || *end != 0 || value < 1 || value > 64) return -1;
  *scale = (int)value;
  return 0;
}

static void make_serial(unsigned char header[16], const char code[2]) {
  memset(header, 0, 16);
  memcpy(header, "0202020202", 10);
  header[10] = (unsigned char)code[0];
  header[11] = (unsigned char)code[1];
}

static void add_local_ipv4(int fd, unsigned char header[16]) {
  struct sockaddr_storage ss;
  struct sockaddr_in *sin;
  socklen_t len;

  memset(&ss, 0, sizeof(ss));
  len = sizeof(ss);
  if (getsockname(fd, (struct sockaddr *)&ss, &len) != 0 || ss.ss_family != AF_INET) return;

  sin = (struct sockaddr_in *)&ss;
  memcpy(header + 12, &sin->sin_addr.s_addr, 4);
}

static void decode_frame(const unsigned char *frame, unsigned char *rgb) {
  int i;

  for (i = 0; i < PIXELS; i++) {
    rgb[i * 3] = frame[i];
    rgb[i * 3 + 1] = frame[PIXELS + i];
    rgb[i * 3 + 2] = frame[PIXELS * 2 + i];
  }
}

static void usage(const char *name) {
  fprintf(stderr, "Usage: %s [scale] [code]\n", name);
  fprintf(stderr, "  scale  window scale, default %d\n", DEFAULT_SCALE);
  fprintf(stderr, "  code   two hexadecimal digits, default 01\n");
}

int main(int argc, char **argv) {
  unsigned char header[16], frame[FRAME_SIZE], rgb[PIXELS * 3];
  signed char rssi;
  char code[2], title[64];
  int fd, scale, running;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;

  if (argc > 3) {
    usage(argv[0]);
    return 1;
  }
  if (parse_scale(argc >= 2 ? argv[1] : 0, &scale) != 0) {
    fprintf(stderr, "Invalid scale.\n");
    return 1;
  }
  if (parse_code(argc >= 3 ? argv[2] : 0, code) != 0) {
    fprintf(stderr, "Invalid code: use two hexadecimal digits.\n");
    return 1;
  }

  signal(SIGPIPE, SIG_IGN);
  fd = connect_server();
  if (fd < 0) {
    fprintf(stderr, "Cannot connect to %s:%d.\n", SERVER_HOST, SERVER_PORT);
    return 1;
  }

  make_serial(header, code);
  add_local_ipv4(fd, header);
  if (send_full(fd, header, sizeof(header)) != (int)sizeof(header)) {
    fprintf(stderr, "Handshake failed.\n");
    close(fd);
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    close(fd);
    return 1;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  snprintf(title, sizeof(title), "Virtual Display 0202020202%c%c", code[0], code[1]);
  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WIDTH * scale, HEIGHT * scale, SDL_WINDOW_SHOWN);
  if (window == 0) {
    fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
    SDL_Quit();
    close(fd);
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == 0) renderer = SDL_CreateRenderer(window, -1, 0);
  if (renderer == 0) {
    fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(fd);
    return 1;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                              WIDTH, HEIGHT);
  if (texture == 0) {
    fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    close(fd);
    return 1;
  }

  rssi = VIRTUAL_RSSI;
  running = 1;
  for (; running; ) {
    for (; SDL_PollEvent(&event); ) {
      if (event.type == SDL_QUIT) running = 0;
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = 0;
    }
    if (!running) break;

    if (recv_full(fd, frame, FRAME_SIZE) != FRAME_SIZE) {
      fprintf(stderr, "Server connection closed.\n");
      break;
    }
    if (send_full(fd, &rssi, 1) != 1) {
      fprintf(stderr, "RSSI reply failed.\n");
      break;
    }

    decode_frame(frame, rgb);
    SDL_UpdateTexture(texture, 0, rgb, WIDTH * 3);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  close(fd);
  return 0;
}
