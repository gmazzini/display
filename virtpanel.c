// virtual implementation
// gcc -O3 display/virtpanel.c -o virtpanel $(sdl2-config --cflags --libs)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <SDL.h>

#define MAC "AABBCCDDEEFF"
#define HOST_DEFAULT "display.mazzini.org"
#define PORT_DEFAULT 5000

#define LEN  6144
#define W    64
#define H    64

static int recvn(int fd, void *buf, int n) {
    int got, r;
    char *p;

    got = 0;
    p = (char *)buf;
    while (got < n) {
        r = (int)recv(fd, p + got, (size_t)(n - got), 0);
        if (r <= 0) return -1;
        got += r;
    }
    return got;
}

static int sendn(int fd, const void *buf, int n) {
    int sent, r;
    const char *p;

    sent = 0;
    p = (const char *)buf;
    while (sent < n) {
        r = (int)send(fd, p + sent, (size_t)(n - sent), 0);
        if (r <= 0) return -1;
        sent += r;
    }
    return sent;
}

static int connect_tcp_dns(const char *host, int port) {
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *ai;
    char portstr[16];
    int fd;
    int one;
    int rc;

    fd = -1;
    res = NULL;

    snprintf(portstr, sizeof(portstr), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rc = getaddrinfo(host, portstr, &hints, &res);
    if (rc != 0) return -1;

    for (ai = res; ai != NULL; ai = ai->ai_next) {
        fd = (int)socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) continue;

        one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, (socklen_t)sizeof(one));

        if (connect(fd, ai->ai_addr, (socklen_t)ai->ai_addrlen) == 0) {
            freeaddrinfo(res);
            return fd;
        }

        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);
    return -1;
}

/* prende l'IPv4 locale usata dalla connessione TCP (come "WiFi.localIP()" lato ESP) */
static void get_local_ip4_from_socket(int fd, uint8_t out_ip[4]) {
    struct sockaddr_storage ss;
    socklen_t slen;
    struct sockaddr_in *sin;

    out_ip[0] = 0; out_ip[1] = 0; out_ip[2] = 0; out_ip[3] = 0;

    memset(&ss, 0, sizeof(ss));
    slen = (socklen_t)sizeof(ss);

    if (getsockname(fd, (struct sockaddr *)&ss, &slen) != 0) return;
    if (ss.ss_family != AF_INET) return;

    sin = (struct sockaddr_in *)&ss;
    memcpy(out_ip, &sin->sin_addr.s_addr, 4);
    /* sin_addr.s_addr è in network order; per bytes va benissimo così */
}

/* Il frame 6144 è: R[2048] G[2048] B[2048], nibble-packed (2 pixel per byte), 4-bit -> <<4 */
static void decode_frame_to_rgb565(const uint8_t *f, uint16_t *out565) {
    int i, j, z;
    int rbase, gbase, bbase;

    rbase = 0;
    gbase = 2048;
    bbase = 4096;

    z = 0;
    for (j = 0; j < H; j++) {
        for (i = 0; i < W; i++) {
            uint8_t r, g, b;
            uint16_t pix;

            if ((i & 1) == 0) {
                r = f[z + rbase] & 0xF0;
                g = f[z + gbase] & 0xF0;
                b = f[z + bbase] & 0xF0;
            } else {
                r = (uint8_t)((f[z + rbase] & 0x0F) << 4);
                g = (uint8_t)((f[z + gbase] & 0x0F) << 4);
                b = (uint8_t)((f[z + bbase] & 0x0F) << 4);
                z++;
            }

            /* RGB565 */
            pix = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
            *out565++ = pix;
        }
    }
}

static uint32_t now_ms(void) {
    return (uint32_t)SDL_GetTicks();
}

int main(int argc, char **argv) {
    const char *host;
    int port, scale,fd, running;
    uint8_t macip[16], ip4[4], frame[LEN], rssi;
    uint16_t pix565[W * H];
    SDL_Window *win;
    SDL_Renderer *ren;
    SDL_Texture *tex;
    SDL_Event ev;
    uint32_t nextSerMs;

    /* unico argomento obbligatorio: scala */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <scale>\n", argv[0]);
        fprintf(stderr, "Example: %s 10\n", argv[0]);
        return 1;
    }

    scale = atoi(argv[1]);

    host = HOST_DEFAULT;   /* display.mazzini.org */
    port = PORT_DEFAULT;   /* 5000 */
    memset(macip, 0, sizeof(macip));
    memcpy(macip, MAC, 12);

    fd = connect_tcp_dns(host, port);
    if (fd < 0) {
        fprintf(stderr, "connect %s:%d failed\n", host, port);
        return 1;
    }

    get_local_ip4_from_socket(fd, ip4);
    macip[12] = ip4[0];
    macip[13] = ip4[1];
    macip[14] = ip4[2];
    macip[15] = ip4[3];

    if (sendn(fd, macip, 16) != 16) {
        fprintf(stderr, "handshake send failed\n");
        close(fd);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        close(fd);
        return 1;
    }

    win = SDL_CreateWindow(
        "Virtual Display 64x64 (ESP protocol)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        W * scale, H * scale,
        SDL_WINDOW_SHOWN
    );
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        close(fd);
        return 1;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        close(fd);
        return 1;
    }

    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, W, H);
    if (!tex) {
        fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        close(fd);
        return 1;
    }

    running = 1;
    nextSerMs = now_ms() + 30000U;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = 0;
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) running = 0;
        }

        /* keepalive SER ogni 30s, come ESP */
        if ((int32_t)(now_ms() - nextSerMs) >= 0) {
            get_local_ip4_from_socket(fd, ip4);
            macip[12] = ip4[0];
            macip[13] = ip4[1];
            macip[14] = ip4[2];
            macip[15] = ip4[3];

            if (sendn(fd, macip, 16) != 16) break;
            nextSerMs = now_ms() + 30000U;
        }

        if (recvn(fd, frame, LEN) != LEN) {
            fprintf(stderr, "recv closed\n");
            break;
        }

        rssi = 127;
        if (sendn(fd, &rssi, 1) != 1) {
            fprintf(stderr, "send rssi failed\n");
            break;
        }

        decode_frame_to_rgb565(frame, pix565);

        SDL_UpdateTexture(tex, NULL, pix565, W * (int)sizeof(uint16_t));
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    close(fd);
    return 0;
}
