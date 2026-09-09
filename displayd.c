// Gianluca Mazzini @2026- Version 1.25
// RGB888 display daemon with integrated renderer and video engine.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define BASE_DIR "/home/tools/mcp/work/display"
#define DEFAULT_PORT 5002
#define DEFAULT_CONTROL_SOCKET BASE_DIR "/displayd.sock"
#define PID_FILE BASE_DIR "/displayd.pid"
#define LOG_FILE BASE_DIR "/displayd.log"
#define VIDEO_DIR BASE_DIR "/tmpdata/video"
#define VIDEO_CONFIG BASE_DIR "/video.conf"
#define IMAGE_DIR BASE_DIR "/tmpdata/images"
#define PGR_DIR BASE_DIR "/tmpdata/pgr"
#define FONT_DIR BASE_DIR "/font"

#define DISPLAYD_VERSION "1.25"

#define WIDTH 64
#define HEIGHT 64
#define PIXELS (WIDTH * HEIGHT)
#define FRAME_LEN (PIXELS * 3)
#define FF_HEADER_LEN 16
#define FF_PIXEL_BYTES 8
#define MAX_THREADS 100
#define MAX_VIDEOS 64
#define VIDEO_NAME_LEN 64

#define MAX_PROGRAM_BLOCKS 100
#define MAX_PROGRAM_OPS 128
#define MAX_RPN_TOKENS 32
#define MAX_STEPS 10000
#define RPN_STACK_MAX 50
#define RENDER_MAX_ARGS 8
#define RENDER_TOKEN_LEN 128
#define RENDER_TEXT_LEN 256
#define MAX_STATES    (MAX_THREADS * 4)

#define CONTROL_RECV_TIMEOUT_SEC 3
#define CONTROL_SEND_TIMEOUT_SEC 3
#define CGI_MAX_BODY 65536U
#define MAX_FONT_CACHE 64
#define MAX_FONT_NAME 64
#define MAX_FONT_SPEC 128
#define MAX_FONT_SIZE 64
#define MAX_FONT_WIDTH 64
#define FONT_FIRST_CHAR 32
#define FONT_LAST_CHAR 126
#define FONT_GLYPHS (FONT_LAST_CHAR - FONT_FIRST_CHAR + 1)

struct myThread {
  volatile int active;
  char ser[13];
  char ip[16];
  unsigned short port;
  volatile unsigned long step;
  volatile int force_step_pending;
  unsigned long force_step;
  time_t epoch;
  int fd;
  signed char rssi;
  int mir;
  unsigned long t;
  unsigned long gen;
  unsigned char bin[FRAME_LEN];
};

struct clientState {
  int used;
  char ser[13];
  unsigned long step;
  time_t epoch;
};

struct video {
  int used;
  unsigned long base;
  unsigned long frames;
  size_t bytes;
  char name[VIDEO_NAME_LEN];
  unsigned char *data;
};

struct renderContext {
  unsigned char *frame;
  unsigned char vars[10][3];
};

enum renderOpcode {
  RENDER_NONE,
  RENDER_TEXT,
  RENDER_RECT,
  RENDER_LINE,
  RENDER_PIXEL,
  RENDER_IMAGE,
  RENDER_COPY,
  RENDER_SAMPLE,
  RENDER_AVERAGE,
  RENDER_AVERAGELINE
};

struct renderCommand {
  int op;
  int argc;
  char arg[RENDER_MAX_ARGS][RENDER_TOKEN_LEN];
  char text[RENDER_TEXT_LEN];
};

enum programBlockType {
  PROGRAM_FRAME,
  PROGRAM_VIDEO
};

enum programInstructionType {
  PROGRAM_RAND,
  PROGRAM_CALC,
  PROGRAM_RENDER
};

enum rpnTokenType {
  RPN_VALUE,
  RPN_VARIABLE,
  RPN_OPERATOR
};

struct rpnToken {
  int type;
  int value;
  char op;
};

struct programInstruction {
  int type;
  int var;
  int min;
  int max;
  char fmt[20];
  int rpn_count;
  struct rpnToken rpn[MAX_RPN_TOKENS];
  struct renderCommand render;
};

struct programBlock {
  int type;
  int first_step;
  int last_step;
  int interval_ms;
  unsigned long video_base;
  int first_op;
  int op_count;
};

struct program {
  int block_count;
  int op_count;
  int total_steps;
  struct programBlock block[MAX_PROGRAM_BLOCKS];
  struct programInstruction op[MAX_PROGRAM_OPS];
};

struct fontGlyph {
  unsigned char *bitmap;
  int width;
  int height;
  int left;
  int top;
  int advance;
  int visible;
  int visible_left;
  int visible_top;
  int visible_right;
  int visible_bottom;
  int loaded;
};

struct fontCache {
  int used;
  char name[MAX_FONT_NAME];
  int size;
  int fixed_width;
  int ascender;
  int line_height;
  unsigned long renders;
  size_t bytes;
  FT_Face face;
  struct fontGlyph glyphs[FONT_GLYPHS];
};

struct myThread mythr[MAX_THREADS];
static struct clientState mystate[MAX_STATES];
static struct video videos[MAX_VIDEOS];
static struct fontCache font_cache[MAX_FONT_CACHE];
static FT_Library font_library;
static int font_library_ready;
static time_t server_startup_time = 0;
static int listen_port = DEFAULT_PORT;
static int daemon_mode;
static int pid_fd = -1;
static char control_socket[sizeof(((struct sockaddr_un *)0)->sun_path)] = DEFAULT_CONTROL_SOCKET;


/*
 * session_gen is incremented for every accepted physical session.
 * It prevents an old thread from clearing a monitor slot that has
 * already been reused by a newer session.
 */
static unsigned long session_gen = 0;

pthread_mutex_t mon_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t video_rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t font_mutex = PTHREAD_MUTEX_INITIALIZER;

static long elapsed_since_epoch(time_t now, time_t epoch) {
  if (epoch <= 0) return 0;
  if (now < epoch) return 0;
  return (long)(now - epoch);
}

static int send_all(int fd, const void *buf, size_t len) {
  const char *p;
  size_t sent;
  ssize_t r;

  p = (const char *)buf;
  sent = 0;

  for (; sent < len; ) {
    r = send(fd, p + sent, len - sent, 0);
    if (r <= 0) {
      if (r < 0 && errno == EINTR) continue;
      return -1;
    }
    sent += (size_t)r;
  }

  return 0;
}

static int read_full(int fd, void *buf, size_t len) {
  char *p;
  size_t got;
  ssize_t r;

  p = (char *)buf;
  got = 0;

  for (; got < len; ) {
    r = recv(fd, p + got, len - got, 0);
    if (r <= 0) {
      if (r < 0 && errno == EINTR) continue;
      return -1;
    }
    got += (size_t)r;
  }

  return 0;
}

/*
 * This helper must be called with mon_mutex held.
 * It only shuts the socket down. The owner thread will close it.
 * This avoids double-close and file-descriptor reuse hazards.
 */
static void shutdown_session_locked(int idx, const char *reason) {
  time_t now;
  long depoch;

  if (idx < 0 || idx >= MAX_THREADS) return;
  if (!mythr[idx].active) return;

  now = time(NULL);
  depoch = elapsed_since_epoch(now, mythr[idx].epoch);

  printf("Session shutdown: reason=%s idx=%d serial=%s peer=%s:%u step=%lu depoch=%ld rssi=%d gen=%lu\n",
         reason ? reason : "unknown",
         idx,
         mythr[idx].ser,
         mythr[idx].ip,
         (unsigned)mythr[idx].port,
         mythr[idx].step,
         depoch,
         (int)mythr[idx].rssi,
         mythr[idx].gen);
  fflush(stdout);

  if (mythr[idx].fd >= 0) {
    shutdown(mythr[idx].fd, SHUT_RDWR);
  }

  /*
   * Mark inactive immediately so the monitor no longer reports the old
   * session and so a new connection can claim a slot.
   *
   * The owner thread may still be running for a short time, but all of its
   * monitor updates are protected by the generation check.
   */
  mythr[idx].active = 0;
  mythr[idx].force_step_pending = 0;
  mythr[idx].force_step = 0;
  mythr[idx].mir = -1;
  mythr[idx].t = 0;
}

/*
 * This helper must be called with mon_mutex held.
 */
static int session_is_current_locked(int idx, unsigned long gen) {
  if (idx < 0 || idx >= MAX_THREADS) return 0;
  return mythr[idx].active && mythr[idx].gen == gen;
}

/*
 * These helpers must be called with mon_mutex held.
 * mystate[] is the logical per-serial state.
 * mythr[] is only the physical TCP session state.
 */
static int state_find_locked(const char *ser) {
  int i;

  for (i = 0; i < MAX_STATES; i++) {
    if (mystate[i].used && strncmp(mystate[i].ser, ser, 12) == 0) {
      return i;
    }
  }

  return -1;
}

static int state_get_or_create_locked(const char *ser) {
  int i;

  i = state_find_locked(ser);
  if (i >= 0) return i;

  for (i = 0; i < MAX_STATES; i++) {
    if (!mystate[i].used) {
      mystate[i].used = 1;
      strncpy(mystate[i].ser, ser, 12);
      mystate[i].ser[12] = 0;
      mystate[i].step = 0;
      mystate[i].epoch = 0;
      return i;
    }
  }

  return -1;
}

static void update_step_if_current(int idx, unsigned long gen, unsigned long step) {
  int si;

  pthread_mutex_lock(&mon_mutex);

  if (session_is_current_locked(idx, gen)) {
    mythr[idx].step = step;

    si = state_get_or_create_locked(mythr[idx].ser);
    if (si >= 0 && step > mystate[si].step) {
      mystate[si].step = step;
    }
  }

  pthread_mutex_unlock(&mon_mutex);
}

static int take_forced_step_if_current(int idx, unsigned long gen, unsigned long *step_out) {
  int si, changed;

  changed = 0;

  pthread_mutex_lock(&mon_mutex);

  if (session_is_current_locked(idx, gen) && mythr[idx].force_step_pending) {
    *step_out = mythr[idx].force_step;
    mythr[idx].step = *step_out;
    mythr[idx].force_step_pending = 0;

    si = state_get_or_create_locked(mythr[idx].ser);
    if (si >= 0) {
      mystate[si].step = *step_out;
    }

    changed = 1;
  }

  pthread_mutex_unlock(&mon_mutex);

  return changed;
}

static void update_rssi_if_current(int idx, unsigned long gen, signed char rssi) {
  pthread_mutex_lock(&mon_mutex);
  if (session_is_current_locked(idx, gen)) {
    mythr[idx].rssi = rssi;
  }
  pthread_mutex_unlock(&mon_mutex);
}

static void update_frame_if_current(int idx, unsigned long gen, const unsigned char *frame, unsigned long t) {
  pthread_mutex_lock(&mon_mutex);
  if (session_is_current_locked(idx, gen)) {
    memcpy(mythr[idx].bin, frame, FRAME_LEN);
    mythr[idx].t = t;
  }
  pthread_mutex_unlock(&mon_mutex);
}

static int valid_serial(const char *ser) {
  int i;

  if (strlen(ser) != 12) return 0;
  for (i = 0; i < 12; i++) {
    if (!isxdigit((unsigned char)ser[i])) return 0;
  }
  return 1;
}

static int valid_name(const char *name);

static int hex_digit(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

static int hex_byte(const char *s, unsigned int *value) {
  int hi, lo;

  hi = hex_digit((unsigned char)s[0]);
  lo = hex_digit((unsigned char)s[1]);
  if (hi < 0 || lo < 0) return -1;
  *value = (unsigned int)((hi << 4) | lo);
  return 0;
}

static int next_token(char **pp, char *dst, size_t size) {
  char *p;
  size_t n;

  p = *pp;
  for (; *p == ' ' || *p == '\t'; p++) {
  }
  if (*p == 0) return 0;

  n = 0;
  for (; *p != 0 && *p != ' ' && *p != '\t'; p++) {
    if (n + 1 < size) dst[n++] = *p;
  }
  dst[n] = 0;
  *pp = p;
  return 1;
}

static void keyword_upper(char *text) {
  unsigned char *p;

  for (p = (unsigned char *)text; *p != 0; p++) *p = (unsigned char)toupper(*p);
}

static int token_int(const char *token, int *value) {
  char *endp;
  long n;

  errno = 0;
  endp = 0;
  n = strtol(token, &endp, 10);
  if (errno != 0 || endp == token || *endp != 0 || n < INT_MIN || n > INT_MAX) return -1;
  *value = (int)n;
  return 0;
}

static int token_var(const char *token, int *value) {
  const char *p;

  p = token;
  if (!isdigit((unsigned char)*p)) p++;
  return token_int(p, value);
}

static int color_token(struct renderContext *ctx, const char *token,
                       unsigned int *r, unsigned int *g, unsigned int *b, unsigned int *a) {
  int idx;

  if (token[0] == 'V') {
    if (strlen(token) != 5 || token[1] < '0' || token[1] > '9') return -1;
    idx = token[1] - '0';
    *r = ctx->vars[idx][0];
    *g = ctx->vars[idx][1];
    *b = ctx->vars[idx][2];

    switch (token[2]) {
      case 'n':
        break;
      case 'i':
        *r = 255U - *r;
        *g = 255U - *g;
        *b = 255U - *b;
        break;
      case 'e':
        *r += *r / 2U;
        *g += *g / 2U;
        *b += *b / 2U;
        if (*r > 255U) *r = 255U;
        if (*g > 255U) *g = 255U;
        if (*b > 255U) *b = 255U;
        break;
      case 'r':
        *r -= *r / 2U;
        *g -= *g / 2U;
        *b -= *b / 2U;
        break;
      default:
        return -1;
    }

    return hex_byte(token + 3, a);
  }

  if (strlen(token) != 8) return -1;
  if (hex_byte(token, r) != 0 ||
      hex_byte(token + 2, g) != 0 ||
      hex_byte(token + 4, b) != 0 ||
      hex_byte(token + 6, a) != 0) return -1;
  return 0;
}

static void blend_pixel(unsigned char *frame, int pos,
                        unsigned int r, unsigned int g, unsigned int b, unsigned int alpha) {
  unsigned int inv;

  if (pos < 0 || pos >= PIXELS) return;
  inv = 255U - alpha;
  frame[pos] = (unsigned char)((r * alpha + (unsigned int)frame[pos] * inv) / 255U);
  frame[PIXELS + pos] = (unsigned char)((g * alpha + (unsigned int)frame[PIXELS + pos] * inv) / 255U);
  frame[PIXELS * 2 + pos] = (unsigned char)((b * alpha + (unsigned int)frame[PIXELS * 2 + pos] * inv) / 255U);
}

static int load_farbfeld_image(const char *name, unsigned char *image) {
  unsigned char header[FF_HEADER_LEN], pixel[FF_PIXEL_BYTES];
  char path[256];
  FILE *fp;
  int i;

  if (!valid_name(name)) return -1;
  if (snprintf(path, sizeof(path), "%s/%s.ff", IMAGE_DIR, name) >= (int)sizeof(path)) return -1;

  fp = fopen(path, "rb");
  if (fp == 0) return -1;

  if (fread(header, 1, sizeof(header), fp) != sizeof(header) ||
      memcmp(header, "farbfeld", 8) != 0 ||
      header[8] != 0 || header[9] != 0 || header[10] != 0 || header[11] != WIDTH ||
      header[12] != 0 || header[13] != 0 || header[14] != 0 || header[15] != HEIGHT) {
    fclose(fp);
    return -1;
  }

  for (i = 0; i < PIXELS; i++) {
    if (fread(pixel, 1, sizeof(pixel), fp) != sizeof(pixel)) {
      fclose(fp);
      return -1;
    }
    image[i] = pixel[0];
    image[PIXELS + i] = pixel[2];
    image[PIXELS * 2 + i] = pixel[4];
  }

  fclose(fp);
  return 0;
}

static int valid_font_name(const char *name) {
  const unsigned char *p;

  if (name == 0 || name[0] == 0) return 0;
  for (p = (const unsigned char *)name; *p != 0; p++) {
    if (!isalnum(*p) && *p != '_' && *p != '-') return 0;
  }
  return 1;
}

static int parse_font_number(const char *text, int min, int max, int *value) {
  char *endp;
  long n;

  errno = 0;
  endp = 0;
  n = strtol(text, &endp, 10);
  if (errno != 0 || endp == text || *endp != 0 || n < min || n > max) return -1;
  *value = (int)n;
  return 0;
}

static int parse_font_spec(const char *spec, char *name, size_t name_size,
                           int *size, int *fixed_width) {
  char work[MAX_FONT_SPEC], *p, *q;
  size_t len;

  if (spec == 0 || strlen(spec) >= sizeof(work)) return -1;
  strcpy(work, spec);
  p = strchr(work, ':');
  if (p == 0) return -1;
  *p++ = 0;
  q = strchr(p, ':');
  if (q != 0) *q++ = 0;
  if (strchr(p, ':') != 0 || (q != 0 && strchr(q, ':') != 0)) return -1;
  if (!valid_font_name(work)) return -1;
  len = strlen(work);
  if (len + 1 > name_size) return -1;
  memcpy(name, work, len + 1);
  if (parse_font_number(p, 1, MAX_FONT_SIZE, size) != 0) return -1;
  *fixed_width = 0;
  if (q != 0 && parse_font_number(q, 1, MAX_FONT_WIDTH, fixed_width) != 0) return -1;
  return 0;
}

static int font_library_init_locked(void) {
  if (font_library_ready) return 0;
  if (FT_Init_FreeType(&font_library) != 0) return -1;
  font_library_ready = 1;
  return 0;
}

static struct fontCache *font_cache_get(const char *spec) {
  struct fontCache *font;
  char name[MAX_FONT_NAME], path[256];
  int size, fixed_width, i, result;

  if (parse_font_spec(spec, name, sizeof(name), &size, &fixed_width) != 0) return 0;
  pthread_mutex_lock(&font_mutex);
  for (i = 0; i < MAX_FONT_CACHE; i++) {
    if (!font_cache[i].used) continue;
    if (font_cache[i].size == size && font_cache[i].fixed_width == fixed_width &&
        strcmp(font_cache[i].name, name) == 0) {
      font_cache[i].renders++;
      font = &font_cache[i];
      pthread_mutex_unlock(&font_mutex);
      return font;
    }
  }

  if (font_library_init_locked() != 0) {
    pthread_mutex_unlock(&font_mutex);
    return 0;
  }

  font = 0;
  for (i = 0; i < MAX_FONT_CACHE; i++) {
    if (!font_cache[i].used) {
      font = &font_cache[i];
      break;
    }
  }
  if (font == 0) {
    pthread_mutex_unlock(&font_mutex);
    return 0;
  }

  memset(font, 0, sizeof(*font));
  if (snprintf(path, sizeof(path), "%s/%s.ttf", FONT_DIR, name) >= (int)sizeof(path)) {
    pthread_mutex_unlock(&font_mutex);
    return 0;
  }
  result = FT_New_Face(font_library, path, 0, &font->face);
  if (result != 0) {
    if (snprintf(path, sizeof(path), "%s/%s.otf", FONT_DIR, name) >= (int)sizeof(path)) {
      pthread_mutex_unlock(&font_mutex);
      return 0;
    }
    result = FT_New_Face(font_library, path, 0, &font->face);
  }
  if (result != 0 || FT_Set_Pixel_Sizes(font->face, 0, (FT_UInt)size) != 0) {
    if (font->face != 0) FT_Done_Face(font->face);
    memset(font, 0, sizeof(*font));
    pthread_mutex_unlock(&font_mutex);
    return 0;
  }

  font->used = 1;
  memcpy(font->name, name, strlen(name) + 1);
  font->size = size;
  font->fixed_width = fixed_width;
  font->ascender = (int)((font->face->size->metrics.ascender + 63) >> 6);
  font->line_height = (int)((font->face->size->metrics.height + 63) >> 6);
  if (font->ascender < 1) font->ascender = size;
  if (font->line_height < 1) font->line_height = size;
  font->renders = 1;
  pthread_mutex_unlock(&font_mutex);
  return font;
}

static int font_bitmap_value(const FT_Bitmap *bitmap, int x, int y) {
  const unsigned char *row;
  int pitch, value;

  pitch = bitmap->pitch;
  if (pitch >= 0) row = bitmap->buffer + (size_t)y * (size_t)pitch;
  else row = bitmap->buffer + (size_t)(bitmap->rows - 1U - (unsigned int)y) * (size_t)(-pitch);

  if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
    value = row[x];
    if (bitmap->num_grays > 1 && bitmap->num_grays != 256)
      value = value * 255 / ((int)bitmap->num_grays - 1);
    return value;
  }
  if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO)
    return (row[x >> 3] & (0x80U >> (x & 7))) != 0 ? 255 : 0;
  return -1;
}

static struct fontGlyph *font_glyph_get(struct fontCache *font, int code) {
  struct fontGlyph *glyph;
  FT_GlyphSlot slot;
  FT_Bitmap *bitmap;
  unsigned char *data;
  size_t bytes;
  int x, y, value, load_flags;

  if (code < FONT_FIRST_CHAR || code > FONT_LAST_CHAR) code = '?';
  glyph = &font->glyphs[code - FONT_FIRST_CHAR];
  pthread_mutex_lock(&font_mutex);
  if (glyph->loaded) {
    pthread_mutex_unlock(&font_mutex);
    return glyph;
  }

  if (font->size <= 9 || font->fixed_width != 0)
    load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_MONO | FT_LOAD_MONOCHROME;
  else load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL;

  if (FT_Load_Char(font->face, (FT_ULong)code, load_flags) != 0) {
    pthread_mutex_unlock(&font_mutex);
    return 0;
  }
  slot = font->face->glyph;
  bitmap = &slot->bitmap;
  if (bitmap->pixel_mode != FT_PIXEL_MODE_GRAY && bitmap->pixel_mode != FT_PIXEL_MODE_MONO) {
    pthread_mutex_unlock(&font_mutex);
    return 0;
  }
  if (font->fixed_width != 0 && (int)bitmap->width > font->fixed_width) {
    pthread_mutex_unlock(&font_mutex);
    return 0;
  }

  bytes = (size_t)bitmap->width * (size_t)bitmap->rows;
  data = 0;
  glyph->visible = 0;
  if (bytes != 0) {
    data = (unsigned char *)malloc(bytes);
    if (data == 0) {
      pthread_mutex_unlock(&font_mutex);
      return 0;
    }
    for (y = 0; y < (int)bitmap->rows; y++) {
      for (x = 0; x < (int)bitmap->width; x++) {
        value = font_bitmap_value(bitmap, x, y);
        if (value < 0) {
          free(data);
          pthread_mutex_unlock(&font_mutex);
          return 0;
        }
        data[(size_t)y * bitmap->width + (size_t)x] = (unsigned char)value;
        if (value != 0) {
          if (!glyph->visible) {
            glyph->visible_left = x;
            glyph->visible_top = y;
            glyph->visible_right = x;
            glyph->visible_bottom = y;
            glyph->visible = 1;
          }
          else {
            if (x < glyph->visible_left) glyph->visible_left = x;
            if (y < glyph->visible_top) glyph->visible_top = y;
            if (x > glyph->visible_right) glyph->visible_right = x;
            if (y > glyph->visible_bottom) glyph->visible_bottom = y;
          }
        }
      }
    }
  }

  glyph->bitmap = data;
  glyph->width = (int)bitmap->width;
  glyph->height = (int)bitmap->rows;
  glyph->left = slot->bitmap_left;
  glyph->top = slot->bitmap_top;
  glyph->advance = (int)((slot->advance.x + 32) >> 6);
  if (glyph->advance < 1) glyph->advance = 1;
  glyph->loaded = 1;
  font->bytes += bytes;
  pthread_mutex_unlock(&font_mutex);
  return glyph;
}

static void font_cache_report(int fd) {
  char line[512], chars[256], spec[MAX_FONT_SPEC];
  size_t used;
  int i, j, count, len, c;

  pthread_mutex_lock(&font_mutex);
  count = 0;
  for (i = 0; i < MAX_FONT_CACHE; i++) if (font_cache[i].used) count++;
  len = snprintf(line, sizeof(line), "font_cache=%d/%d\n", count, MAX_FONT_CACHE);
  send_all(fd, line, (size_t)len);

  for (i = 0; i < MAX_FONT_CACHE; i++) {
    if (!font_cache[i].used) continue;
    if (font_cache[i].fixed_width != 0)
      snprintf(spec, sizeof(spec), "%s:%d:%d", font_cache[i].name,
               font_cache[i].size, font_cache[i].fixed_width);
    else snprintf(spec, sizeof(spec), "%s:%d", font_cache[i].name, font_cache[i].size);

    used = 0;
    chars[used++] = '"';
    count = 0;
    for (j = 0; j < FONT_GLYPHS; j++) {
      if (!font_cache[i].glyphs[j].loaded) continue;
      count++;
      c = FONT_FIRST_CHAR + j;
      if ((c == '"' || c == '\\') && used + 2 < sizeof(chars)) chars[used++] = '\\';
      if (used + 1 < sizeof(chars)) chars[used++] = (char)c;
    }
    if (used + 2 <= sizeof(chars)) chars[used++] = '"';
    chars[used] = 0;

    len = snprintf(line, sizeof(line),
                   "%s mode=%s renders=%lu glyphs=%d bitmap_bytes=%lu line=%d ascender=%d chars=%s\n",
                   spec, (font_cache[i].size <= 9 || font_cache[i].fixed_width != 0) ? "mono" : "gray",
                   font_cache[i].renders, count, (unsigned long)font_cache[i].bytes,
                   font_cache[i].line_height, font_cache[i].ascender, chars);
    send_all(fd, line, (size_t)len);
  }
  pthread_mutex_unlock(&font_mutex);
}

static void render_begin(struct renderContext *ctx, unsigned char *frame) {
  ctx->frame = frame;
  memset(frame, 0, FRAME_LEN);
  memset(ctx->vars, 0, sizeof(ctx->vars));
}

static int render_fixed_args(char **pp, struct renderCommand *cmd, int argc) {
  char extra[RENDER_TOKEN_LEN];
  int i;

  if (argc < 0 || argc > RENDER_MAX_ARGS) return -1;
  cmd->argc = argc;
  for (i = 0; i < argc; i++) {
    if (!next_token(pp, cmd->arg[i], sizeof(cmd->arg[i]))) return -1;
  }
  if (next_token(pp, extra, sizeof(extra))) return -1;
  return 0;
}

static int render_parse_line(const char *line, struct renderCommand *cmd) {
  char work[512], keyword[32];
  char *p;
  size_t len;

  if (line == 0 || cmd == 0 || strlen(line) >= sizeof(work)) return -1;
  memset(cmd, 0, sizeof(*cmd));
  strcpy(work, line);
  p = work;
  for (; *p == ' ' || *p == '\t'; p++) {
  }
  if (*p == 0 || *p == '#') {
    cmd->op = RENDER_NONE;
    return 0;
  }
  if (!next_token(&p, keyword, sizeof(keyword))) return -1;
  keyword_upper(keyword);

  if (strcmp(keyword, "TEXT") == 0) {
    cmd->op = RENDER_TEXT;
    cmd->argc = 5;
    for (len = 0; len < 5; len++) {
      if (!next_token(&p, cmd->arg[len], sizeof(cmd->arg[len]))) return -1;
    }
    for (; *p == ' ' || *p == '\t'; p++) {
    }
    if (strlen(p) >= sizeof(cmd->text)) return -1;
    strcpy(cmd->text, p);
    return 0;
  }
  if (strcmp(keyword, "RECT") == 0) {
    cmd->op = RENDER_RECT;
    return render_fixed_args(&p, cmd, 5);
  }
  if (strcmp(keyword, "LINE") == 0) {
    cmd->op = RENDER_LINE;
    return render_fixed_args(&p, cmd, 5);
  }
  if (strcmp(keyword, "PIXEL") == 0) {
    cmd->op = RENDER_PIXEL;
    return render_fixed_args(&p, cmd, 3);
  }
  if (strcmp(keyword, "IMAGE") == 0) {
    cmd->op = RENDER_IMAGE;
    return render_fixed_args(&p, cmd, 2);
  }
  if (strcmp(keyword, "COPY") == 0) {
    cmd->op = RENDER_COPY;
    return render_fixed_args(&p, cmd, 8);
  }
  if (strcmp(keyword, "SAMPLE") == 0) {
    cmd->op = RENDER_SAMPLE;
    return render_fixed_args(&p, cmd, 3);
  }
  if (strcmp(keyword, "AVERAGE") == 0) {
    cmd->op = RENDER_AVERAGE;
    return render_fixed_args(&p, cmd, 5);
  }
  if (strcmp(keyword, "AVERAGELINE") == 0) {
    cmd->op = RENDER_AVERAGELINE;
    return render_fixed_args(&p, cmd, 5);
  }
  return -1;
}

static int expand_vars(const char *src, char vars[30][30], char *dst, size_t size) {
  const char *p, *q, *value;
  size_t used, len;
  int idx;

  used = 0;
  for (p = src; *p != 0; ) {
    if (*p != '@' || vars == 0 || !isdigit((unsigned char)p[1])) {
      if (used + 1 >= size) return -1;
      dst[used++] = *p++;
      continue;
    }

    q = p + 1;
    idx = 0;
    for (; isdigit((unsigned char)*q); q++) {
      idx = idx * 10 + (*q - '0');
      if (idx >= 30) return -1;
    }
    value = vars[idx];
    len = strlen(value);
    if (used + len >= size) return -1;
    memcpy(dst + used, value, len);
    used += len;
    p = q;
  }
  dst[used] = 0;
  return 0;
}

static int render_arg(const struct renderCommand *cmd, int index, char vars[30][30],
                      char *dst, size_t size) {
  if (index < 0 || index >= cmd->argc) return -1;
  return expand_vars(cmd->arg[index], vars, dst, size);
}

static int render_execute(struct renderContext *ctx, const struct renderCommand *cmd,
                          char vars[30][30]) {
  unsigned char image[FRAME_LEN];
  char token[RENDER_TOKEN_LEN], color1[16], color2[16], image_name[VIDEO_NAME_LEN];
  char font_spec[MAX_FONT_SPEC], text[512];
  struct fontCache *font;
  struct fontGlyph *glyph;
  unsigned int r, g, b, a, rb, gb, bb, ab, coverage, alpha;
  unsigned long sr, sg, sb, count;
  int x, y, x2, y2, dx, dy, dstx, dsty, var;
  int i, j, k, width, pos, steps, cursor, baseline, gx, gy;
  int text_left, text_top, text_right, text_bottom, visible;
  int glyph_left, glyph_top, glyph_right, glyph_bottom, box_width, box_height;
  double x1d, y1d, x2d, y2d, len, ad, bd, dd, xd, yd;

  if (ctx == 0 || cmd == 0) return -1;

  switch (cmd->op) {
    case RENDER_NONE:
      return 0;

    case RENDER_TEXT:
      if (render_arg(cmd, 0, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 2, vars, color1, sizeof(color1)) != 0 ||
          render_arg(cmd, 3, vars, color2, sizeof(color2)) != 0 ||
          render_arg(cmd, 4, vars, font_spec, sizeof(font_spec)) != 0 ||
          expand_vars(cmd->text, vars, text, sizeof(text)) != 0) return -1;
      if (color_token(ctx, color1, &r, &g, &b, &a) != 0 ||
          color_token(ctx, color2, &rb, &gb, &bb, &ab) != 0) return -1;
      font = font_cache_get(font_spec);
      if (font == 0) return -1;

      text_left = 0;
      text_top = 0;
      text_right = 0;
      text_bottom = 0;
      visible = 0;
      cursor = 0;
      for (k = 0; text[k] != 0; k++) {
        glyph = font_glyph_get(font, (unsigned char)text[k]);
        if (glyph == 0) return -1;
        width = font->fixed_width != 0 ? font->fixed_width : glyph->advance;
        if (font->fixed_width != 0) gx = cursor + (font->fixed_width - glyph->width) / 2;
        else gx = cursor + glyph->left;
        if (glyph->visible) {
          glyph_left = gx + glyph->visible_left;
          glyph_right = gx + glyph->visible_right;
          glyph_top = -glyph->top + glyph->visible_top;
          glyph_bottom = -glyph->top + glyph->visible_bottom;
          if (!visible) {
            text_left = glyph_left;
            text_top = glyph_top;
            text_right = glyph_right;
            text_bottom = glyph_bottom;
            visible = 1;
          }
          else {
            if (glyph_left < text_left) text_left = glyph_left;
            if (glyph_top < text_top) text_top = glyph_top;
            if (glyph_right > text_right) text_right = glyph_right;
            if (glyph_bottom > text_bottom) text_bottom = glyph_bottom;
          }
        }
        cursor += width;
      }
      if (visible) {
        box_width = text_right - text_left + 1;
        box_height = text_bottom - text_top + 1;
        if (x == -1) cursor = WIDTH - 1 - text_right;
        else if (x == -2) cursor = (WIDTH - box_width) / 2 - text_left;
        else cursor = x - text_left;

        if (y == -1) baseline = HEIGHT - 1 - text_bottom;
        else if (y == -2) baseline = (HEIGHT - box_height) / 2 - text_top;
        else baseline = y - text_top;

        if (ab != 0) {
          for (j = baseline + text_top; j <= baseline + text_bottom; j++) {
            if (j < 0 || j >= HEIGHT) continue;
            for (i = cursor + text_left; i <= cursor + text_right; i++) {
              if (i >= 0 && i < WIDTH)
                blend_pixel(ctx->frame, j * WIDTH + i, rb, gb, bb, ab);
            }
          }
        }
      }
      else {
        cursor = x < 0 ? 0 : x;
        baseline = y < 0 ? 0 : y;
      }

      for (k = 0; text[k] != 0; k++) {
        glyph = font_glyph_get(font, (unsigned char)text[k]);
        if (glyph == 0) return -1;
        width = font->fixed_width != 0 ? font->fixed_width : glyph->advance;
        if (font->fixed_width != 0) gx = cursor + (font->fixed_width - glyph->width) / 2;
        else gx = cursor + glyph->left;
        gy = baseline - glyph->top;
        for (j = 0; j < glyph->height; j++) {
          dsty = gy + j;
          if (dsty < 0 || dsty >= HEIGHT) continue;
          for (i = 0; i < glyph->width; i++) {
            dstx = gx + i;
            if (dstx < 0 || dstx >= WIDTH) continue;
            coverage = glyph->bitmap[(size_t)j * (size_t)glyph->width + (size_t)i];
            if (coverage == 0) continue;
            alpha = (a * coverage + 127U) / 255U;
            blend_pixel(ctx->frame, dsty * WIDTH + dstx, r, g, b, alpha);
          }
        }
        cursor += width;
      }
      return 0;

    case RENDER_RECT:
      if (render_arg(cmd, 0, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 2, vars, token, sizeof(token)) != 0 || token_int(token, &x2) != 0 ||
          render_arg(cmd, 3, vars, token, sizeof(token)) != 0 || token_int(token, &y2) != 0 ||
          render_arg(cmd, 4, vars, color1, sizeof(color1)) != 0 ||
          color_token(ctx, color1, &r, &g, &b, &a) != 0) return -1;
      if (x2 < x) { i = x; x = x2; x2 = i; }
      if (y2 < y) { i = y; y = y2; y2 = i; }
      if (x < 0) x = 0;
      if (y < 0) y = 0;
      if (x2 >= WIDTH) x2 = WIDTH - 1;
      if (y2 >= HEIGHT) y2 = HEIGHT - 1;
      for (j = y; j <= y2; j++) {
        for (i = x; i <= x2; i++) blend_pixel(ctx->frame, j * WIDTH + i, r, g, b, a);
      }
      return 0;

    case RENDER_LINE:
      if (render_arg(cmd, 0, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 2, vars, token, sizeof(token)) != 0 || token_int(token, &x2) != 0 ||
          render_arg(cmd, 3, vars, token, sizeof(token)) != 0 || token_int(token, &y2) != 0 ||
          render_arg(cmd, 4, vars, color1, sizeof(color1)) != 0 ||
          color_token(ctx, color1, &r, &g, &b, &a) != 0) return -1;
      x1d = (double)x;
      y1d = (double)y;
      x2d = (double)x2;
      y2d = (double)y2;
      len = sqrt((x1d - x2d) * (x1d - x2d) + (y1d - y2d) * (y1d - y2d));
      if (len == 0.0) {
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
          blend_pixel(ctx->frame, y * WIDTH + x, r, g, b, a);
        return 0;
      }
      if (fabs(x2d - x1d) < 20.0 && y1d != y2d) {
        if (y1d > y2d) { ad = y1d; y1d = y2d; y2d = ad; ad = x1d; x1d = x2d; x2d = ad; }
        ad = (x1d - x2d) / (y1d - y2d);
        bd = x1d - ad * y1d;
        dd = (y2d - y1d) / len / 2.0;
        if (dd <= 0.0) dd = 0.5;
        for (steps = 0, yd = y1d; yd <= y2d && steps <= 1000; yd += dd, steps++) {
          xd = ad * yd + bd;
          x = (int)xd;
          y = (int)yd;
          if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
            blend_pixel(ctx->frame, y * WIDTH + x, r, g, b, a);
        }
      }
      else {
        if (x1d > x2d) { ad = x1d; x1d = x2d; x2d = ad; ad = y1d; y1d = y2d; y2d = ad; }
        if (x1d == x2d) return 0;
        ad = (y1d - y2d) / (x1d - x2d);
        bd = y1d - ad * x1d;
        dd = (x2d - x1d) / len / 2.0;
        if (dd <= 0.0) dd = 0.5;
        for (steps = 0, xd = x1d; xd <= x2d && steps <= 1000; xd += dd, steps++) {
          yd = ad * xd + bd;
          x = (int)xd;
          y = (int)yd;
          if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
            blend_pixel(ctx->frame, y * WIDTH + x, r, g, b, a);
        }
      }
      return 0;

    case RENDER_PIXEL:
      if (render_arg(cmd, 0, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 2, vars, color1, sizeof(color1)) != 0 ||
          color_token(ctx, color1, &r, &g, &b, &a) != 0) return -1;
      if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        blend_pixel(ctx->frame, y * WIDTH + x, r, g, b, a);
      return 0;

    case RENDER_IMAGE:
      if (render_arg(cmd, 0, vars, image_name, sizeof(image_name)) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || hex_byte(token, &a) != 0) return -1;
      if (load_farbfeld_image(image_name, image) != 0) return -1;
      for (i = 0; i < PIXELS; i++)
        blend_pixel(ctx->frame, i, image[i], image[PIXELS + i], image[PIXELS * 2 + i], a);
      return 0;

    case RENDER_COPY:
      if (render_arg(cmd, 0, vars, image_name, sizeof(image_name)) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || hex_byte(token, &a) != 0 ||
          render_arg(cmd, 2, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 3, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 4, vars, token, sizeof(token)) != 0 || token_int(token, &x2) != 0 ||
          render_arg(cmd, 5, vars, token, sizeof(token)) != 0 || token_int(token, &y2) != 0 ||
          render_arg(cmd, 6, vars, token, sizeof(token)) != 0 || token_int(token, &dstx) != 0 ||
          render_arg(cmd, 7, vars, token, sizeof(token)) != 0 || token_int(token, &dsty) != 0) return -1;
      if (load_farbfeld_image(image_name, image) != 0) return -1;
      if (x < 0) x = 0;
      if (y < 0) y = 0;
      if (x2 >= WIDTH) x2 = WIDTH - 1;
      if (y2 >= HEIGHT) y2 = HEIGHT - 1;
      for (j = y; j <= y2; j++) {
        for (i = x; i <= x2; i++) {
          dx = dstx + i - x;
          dy = dsty + j - y;
          if (dx < 0 || dx >= WIDTH || dy < 0 || dy >= HEIGHT) continue;
          pos = j * WIDTH + i;
          blend_pixel(ctx->frame, dy * WIDTH + dx,
                      image[pos], image[PIXELS + pos], image[PIXELS * 2 + pos], a);
        }
      }
      return 0;

    case RENDER_SAMPLE:
      if (render_arg(cmd, 0, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 2, vars, token, sizeof(token)) != 0 || token_var(token, &var) != 0) return -1;
      if (var < 0 || var >= 10 || x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return -1;
      pos = y * WIDTH + x;
      ctx->vars[var][0] = ctx->frame[pos];
      ctx->vars[var][1] = ctx->frame[PIXELS + pos];
      ctx->vars[var][2] = ctx->frame[PIXELS * 2 + pos];
      return 0;

    case RENDER_AVERAGE:
      if (render_arg(cmd, 0, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 2, vars, token, sizeof(token)) != 0 || token_int(token, &x2) != 0 ||
          render_arg(cmd, 3, vars, token, sizeof(token)) != 0 || token_int(token, &y2) != 0 ||
          render_arg(cmd, 4, vars, token, sizeof(token)) != 0 || token_var(token, &var) != 0) return -1;
      if (var < 0 || var >= 10) return -1;
      if (x2 < x) { i = x; x = x2; x2 = i; }
      if (y2 < y) { i = y; y = y2; y2 = i; }
      if (x < 0) x = 0;
      if (y < 0) y = 0;
      if (x2 >= WIDTH) x2 = WIDTH - 1;
      if (y2 >= HEIGHT) y2 = HEIGHT - 1;
      sr = 0; sg = 0; sb = 0; count = 0;
      for (j = y; j <= y2; j++) {
        for (i = x; i <= x2; i++) {
          pos = j * WIDTH + i;
          sr += ctx->frame[pos];
          sg += ctx->frame[PIXELS + pos];
          sb += ctx->frame[PIXELS * 2 + pos];
          count++;
        }
      }
      if (count == 0) return -1;
      ctx->vars[var][0] = (unsigned char)(sr / count);
      ctx->vars[var][1] = (unsigned char)(sg / count);
      ctx->vars[var][2] = (unsigned char)(sb / count);
      return 0;

    case RENDER_AVERAGELINE:
      if (render_arg(cmd, 0, vars, token, sizeof(token)) != 0 || token_int(token, &x) != 0 ||
          render_arg(cmd, 1, vars, token, sizeof(token)) != 0 || token_int(token, &y) != 0 ||
          render_arg(cmd, 2, vars, token, sizeof(token)) != 0 || token_int(token, &x2) != 0 ||
          render_arg(cmd, 3, vars, token, sizeof(token)) != 0 || token_int(token, &y2) != 0 ||
          render_arg(cmd, 4, vars, token, sizeof(token)) != 0 || token_var(token, &var) != 0) return -1;
      if (var < 0 || var >= 10) return -1;
      sr = 0; sg = 0; sb = 0; count = 0;
      x1d = (double)x;
      y1d = (double)y;
      x2d = (double)x2;
      y2d = (double)y2;
      len = sqrt((x1d - x2d) * (x1d - x2d) + (y1d - y2d) * (y1d - y2d));
      if (len == 0.0) {
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
          pos = y * WIDTH + x;
          sr = ctx->frame[pos];
          sg = ctx->frame[PIXELS + pos];
          sb = ctx->frame[PIXELS * 2 + pos];
          count = 1;
        }
      }
      else if (fabs(x2d - x1d) < 20.0 && y1d != y2d) {
        if (y1d > y2d) { ad = y1d; y1d = y2d; y2d = ad; ad = x1d; x1d = x2d; x2d = ad; }
        ad = (x1d - x2d) / (y1d - y2d);
        bd = x1d - ad * y1d;
        dd = (y2d - y1d) / len / 2.0;
        if (dd <= 0.0) dd = 0.5;
        for (steps = 0, yd = y1d; yd <= y2d && steps <= 1000; yd += dd, steps++) {
          xd = ad * yd + bd;
          x = (int)xd;
          y = (int)yd;
          if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) continue;
          pos = y * WIDTH + x;
          sr += ctx->frame[pos];
          sg += ctx->frame[PIXELS + pos];
          sb += ctx->frame[PIXELS * 2 + pos];
          count++;
        }
      }
      else {
        if (x1d > x2d) { ad = x1d; x1d = x2d; x2d = ad; ad = y1d; y1d = y2d; y2d = ad; }
        if (x1d != x2d) {
          ad = (y1d - y2d) / (x1d - x2d);
          bd = y1d - ad * x1d;
          dd = (x2d - x1d) / len / 2.0;
          if (dd <= 0.0) dd = 0.5;
          for (steps = 0, xd = x1d; xd <= x2d && steps <= 1000; xd += dd, steps++) {
            yd = ad * xd + bd;
            x = (int)xd;
            y = (int)yd;
            if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) continue;
            pos = y * WIDTH + x;
            sr += ctx->frame[pos];
            sg += ctx->frame[PIXELS + pos];
            sb += ctx->frame[PIXELS * 2 + pos];
            count++;
          }
        }
      }
      if (count == 0) return -1;
      ctx->vars[var][0] = (unsigned char)(sr / count);
      ctx->vars[var][1] = (unsigned char)(sg / count);
      ctx->vars[var][2] = (unsigned char)(sb / count);
      return 0;
  }
  return -1;
}

static int render_file(const char *name, unsigned char *frame) {
  struct renderContext ctx;
  struct renderCommand cmd;
  char path[256], line[512];
  FILE *fp;
  size_t len;

  if (!valid_name(name)) return -1;
  if (snprintf(path, sizeof(path), "%s/%s", PGR_DIR, name) >= (int)sizeof(path)) return -1;
  fp = fopen(path, "rt");
  if (fp == 0) return -1;

  render_begin(&ctx, frame);
  for (;;) {
    if (fgets(line, sizeof(line), fp) == 0) break;
    len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') line[--len] = 0;
    if (len > 0 && line[len - 1] == '\r') line[len - 1] = 0;
    if (render_parse_line(line, &cmd) != 0 || render_execute(&ctx, &cmd, 0) != 0) {
      fclose(fp);
      return -1;
    }
  }

  fclose(fp);
  return 0;
}

static int render_text(const char *text, size_t len, unsigned char *frame) {
  struct renderContext ctx;
  struct renderCommand cmd;
  char line[512];
  size_t pos, start, line_len;

  render_begin(&ctx, frame);
  pos = 0;

  for (;;) {
    if (pos >= len) break;
    start = pos;
    for (; pos < len && text[pos] != '\n'; pos++) {
    }

    line_len = pos - start;
    if (line_len > 0 && text[start + line_len - 1] == '\r') line_len--;
    if (line_len >= sizeof(line)) return -1;

    memcpy(line, text + start, line_len);
    line[line_len] = 0;
    if (render_parse_line(line, &cmd) != 0 || render_execute(&ctx, &cmd, 0) != 0) return -1;

    if (pos < len && text[pos] == '\n') pos++;
  }

  return 0;
}

static void cgi_error(const char *status, const char *message) {
  printf("Status: %s\r\n", status);
  printf("Content-Type: text/plain; charset=utf-8\r\n");
  printf("Cache-Control: no-store\r\n\r\n");
  printf("%s\n", message);
}

static void cgi_editor(void) {
  static const char html[] =
    "<!doctype html>\n"
    "<html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
    "<title>displayd editor</title><style>"
    "body{font-family:system-ui,sans-serif;margin:20px;background:#111;color:#ddd}"
    ".main{display:flex;gap:20px;align-items:flex-start;flex-wrap:wrap}"
    "textarea{width:560px;height:420px;background:#181818;color:#eee;border:1px solid #555;padding:10px;font-family:monospace;font-size:14px}"
    "canvas{width:320px;height:320px;background:#000;image-rendering:pixelated;border:1px solid #555}"
    "button{margin-top:10px;padding:8px 18px;font-size:14px}#status{margin-top:8px;color:#aaa}"
    "</style></head><body><h1>displayd .des editor</h1><div class=\"main\"><div>"
    "<textarea id=\"des\" spellcheck=\"false\"></textarea><br><button id=\"render\">Render</button><div id=\"status\"></div>"
    "</div><canvas id=\"panel\" width=\"64\" height=\"64\"></canvas></div><script>"
    "const d=document.getElementById('des'),c=document.getElementById('panel'),x=c.getContext('2d'),s=document.getElementById('status');"
    "async function render(){s.textContent='Rendering...';try{const r=await fetch(location.pathname,{method:'POST',headers:{'Content-Type':'text/plain; charset=utf-8'},body:d.value,cache:'no-store'});"
    "if(!r.ok){s.textContent=await r.text();return}const b=new Uint8Array(await r.arrayBuffer());if(b.length!==12288){s.textContent='Invalid frame: '+b.length+' bytes';return}"
    "const im=x.createImageData(64,64);for(let i=0;i<4096;i++){im.data[i*4]=b[i];im.data[i*4+1]=b[4096+i];im.data[i*4+2]=b[8192+i];im.data[i*4+3]=255}x.putImageData(im,0,0);s.textContent='RGB888 preview';}catch(e){s.textContent=String(e)}}"
    "document.getElementById('render').onclick=render;d.addEventListener('keydown',e=>{if((e.ctrlKey||e.metaKey)&&e.key==='Enter')render()});"
    "</script></body></html>\n";

  printf("Content-Type: text/html; charset=utf-8\r\n");
  printf("Cache-Control: no-store\r\n");
  printf("Content-Length: %lu\r\n\r\n", (unsigned long)(sizeof(html) - 1));
  fwrite(html, 1, sizeof(html) - 1, stdout);
}

static int cgi_main(void) {
  const char *method, *length_text;
  unsigned char frame[FRAME_LEN];
  char *body, *endp;
  unsigned long content_length;
  size_t got, r;

  method = getenv("REQUEST_METHOD");
  if (method == 0 || strcmp(method, "GET") == 0) {
    cgi_editor();
    return 0;
  }

  if (strcmp(method, "POST") != 0) {
    cgi_error("405 Method Not Allowed", "Only GET and POST are supported.");
    return 0;
  }

  length_text = getenv("CONTENT_LENGTH");
  if (length_text == 0 || *length_text == 0) {
    cgi_error("411 Length Required", "Missing CONTENT_LENGTH.");
    return 0;
  }

  errno = 0;
  endp = 0;
  content_length = strtoul(length_text, &endp, 10);
  if (errno != 0 || endp == length_text || *endp != 0) {
    cgi_error("400 Bad Request", "Invalid CONTENT_LENGTH.");
    return 0;
  }
  if (content_length > CGI_MAX_BODY) {
    cgi_error("413 Content Too Large", "The .des document is too large.");
    return 0;
  }

  body = (char *)malloc((size_t)content_length + 1U);
  if (body == 0) {
    cgi_error("500 Internal Server Error", "Memory allocation failed.");
    return 0;
  }

  got = 0;
  for (; got < (size_t)content_length; ) {
    r = fread(body + got, 1, (size_t)content_length - got, stdin);
    if (r == 0) break;
    got += r;
  }
  if (got != (size_t)content_length) {
    free(body);
    cgi_error("400 Bad Request", "Incomplete request body.");
    return 0;
  }
  body[content_length] = 0;

  if (render_text(body, (size_t)content_length, frame) != 0) {
    free(body);
    cgi_error("400 Bad Request", "Render error in .des document.");
    return 0;
  }
  free(body);

  printf("Content-Type: application/octet-stream\r\n");
  printf("Cache-Control: no-store\r\n");
  printf("Content-Length: %d\r\n\r\n", FRAME_LEN);
  fwrite(frame, 1, FRAME_LEN, stdout);
  return 0;
}

static int valid_name(const char *name) {
  const unsigned char *p;

  if (name[0] == 0 || strstr(name, "..") != 0) return 0;
  for (p = (const unsigned char *)name; *p != 0; p++) {
    if (!isalnum(*p) && *p != '_' && *p != '-' && *p != '.') return 0;
  }
  return 1;
}

static int program_int_format(const char *fmt) {
  const char *p;

  if (fmt == 0 || *fmt++ != '%') return 0;
  p = fmt;
  if (*p == '0') p++;
  for (; isdigit((unsigned char)*p); p++) {
  }
  if (*p++ != 'd') return 0;
  return *p == 0;
}

static int program_ulong(const char *text, unsigned long *value) {
  char *endp;
  unsigned long n;

  errno = 0;
  endp = 0;
  n = strtoul(text, &endp, 10);
  if (errno != 0 || endp == text || *endp != 0) return -1;
  *value = n;
  return 0;
}

static int program_var(const char *text, int *value) {
  const char *p;

  p = text;
  if (*p == '@') p++;
  if (token_int(p, value) != 0 || *value < 0 || *value >= 30) return -1;
  return 0;
}

static int program_add_block(struct program *program, int type, int last_step,
                             unsigned long video_base) {
  struct programBlock *block;
  int first_step;

  if (program->block_count >= MAX_PROGRAM_BLOCKS || last_step < 0 || last_step >= MAX_STEPS) return -1;
  first_step = program->block_count == 0 ? 0 : program->block[program->block_count - 1].last_step + 1;
  if (last_step < first_step) return -1;

  block = &program->block[program->block_count++];
  memset(block, 0, sizeof(*block));
  block->type = type;
  block->first_step = first_step;
  block->last_step = last_step;
  block->interval_ms = type == PROGRAM_VIDEO ? 40 : 1000;
  block->video_base = video_base;
  block->first_op = program->op_count;
  program->total_steps = last_step + 1;
  return program->block_count - 1;
}

static int program_add_instruction(struct program *program, int block_index,
                                   struct programInstruction **instruction) {
  struct programBlock *block;

  if (block_index < 0 || block_index >= program->block_count ||
      program->block[block_index].type != PROGRAM_FRAME ||
      program->op_count >= MAX_PROGRAM_OPS) return -1;
  block = &program->block[block_index];
  *instruction = &program->op[program->op_count++];
  memset(*instruction, 0, sizeof(**instruction));
  block->op_count++;
  return 0;
}

static int program_load(const char *name, struct program *program) {
  struct programInstruction *instruction;
  struct rpnToken *rpn;
  char path[256], line[512], work[512], keyword[32], token[RENDER_TOKEN_LEN], extra[8];
  char *p, *q;
  FILE *fp;
  size_t len;
  unsigned long video_base;
  int line_number, block_index, last_step, value, min, max, var;

  if (!valid_name(name) || program == 0) return -1;
  if (snprintf(path, sizeof(path), "%s/%s.seq", PGR_DIR, name) >= (int)sizeof(path)) return -1;
  fp = fopen(path, "rt");
  if (fp == 0) return -1;

  memset(program, 0, sizeof(*program));
  block_index = -1;
  line_number = 0;
  for (;;) {
    if (fgets(line, sizeof(line), fp) == 0) break;
    line_number++;
    len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') line[--len] = 0;
    if (len > 0 && line[len - 1] == '\r') line[--len] = 0;
    p = line;
    for (; *p == ' ' || *p == '\t'; p++) {
    }
    if (*p == 0 || *p == '#') continue;
    if (strlen(p) >= sizeof(work)) goto error;
    strcpy(work, p);
    q = work;
    if (!next_token(&q, keyword, sizeof(keyword))) continue;
    keyword_upper(keyword);

    if (strcmp(keyword, "UNTIL") == 0) {
      if (!next_token(&q, token, sizeof(token)) || token_int(token, &last_step) != 0 ||
          next_token(&q, extra, sizeof(extra))) goto error;
      block_index = program_add_block(program, PROGRAM_FRAME, last_step, 0);
      if (block_index < 0) goto error;
      continue;
    }

    if (strcmp(keyword, "VIDEO") == 0) {
      if (!next_token(&q, token, sizeof(token)) || token_int(token, &last_step) != 0 ||
          !next_token(&q, token, sizeof(token)) || program_ulong(token, &video_base) != 0 ||
          next_token(&q, extra, sizeof(extra))) goto error;
      block_index = program_add_block(program, PROGRAM_VIDEO, last_step, video_base);
      if (block_index < 0) goto error;
      block_index = -1;
      continue;
    }

    if (block_index < 0 || block_index >= program->block_count ||
        program->block[block_index].type != PROGRAM_FRAME) goto error;

    if (strcmp(keyword, "WAIT") == 0) {
      if (!next_token(&q, token, sizeof(token)) || token_int(token, &value) != 0 || value <= 0 ||
          next_token(&q, extra, sizeof(extra))) goto error;
      program->block[block_index].interval_ms = value;
      continue;
    }

    if (strcmp(keyword, "RAND") == 0) {
      if (program_add_instruction(program, block_index, &instruction) != 0) goto error;
      instruction->type = PROGRAM_RAND;
      if (!next_token(&q, token, sizeof(token)) || program_var(token, &var) != 0) goto error;
      instruction->var = var;
      if (!next_token(&q, instruction->fmt, sizeof(instruction->fmt)) ||
          !program_int_format(instruction->fmt) ||
          !next_token(&q, token, sizeof(token)) || token_int(token, &min) != 0 ||
          !next_token(&q, token, sizeof(token)) || token_int(token, &max) != 0 || max < min ||
          next_token(&q, extra, sizeof(extra))) goto error;
      instruction->min = min;
      instruction->max = max;
      continue;
    }

    if (strcmp(keyword, "CALC") == 0) {
      if (program_add_instruction(program, block_index, &instruction) != 0) goto error;
      instruction->type = PROGRAM_CALC;
      if (!next_token(&q, token, sizeof(token)) || program_var(token, &var) != 0) goto error;
      instruction->var = var;
      if (!next_token(&q, instruction->fmt, sizeof(instruction->fmt)) ||
          !program_int_format(instruction->fmt)) goto error;
      for (;;) {
        if (!next_token(&q, token, sizeof(token))) break;
        if (instruction->rpn_count >= MAX_RPN_TOKENS) goto error;
        rpn = &instruction->rpn[instruction->rpn_count++];
        if (token[0] == '@') {
          if (program_var(token, &rpn->value) != 0) goto error;
          rpn->type = RPN_VARIABLE;
        }
        else if (strlen(token) == 1 && strchr("+-*/", token[0]) != 0) {
          rpn->type = RPN_OPERATOR;
          rpn->op = token[0];
        }
        else {
          if (token_int(token, &rpn->value) != 0) goto error;
          rpn->type = RPN_VALUE;
        }
      }
      if (instruction->rpn_count == 0) goto error;
      continue;
    }

    if (program_add_instruction(program, block_index, &instruction) != 0) goto error;
    instruction->type = PROGRAM_RENDER;
    if (render_parse_line(p, &instruction->render) != 0 || instruction->render.op == RENDER_NONE) goto error;
  }

  fclose(fp);
  if (program->block_count == 0 || program->total_steps <= 0) return -1;
  return 0;

error:
  fprintf(stderr, "Program parse error: %s.seq line %d: %s\n", name, line_number, line);
  fclose(fp);
  return -1;
}

static struct programBlock *program_block(struct program *program, int step) {
  int lo, hi, mid;

  lo = 0;
  hi = program->block_count - 1;
  for (; lo <= hi; ) {
    mid = lo + (hi - lo) / 2;
    if (step < program->block[mid].first_step) hi = mid - 1;
    else if (step > program->block[mid].last_step) lo = mid + 1;
    else return &program->block[mid];
  }
  return 0;
}

static int program_random(struct programInstruction *instruction, char vars[30][30],
                          unsigned int *seed) {
  unsigned int span;
  int value, n;

  span = (unsigned int)(instruction->max - instruction->min + 1);
  value = instruction->min + (int)(rand_r(seed) % span);
  n = snprintf(vars[instruction->var], sizeof(vars[instruction->var]), instruction->fmt, value);
  return n < 0 || n >= (int)sizeof(vars[instruction->var]) ? -1 : 0;
}

static int program_calc(struct programInstruction *instruction, char vars[30][30]) {
  int stack[RPN_STACK_MAX];
  struct rpnToken *token;
  int i, sp, a, b, n;

  sp = 0;
  for (i = 0; i < instruction->rpn_count; i++) {
    token = &instruction->rpn[i];
    if (token->type == RPN_VALUE || token->type == RPN_VARIABLE) {
      if (sp >= RPN_STACK_MAX) return -1;
      stack[sp++] = token->type == RPN_VALUE ? token->value : atoi(vars[token->value]);
      continue;
    }
    if (token->type != RPN_OPERATOR || sp < 2) return -1;
    b = stack[--sp];
    a = stack[--sp];
    switch (token->op) {
      case '+': a += b; break;
      case '-': a -= b; break;
      case '*': a *= b; break;
      case '/':
        if (b == 0) return -1;
        a /= b;
        break;
      default:
        return -1;
    }
    stack[sp++] = a;
  }
  if (sp != 1) return -1;
  n = snprintf(vars[instruction->var], sizeof(vars[instruction->var]), instruction->fmt, stack[0]);
  return n < 0 || n >= (int)sizeof(vars[instruction->var]) ? -1 : 0;
}

static int video_find_base_locked(unsigned long base) {
  int i;

  for (i = 0; i < MAX_VIDEOS; i++) {
    if (videos[i].used && videos[i].base == base) return i;
  }
  return -1;
}

static int video_overlaps_locked(unsigned long base, unsigned long frames, int skip) {
  unsigned long end, other_end;
  int i;

  if (frames == 0 || base > ULONG_MAX - frames) return 1;
  end = base + frames;

  for (i = 0; i < MAX_VIDEOS; i++) {
    if (i == skip || !videos[i].used) continue;
    if (videos[i].base > ULONG_MAX - videos[i].frames) return 1;
    other_end = videos[i].base + videos[i].frames;
    if (base < other_end && videos[i].base < end) return 1;
  }
  return 0;
}

static int video_load(unsigned long base, const char *name,
                      unsigned long *frames_out, size_t *bytes_out) {
  unsigned char raw[FRAME_LEN];
  unsigned char *data, *old_data, *dst;
  struct stat st;
  char path[256];
  FILE *fp;
  unsigned long frames, frame;
  size_t bytes;
  int i, slot, old_slot;

  if (!valid_name(name)) return -1;
  if (snprintf(path, sizeof(path), "%s/%s", VIDEO_DIR, name) >= (int)sizeof(path)) return -1;
  if (stat(path, &st) != 0 || st.st_size <= 0 || st.st_size % FRAME_LEN != 0) return -1;
  if ((unsigned long long)st.st_size > (unsigned long long)SIZE_MAX) return -1;

  bytes = (size_t)st.st_size;
  frames = (unsigned long)(bytes / FRAME_LEN);
  if (frames == 0 || base > ULONG_MAX - frames) return -1;

  data = (unsigned char *)malloc(bytes);
  if (data == 0) return -1;

  fp = fopen(path, "rb");
  if (fp == 0) {
    free(data);
    return -1;
  }

  for (frame = 0; frame < frames; frame++) {
    if (fread(raw, 1, FRAME_LEN, fp) != FRAME_LEN) {
      fclose(fp);
      free(data);
      return -1;
    }
    dst = data + (size_t)frame * FRAME_LEN;
    memcpy(dst, raw + PIXELS * 2, PIXELS);
    memcpy(dst + PIXELS, raw, PIXELS);
    memcpy(dst + PIXELS * 2, raw + PIXELS, PIXELS);
  }
  fclose(fp);

  pthread_rwlock_wrlock(&video_rwlock);
  old_slot = video_find_base_locked(base);
  if (video_overlaps_locked(base, frames, old_slot)) {
    pthread_rwlock_unlock(&video_rwlock);
    free(data);
    return -2;
  }

  slot = old_slot;
  if (slot < 0) {
    for (i = 0; i < MAX_VIDEOS; i++) {
      if (!videos[i].used) {
        slot = i;
        break;
      }
    }
  }
  if (slot < 0) {
    pthread_rwlock_unlock(&video_rwlock);
    free(data);
    return -3;
  }

  old_data = videos[slot].used ? videos[slot].data : 0;
  memset(&videos[slot], 0, sizeof(videos[slot]));
  videos[slot].used = 1;
  videos[slot].base = base;
  videos[slot].frames = frames;
  videos[slot].bytes = bytes;
  videos[slot].data = data;
  memcpy(videos[slot].name, name, strlen(name) + 1);
  pthread_rwlock_unlock(&video_rwlock);

  free(old_data);
  if (frames_out != 0) *frames_out = frames;
  if (bytes_out != 0) *bytes_out = bytes;
  return 0;
}

static int video_unload(unsigned long base) {
  unsigned char *data;
  int slot;

  pthread_rwlock_wrlock(&video_rwlock);
  slot = video_find_base_locked(base);
  if (slot < 0) {
    pthread_rwlock_unlock(&video_rwlock);
    return -1;
  }

  data = videos[slot].data;
  memset(&videos[slot], 0, sizeof(videos[slot]));
  pthread_rwlock_unlock(&video_rwlock);
  free(data);
  return 0;
}

static int video_frame_copy(unsigned long logical_frame, unsigned char *dst) {
  unsigned long offset;
  int i, found;

  found = 0;
  pthread_rwlock_rdlock(&video_rwlock);
  for (i = 0; i < MAX_VIDEOS; i++) {
    if (!videos[i].used) continue;
    if (logical_frame < videos[i].base ||
        logical_frame >= videos[i].base + videos[i].frames) continue;

    offset = logical_frame - videos[i].base;
    memcpy(dst, videos[i].data + (size_t)offset * FRAME_LEN, FRAME_LEN);
    found = 1;
    break;
  }
  pthread_rwlock_unlock(&video_rwlock);
  return found ? 0 : -1;
}

static void video_stats(int *count_out, size_t *bytes_out) {
  size_t bytes;
  int i, count;

  bytes = 0;
  count = 0;
  pthread_rwlock_rdlock(&video_rwlock);
  for (i = 0; i < MAX_VIDEOS; i++) {
    if (!videos[i].used) continue;
    count++;
    bytes += videos[i].bytes;
  }
  pthread_rwlock_unlock(&video_rwlock);

  *count_out = count;
  *bytes_out = bytes;
}

static int ensure_dir(const char *path) {
  if (mkdir(path, 0775) == 0 || errno == EEXIST) return 0;
  return -1;
}

static void cleanup_files(void) {
  unlink(control_socket);
  if (pid_fd >= 0) {
    unlink(PID_FILE);
    close(pid_fd);
    pid_fd = -1;
  }
}

static int lock_pid(void) {
  char buf[32];
  int len;

  pid_fd = open(PID_FILE, O_RDWR | O_CREAT, 0644);
  if (pid_fd < 0) return -1;
  if (flock(pid_fd, LOCK_EX | LOCK_NB) != 0) {
    close(pid_fd);
    pid_fd = -1;
    return -1;
  }

  if (ftruncate(pid_fd, 0) != 0) return -1;
  len = snprintf(buf, sizeof(buf), "%ld\n", (long)getpid());
  if (write(pid_fd, buf, (size_t)len) != len) return -1;
  return 0;
}

static int start_daemon(void) {
  int fd;

  if (!daemon_mode) return 0;
  if (daemon(1, 1) != 0) return -1;

  fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (fd < 0) return -1;
  if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0) {
    close(fd);
    return -1;
  }
  if (fd > STDERR_FILENO) close(fd);
  return 0;
}

static const char displayd_help[] =
  "displayd " DISPLAYD_VERSION "\n"
  "Usage:\n"
  "  ./displayd\n"
  "  ./displayd start [-p port] [-s socket]\n"
  "  ./displayd server [-p port] [-s socket]\n"
  "  ./displayd <command>\n"
  "\n"
  "Launcher:\n"
  "  start                         run as daemon\n"
  "  server                        run in foreground\n"
  "\n"
  "Commands:\n"
  "  status                        show server and client status\n"
  "  fonts                         show font cache\n"
  "  set <idx> <step>              set client step\n"
  "  clear <idx>                   disconnect client\n"
  "  video load <base> <file>      load video\n"
  "  video unload <base>           unload video\n"
  "  video list                    list loaded videos\n"
  "  render <des> <rgb>            render .des to RGB888 file\n"
  "  stop                          stop server\n"
  "  help                          show this help\n"
  "\n"
  "Options:\n"
  "  -p port                       RGB888 TCP port, default 5002\n"
  "  -s path                       local control socket\n";

static void usage(void) {
  fputs(displayd_help, stdout);
}

static int control_client(int argc, char **argv) {
  struct sockaddr_un addr;
  char cmd[512], buf[1024];
  size_t used, len;
  int fd, i, n;

  used = 0;
  cmd[0] = 0;
  for (i = 0; i < argc; i++) {
    len = strlen(argv[i]);
    if (used + len + (i != 0 ? 1U : 0U) + 2U > sizeof(cmd)) {
      fprintf(stderr, "Control command too long.\n");
      return 1;
    }
    if (i != 0) cmd[used++] = ' ';
    memcpy(cmd + used, argv[i], len);
    used += len;
  }
  cmd[used++] = '\n';
  cmd[used] = 0;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("control socket");
    return 1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  if (strlen(control_socket) >= sizeof(addr.sun_path)) {
    fprintf(stderr, "Control socket path too long.\n");
    close(fd);
    return 1;
  }
  memcpy(addr.sun_path, control_socket, strlen(control_socket) + 1);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    perror("control connect");
    close(fd);
    return 1;
  }

  if (send_all(fd, cmd, used) < 0) {
    perror("control send");
    close(fd);
    return 1;
  }

  for (;;) {
    n = (int)recv(fd, buf, sizeof(buf), 0);
    if (n > 0) {
      if (fwrite(buf, 1, (size_t)n, stdout) != (size_t)n) {
        close(fd);
        return 1;
      }
      continue;
    }
    if (n == 0) break;
    if (errno == EINTR) continue;
    perror("control recv");
    close(fd);
    return 1;
  }

  close(fd);
  return 0;
}

static void send_control_help(int fd) {
  send_all(fd, displayd_help, sizeof(displayd_help) - 1);
}

static int parse_ulong(const char *text, unsigned long *value) {
  char *endp;
  unsigned long n;

  if (text == 0) return -1;
  errno = 0;
  endp = 0;
  n = strtoul(text, &endp, 10);
  if (errno != 0 || endp == text || *endp != 0) return -1;
  *value = n;
  return 0;
}

static int video_load_startup(void) {
  char line[256], base_text[32], name[VIDEO_NAME_LEN], extra[8];
  char *p;
  FILE *fp;
  unsigned long base, frames;
  size_t bytes, len;
  int line_number, loaded, rc;

  fp = fopen(VIDEO_CONFIG, "rt");
  if (fp == 0) {
    if (errno == ENOENT) return 0;
    perror("video config");
    return -1;
  }

  line_number = 0;
  loaded = 0;
  for (;;) {
    if (fgets(line, sizeof(line), fp) == 0) break;
    line_number++;
    len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') line[--len] = 0;
    if (len > 0 && line[len - 1] == '\r') line[--len] = 0;

    p = line;
    for (; *p == ' ' || *p == '\t'; p++) {
    }
    if (*p == 0 || *p == '#') continue;

    if (!next_token(&p, base_text, sizeof(base_text)) ||
        !next_token(&p, name, sizeof(name)) ||
        parse_ulong(base_text, &base) != 0) {
      fprintf(stderr, "Invalid video config line %d: %s\n", line_number, line);
      continue;
    }

    for (; *p == ' ' || *p == '\t'; p++) {
    }
    if (*p != 0 && *p != '#') {
      if (!next_token(&p, extra, sizeof(extra))) extra[0] = 0;
      fprintf(stderr, "Invalid video config line %d: %s\n", line_number, line);
      continue;
    }

    rc = video_load(base, name, &frames, &bytes);
    if (rc != 0) {
      fprintf(stderr, "Cannot load startup video line %d: base=%lu file=%s error=%d\n",
              line_number, base, name, rc);
      continue;
    }

    printf("Startup video: base=%lu frames=%lu bytes=%lu file=%s\n",
           base, frames, (unsigned long)bytes, name);
    loaded++;
  }

  fclose(fp);
  return loaded;
}

static void *control_interface(void *arg) {
  unsigned char render_frame[FRAME_LEN];
  int server_fd, client_fd, n, i, len, target_idx, si, video_count;
  int video_result;
  struct sockaddr_un addr;
  struct timeval rcv_to, snd_to;
  char cmd_buf[512], resp[1024], aux[100], path[256];
  char *cmd, *arg1, *arg2, *arg3;
  time_t now;
  long depoch;
  unsigned long set_step, video_base, video_frames;
  size_t video_bytes, video_total_bytes;
  FILE *fp;

  (void)arg;

  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0) return 0;

  unlink(control_socket);
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  memcpy(addr.sun_path, control_socket, strlen(control_socket) + 1);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(server_fd);
    return 0;
  }

  chmod(control_socket, 0660);
  if (listen(server_fd, 8) < 0) {
    close(server_fd);
    unlink(control_socket);
    return 0;
  }

  rcv_to.tv_sec = CONTROL_RECV_TIMEOUT_SEC;
  rcv_to.tv_usec = 0;
  snd_to.tv_sec = CONTROL_SEND_TIMEOUT_SEC;
  snd_to.tv_usec = 0;

  for (;;) {
    client_fd = accept(server_fd, 0, 0);
    if (client_fd < 0) {
      if (errno == EINTR) continue;
      continue;
    }

    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_to, sizeof(rcv_to));
    setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &snd_to, sizeof(snd_to));

    memset(cmd_buf, 0, sizeof(cmd_buf));
    n = (int)recv(client_fd, cmd_buf, sizeof(cmd_buf) - 1, 0);
    if (n <= 0) {
      close(client_fd);
      continue;
    }

    cmd_buf[strcspn(cmd_buf, "\r\n")] = 0;
    cmd = strtok(cmd_buf, " \t");
    arg1 = strtok(0, " \t");
    arg2 = strtok(0, " \t");
    arg3 = strtok(0, " \t");

    if (cmd == 0 || strcmp(cmd, "help") == 0) {
      send_control_help(client_fd);
    }
    else if (strcmp(cmd, "status") == 0) {
      video_stats(&video_count, &video_total_bytes);
      len = snprintf(resp, sizeof(resp),
                     "displayd " DISPLAYD_VERSION " RGB888 port=%d frame=%d videos=%d video_bytes=%lu startup=%ld\n",
                     listen_port, FRAME_LEN, video_count,
                     (unsigned long)video_total_bytes, (long)server_startup_time);
      send_all(client_fd, resp, (size_t)len);

      now = time(0);
      pthread_mutex_lock(&mon_mutex);
      for (i = 0; i < MAX_THREADS; i++) {
        if (!mythr[i].active) continue;

        if (mythr[i].mir == -1) snprintf(aux, sizeof(aux), "%lu", mythr[i].step);
        else if (mythr[i].mir >= 0 && mythr[i].mir < MAX_THREADS)
          snprintf(aux, sizeof(aux), "mirror:%s", mythr[mythr[i].mir].ser);
        else strcpy(aux, "mirror:invalid");

        depoch = elapsed_since_epoch(now, mythr[i].epoch);
        len = snprintf(resp, sizeof(resp),
                       "%03d %12s %-15s:%-5u %s depoch=%ld rssi=%d\n",
                       i, mythr[i].ser, mythr[i].ip, (unsigned)mythr[i].port,
                       aux, depoch, (int)mythr[i].rssi);
        send_all(client_fd, resp, (size_t)len);
      }
      pthread_mutex_unlock(&mon_mutex);
    }
    else if (strcmp(cmd, "fonts") == 0) {
      font_cache_report(client_fd);
    }
    else if (strcmp(cmd, "clear") == 0 && arg1 != 0) {
      target_idx = atoi(arg1);
      pthread_mutex_lock(&mon_mutex);
      if (target_idx >= 0 && target_idx < MAX_THREADS && mythr[target_idx].active) {
        shutdown_session_locked(target_idx, "local-clear");
        len = snprintf(resp, sizeof(resp), "OK clear %d\n", target_idx);
      }
      else len = snprintf(resp, sizeof(resp), "ERR invalid or inactive index\n");
      pthread_mutex_unlock(&mon_mutex);
      send_all(client_fd, resp, (size_t)len);
    }
    else if (strcmp(cmd, "set") == 0 && arg1 != 0 && arg2 != 0) {
      target_idx = atoi(arg1);
      if (parse_ulong(arg2, &set_step) != 0) {
        send_all(client_fd, "ERR invalid step\n", 17);
      }
      else {
        pthread_mutex_lock(&mon_mutex);
        if (target_idx < 0 || target_idx >= MAX_THREADS || !mythr[target_idx].active) {
          len = snprintf(resp, sizeof(resp), "ERR invalid or inactive index\n");
        }
        else if (mythr[target_idx].mir != -1) {
          len = snprintf(resp, sizeof(resp), "ERR mirror session\n");
        }
        else {
          mythr[target_idx].step = set_step;
          mythr[target_idx].force_step = set_step;
          mythr[target_idx].force_step_pending = 1;
          si = state_get_or_create_locked(mythr[target_idx].ser);
          if (si >= 0) mystate[si].step = set_step;
          len = snprintf(resp, sizeof(resp), "OK set %d %lu\n", target_idx, set_step);
        }
        pthread_mutex_unlock(&mon_mutex);
        send_all(client_fd, resp, (size_t)len);
      }
    }
    else if (strcmp(cmd, "video") == 0 && arg1 != 0 && strcmp(arg1, "load") == 0 &&
             arg2 != 0 && arg3 != 0) {
      if (parse_ulong(arg2, &video_base) != 0) {
        send_all(client_fd, "ERR invalid base\n", 17);
      }
      else {
        video_result = video_load(video_base, arg3, &video_frames, &video_bytes);
        if (video_result == 0) {
          len = snprintf(resp, sizeof(resp),
                         "OK video base=%lu frames=%lu bytes=%lu file=%s\n",
                         video_base, video_frames, (unsigned long)video_bytes, arg3);
        }
        else if (video_result == -2) {
          len = snprintf(resp, sizeof(resp), "ERR video range overlaps an existing video\n");
        }
        else if (video_result == -3) {
          len = snprintf(resp, sizeof(resp), "ERR video table full\n");
        }
        else {
          len = snprintf(resp, sizeof(resp), "ERR cannot load video\n");
        }
        send_all(client_fd, resp, (size_t)len);
      }
    }
    else if (strcmp(cmd, "video") == 0 && arg1 != 0 && strcmp(arg1, "unload") == 0 &&
             arg2 != 0) {
      if (parse_ulong(arg2, &video_base) != 0 || video_unload(video_base) != 0) {
        send_all(client_fd, "ERR video not found\n", 20);
      }
      else {
        len = snprintf(resp, sizeof(resp), "OK video unloaded base=%lu\n", video_base);
        send_all(client_fd, resp, (size_t)len);
      }
    }
    else if (strcmp(cmd, "video") == 0 && arg1 != 0 && strcmp(arg1, "list") == 0) {
      pthread_rwlock_rdlock(&video_rwlock);
      for (i = 0; i < MAX_VIDEOS; i++) {
        if (!videos[i].used) continue;
        len = snprintf(resp, sizeof(resp),
                       "base=%lu frames=%lu end=%lu bytes=%lu file=%s\n",
                       videos[i].base, videos[i].frames,
                       videos[i].base + videos[i].frames - 1,
                       (unsigned long)videos[i].bytes, videos[i].name);
        send_all(client_fd, resp, (size_t)len);
      }
      pthread_rwlock_unlock(&video_rwlock);
    }
    else if (strcmp(cmd, "render") == 0 && arg1 != 0 && arg2 != 0) {
      if (!valid_name(arg2) || render_file(arg1, render_frame) != 0 ||
          snprintf(path, sizeof(path), "%s/%s", BASE_DIR, arg2) >= (int)sizeof(path)) {
        send_all(client_fd, "ERR render\n", 11);
      }
      else {
        fp = fopen(path, "wb");
        if (fp == 0 || fwrite(render_frame, 1, FRAME_LEN, fp) != FRAME_LEN) {
          if (fp != 0) fclose(fp);
          send_all(client_fd, "ERR render output\n", 18);
        }
        else {
          fclose(fp);
          len = snprintf(resp, sizeof(resp), "OK render bytes=%d file=%s\n", FRAME_LEN, arg2);
          send_all(client_fd, resp, (size_t)len);
        }
      }
    }
    else if (strcmp(cmd, "stop") == 0) {
      send_all(client_fd, "OK stopping\n", 12);
      close(client_fd);
      exit(0);
    }
    else {
      send_all(client_fd, "ERR command\n", 12);
    }

    close(client_fd);
  }

  return 0;
}

static void *client(void *p) {
  int fd, one, r, sent, my_idx, mir_idx, state_idx, ln, op_index;
  unsigned char *buf;
  char v[30][30], aux[256], peer_ip[16];
  unsigned char mir_buf[FRAME_LEN], dynamic_frame[FRAME_LEN], video_frame[FRAME_LEN];
  struct renderContext render_ctx;
  struct program program;
  struct programBlock *block;
  struct programInstruction *instruction;
  unsigned char ip0, ip1, ip2, ip3;
  unsigned long t, now, step, resume_step, mir_last, logical_frame;
  struct timeval tv;
  FILE *fp;
  struct timespec ts;
  struct tm tmv;
  unsigned long my_gen;
  unsigned int seed;
  time_t my_epoch;
  long depoch;
  signed char rssi;
  unsigned short peer_port;
  struct sockaddr_in peer;
  socklen_t peer_len;
  const char *end_reason;

  fd = *(int *)p;
  free(p);

  my_idx = -1;
  my_gen = 0;
  my_epoch = 0;
  depoch = 0;
  peer_port = 0;
  memset(v, 0, sizeof(v));
  memset(&program, 0, sizeof(program));
  state_idx = -1;
  resume_step = 0;
  peer_len = sizeof(peer);
  strcpy(peer_ip, "0.0.0.0");
  end_reason = "unknown";

  one = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

  if (getpeername(fd, (struct sockaddr *)&peer, &peer_len) == 0) {
    inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
    peer_port = (unsigned short)ntohs(peer.sin_port);
  }

  /*
   * Initial 16-byte client header:
   *   bytes 0..11  : serial
   *   bytes 12..15 : client-side IP bytes as reported by the device
   */
  if (read_full(fd, aux, 16) < 0) {
    close(fd);
    return 0;
  }

  memcpy(v[0], aux, 12);
  v[0][12] = 0;
  if (!valid_serial(v[0])) {
    close(fd);
    return 0;
  }

  ip0 = (unsigned char)aux[12];
  ip1 = (unsigned char)aux[13];
  ip2 = (unsigned char)aux[14];
  ip3 = (unsigned char)aux[15];

  snprintf(v[1], sizeof(v[1]), "%u.%u.%u.%u",
          (unsigned)ip0,
          (unsigned)ip1,
          (unsigned)ip2,
          (unsigned)ip3);

  /*
   * Register this connection in the monitor.
   *
   * A serial can have only one active physical session.
   * The newest connection wins.
   *
   * step is owned by the serial, not by the socket.
   * epoch is the Unix timestamp of the latest relaunch/reconnection.
   */
  pthread_mutex_lock(&mon_mutex);

  state_idx = state_get_or_create_locked(v[0]);
  if (state_idx >= 0) {
    resume_step = mystate[state_idx].step;
    my_epoch = time(NULL);
    mystate[state_idx].epoch = my_epoch;
  }
  else {
    resume_step = 0;
    my_epoch = time(NULL);
  }

  for (r = 0; r < MAX_THREADS; r++) {
    if (mythr[r].active && strncmp(mythr[r].ser, v[0], 12) == 0) {
      if (mythr[r].step > resume_step) {
        resume_step = mythr[r].step;
      }

      shutdown_session_locked(r, "serial-reconnect");
    }
  }

  if (state_idx >= 0 && resume_step > mystate[state_idx].step) {
    mystate[state_idx].step = resume_step;
  }

  my_gen = ++session_gen;

  for (r = 0; r < MAX_THREADS; r++) {
    if (!mythr[r].active) {
      mythr[r].active = 1;
      mythr[r].fd = fd;
      mythr[r].gen = my_gen;

      memcpy(mythr[r].ser, v[0], 13);

      /*
       * Keep the device-reported IP in the monitor, as in the original code.
       * peer_ip remains available for logs if needed.
       */
      strcpy(mythr[r].ip, v[1]);

      mythr[r].port = peer_port;
      mythr[r].step = resume_step;
      mythr[r].force_step_pending = 0;
      mythr[r].force_step = 0;
      mythr[r].epoch = my_epoch;
      mythr[r].rssi = 0;
      mythr[r].mir = -1;
      mythr[r].t = 0;

      my_idx = r;

      printf("New session: idx=%d serial=%s device_ip=%s peer=%s:%u step=%lu depoch=0 gen=%lu\n",
             my_idx,
             mythr[r].ser,
             mythr[r].ip,
             peer_ip,
             (unsigned)peer_port,
             mythr[r].step,
             my_gen);
      fflush(stdout);

      break;
    }
  }

  pthread_mutex_unlock(&mon_mutex);

  if (my_idx == -1) {
    printf("Too many sessions: rejecting serial=%s peer=%s:%u\n",
           v[0],
           peer_ip,
           (unsigned)peer_port);
    fflush(stdout);
    close(fd);
    return 0;
  }

  snprintf(aux, sizeof(aux), "%s/%s.mat", PGR_DIR, v[0]);
  fp = fopen(aux, "rt");
  if (fp == 0) {
    end_reason = "mat-open";
    goto cleanup;
  }

  if (fgets(v[3], (int)sizeof(v[3]), fp) == 0) {
    fclose(fp);
    end_reason = "mat-read";
    goto cleanup;
  }

  fclose(fp);

  r = (int)strlen(v[3]);
  if (r > 0 && v[3][r - 1] == '\n') v[3][r - 1] = '\0';

  /*
   * Mirroring mode.
   * If the .mat file contains a 12-char serial, this display mirrors another
   * display. The mirror target is still resolved by index, as in the original
   * implementation. If the target reconnects and moves to a new slot, this
   * mirror session will exit and the device can reconnect.
   */
  if (valid_serial(v[3])) {
    mir_last = ULONG_MAX;

    pthread_mutex_lock(&mon_mutex);

    for (mir_idx = 0; mir_idx < MAX_THREADS; mir_idx++) {
      if (mythr[mir_idx].active && strncmp(mythr[mir_idx].ser, v[3], 12) == 0) {
        break;
      }
    }

    if (mir_idx < MAX_THREADS && session_is_current_locked(my_idx, my_gen)) {
      mythr[my_idx].mir = mir_idx;
    }

    pthread_mutex_unlock(&mon_mutex);

    if (mir_idx == MAX_THREADS) {
      end_reason = "mirror-target-unavailable";
      goto cleanup;
    }

    for (;;) {
      pthread_mutex_lock(&mon_mutex);

      if (!session_is_current_locked(my_idx, my_gen)) {
        pthread_mutex_unlock(&mon_mutex);
        end_reason = "session-replaced";
        goto cleanup;
      }

      if (!mythr[mir_idx].active || strncmp(mythr[mir_idx].ser, v[3], 12) != 0) {
        pthread_mutex_unlock(&mon_mutex);
        end_reason = "mirror-target-ended";
        goto cleanup;
      }

      t = mythr[mir_idx].t;
      if (t != 0 && t != mir_last) {
        memcpy(mir_buf, mythr[mir_idx].bin, FRAME_LEN);
      }

      pthread_mutex_unlock(&mon_mutex);

      if (t == 0 || t == mir_last) {
        usleep(1000);
        continue;
      }

      if (send_all(fd, mir_buf, FRAME_LEN) < 0) {
        end_reason = "mirror-send";
        goto cleanup;
      }

      for (;;) {
        r = (int)recv(fd, &rssi, 1, MSG_DONTWAIT);
        if (r != 1) break;
        update_rssi_if_current(my_idx, my_gen, rssi);
      }

      mir_last = t;
    }
  }

  if (program_load(v[3], &program) != 0) {
    end_reason = "program-load";
    goto cleanup;
  }

  gettimeofday(&tv, 0);
  t = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;

  clock_gettime(CLOCK_REALTIME, &ts);
  seed = (unsigned int)((unsigned long)ts.tv_sec ^ (unsigned long)ts.tv_nsec ^ (unsigned long)getpid() ^ my_gen);

  for (step = resume_step;;) {
    take_forced_step_if_current(my_idx, my_gen, &step);
    update_step_if_current(my_idx, my_gen, step);

    pthread_mutex_lock(&mon_mutex);
    if (!session_is_current_locked(my_idx, my_gen)) {
      pthread_mutex_unlock(&mon_mutex);
      end_reason = "session-replaced";
      goto cleanup;
    }
    pthread_mutex_unlock(&mon_mutex);

    gettimeofday(&tv, 0);
    now = tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;
    if (now < t) {
      usleep(1000);
      continue;
    }

    if (program.total_steps <= 0) {
      end_reason = "program-no-steps";
      goto cleanup;
    }
    ln = (int)(step % (unsigned long)program.total_steps);
    block = program_block(&program, ln);
    if (block == 0) {
      end_reason = "program-step-not-found";
      goto cleanup;
    }
    buf = 0;

    if (block->type == PROGRAM_VIDEO) {
      logical_frame = block->video_base + (unsigned long)(ln - block->first_step);
      if (video_frame_copy(logical_frame, video_frame) != 0) memset(video_frame, 0, FRAME_LEN);
      buf = video_frame;
    }
    else if (block->type == PROGRAM_FRAME) {
      render_begin(&render_ctx, dynamic_frame);
      clock_gettime(CLOCK_REALTIME, &ts);
      localtime_r(&ts.tv_sec, &tmv);

      snprintf(v[4], sizeof(v[4]), "%02d", tmv.tm_hour);
      snprintf(v[5], sizeof(v[5]), "%02d", tmv.tm_min);
      snprintf(v[6], sizeof(v[6]), "%02d", tmv.tm_sec);
      snprintf(v[2], sizeof(v[2]), "%lu", step);
      depoch = elapsed_since_epoch(ts.tv_sec, my_epoch);
      snprintf(v[7], sizeof(v[7]), "%ld", depoch);

      for (op_index = block->first_op;
           op_index < block->first_op + block->op_count;
           op_index++) {
        instruction = &program.op[op_index];
        if (instruction->type == PROGRAM_RAND) {
          if (program_random(instruction, v, &seed) != 0) {
            end_reason = "program-rand";
            goto cleanup;
          }
        }
        else if (instruction->type == PROGRAM_CALC) {
          if (program_calc(instruction, v) != 0) {
            end_reason = "program-calc";
            goto cleanup;
          }
        }
        else if (instruction->type == PROGRAM_RENDER) {
          if (render_execute(&render_ctx, &instruction->render, v) != 0) {
            end_reason = "render";
            goto cleanup;
          }
        }
        else {
          end_reason = "program-invalid-instruction";
          goto cleanup;
        }
      }
      buf = dynamic_frame;
    }

    if (buf == 0) {
      end_reason = "program-invalid-block";
      goto cleanup;
    }
    sent = send_all(fd, buf, FRAME_LEN);
    if (sent < 0) {
      end_reason = "send-frame";
      goto cleanup;
    }
    update_frame_if_current(my_idx, my_gen, buf, now);

    for (;;) {
      r = (int)recv(fd, &rssi, 1, MSG_DONTWAIT);
      if (r != 1) break;
      update_rssi_if_current(my_idx, my_gen, rssi);
    }

    step++;
    t += (unsigned long)block->interval_ms;
  }

cleanup:
  /*
   * Only the owner of the current generation may clear the slot.
   * If this thread was replaced by a newer connection, its generation
   * no longer matches and the newer slot is left untouched.
   *
   * The logical per-serial state in mystate[] is intentionally not cleared.
   * It keeps step and epoch for the next reconnection.
   */
  if (my_idx != -1) {
    pthread_mutex_lock(&mon_mutex);

    if (session_is_current_locked(my_idx, my_gen)) {
      mythr[my_idx].active = 0;
      mythr[my_idx].fd = -1;
      mythr[my_idx].force_step_pending = 0;
      mythr[my_idx].force_step = 0;
      mythr[my_idx].mir = -1;
      mythr[my_idx].t = 0;

      depoch = elapsed_since_epoch(time(NULL), mythr[my_idx].epoch);

      printf("Session ended: idx=%d serial=%s step=%lu depoch=%ld gen=%lu reason=%s program=%s\n",
             my_idx,
             mythr[my_idx].ser,
             mythr[my_idx].step,
             depoch,
             my_gen,
             end_reason,
             v[3][0] != 0 ? v[3] : "-");
      fflush(stdout);
    }

    pthread_mutex_unlock(&mon_mutex);
  }

  close(fd);
  return 0;
}

int main(int argc, char **argv) {
  int server_fd, client_fd, *p_fd;

  if (getenv("GATEWAY_INTERFACE") != 0 || getenv("REQUEST_METHOD") != 0) return cgi_main();
  int opt, i, c;
  long port;
  char *endp;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len;
  pthread_t tid, control_tid;

  for (;;) {
    c = getopt(argc, argv, "p:s:h");
    if (c == -1) break;

    switch (c) {
    case 'p':
      errno = 0;
      endp = 0;
      port = strtol(optarg, &endp, 10);
      if (errno != 0 || endp == optarg || *endp != 0 || port < 1 || port > 65535) {
        usage();
        return 1;
      }
      listen_port = (int)port;
      break;

    case 's':
      if (strlen(optarg) >= sizeof(control_socket)) {
        fprintf(stderr, "Control socket path too long.\n");
        return 1;
      }
      strcpy(control_socket, optarg);
      break;

    default:
      usage();
      return c == 'h' ? 0 : 1;
    }
  }

  if (optind >= argc) {
    usage();
    return 0;
  }

  if (strcmp(argv[optind], "help") == 0 && optind + 1 == argc) {
    usage();
    return 0;
  }

  if (strcmp(argv[optind], "start") == 0 || strcmp(argv[optind], "server") == 0) {
    if (optind + 1 != argc) {
      usage();
      return 1;
    }
    daemon_mode = strcmp(argv[optind], "start") == 0;
  }
  else {
    return control_client(argc - optind, argv + optind);
  }

  if (ensure_dir(VIDEO_DIR) != 0 || ensure_dir(IMAGE_DIR) != 0 || ensure_dir(PGR_DIR) != 0) {
    perror("mkdir");
    return 1;
  }

  if (start_daemon() != 0) {
    perror("daemon");
    return 1;
  }

  if (lock_pid() != 0) {
    fprintf(stderr, "displayd already running or pid file unavailable.\n");
    return 1;
  }
  atexit(cleanup_files);

  signal(SIGPIPE, SIG_IGN);
  server_startup_time = time(0);

  pthread_mutex_lock(&mon_mutex);
  memset(mythr, 0, sizeof(mythr));
  memset(mystate, 0, sizeof(mystate));
  for (i = 0; i < MAX_THREADS; i++) {
    mythr[i].fd = -1;
    mythr[i].mir = -1;
  }
  pthread_mutex_unlock(&mon_mutex);
  memset(videos, 0, sizeof(videos));
  video_load_startup();

  if (pthread_create(&control_tid, 0, control_interface, 0) != 0) {
    perror("control thread");
    return 1;
  }
  pthread_detach(control_tid);

  opt = 1;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return 1;
  }

  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons((unsigned short)listen_port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, 32) < 0) {
    perror("listen");
    close(server_fd);
    return 1;
  }

  printf("displayd " DISPLAYD_VERSION ": RGB888 port=%d frame=%d control=%s\n",
         listen_port, FRAME_LEN, control_socket);
  fflush(stdout);

  for (;;) {
    addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
      if (errno == EINTR) continue;
      perror("accept");
      continue;
    }

    p_fd = (int *)malloc(sizeof(int));
    if (p_fd == 0) {
      close(client_fd);
      continue;
    }
    *p_fd = client_fd;

    if (pthread_create(&tid, 0, client, p_fd) != 0) {
      close(client_fd);
      free(p_fd);
      continue;
    }
    pthread_detach(tid);
  }

  return 0;
}
