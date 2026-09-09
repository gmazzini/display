# display

Network-controlled 64x64 HUB75 display system.

## Current components

- `device.c` - ESP32-S3 firmware, Version 4.50, using ESP-IDF.
- `displayd.c` - RGB888 server, renderer, scheduler and CGI editor, Version 1.25.
- `virtpanel.c` - SDL2 virtual panel, Version 2.02.
- `Makefile` - firmware, server and virtual-panel build targets.
- `font/` - TrueType/OpenType fonts used directly by `displayd`.
- `tmpdata/images/` - 64x64 farbfeld images.
- `tmpdata/video/` - video data.
- `video.conf` - videos loaded automatically when `displayd` starts.
- `tmpdata/pgr/` - `.seq` and `.mat` programming files.

The physical device and the virtual panel connect to `displayd.mazzini.org:5002`.
The client sends a 16-byte header containing the 12-character serial and its local IPv4 address. The server sends planar RGB888 frames as `R[4096] G[4096] B[4096]`, 12288 bytes per frame, and the client returns one RSSI byte after each frame.

The complete display path is RGB888: rendering, network transport, physical panel output, browser preview and virtual panel all use 8 bits per channel (24 bits per pixel). The physical firmware drives the HUB75 panel with binary color bit-planes; the compile-time `COLOR_BITS` value can still be set to any value from 1 through 8. The production setting is 8. Lower values use the most significant bits of each RGB888 channel.

## Device firmware

`device.c` uses the ESP32-S3 LCD_CAM peripheral with GDMA and double-buffered bit-plane framebuffers. `COLOR_BITS` selects the number of bit-planes per channel at compile time:

```c
#define COLOR_BITS 8
```

Valid values are `1..8`; therefore the total color depth is `3 * COLOR_BITS` bits per pixel. For example, `COLOR_BITS=6` gives 18 bits per pixel, while `COLOR_BITS=8` gives RGB888 (24 bits per pixel). The firmware keeps the most significant `COLOR_BITS` bits of each incoming RGB888 channel. The base PWM exposure is derived automatically from `COLOR_BITS`, while the binary planes retain weights `1, 2, 4, ...`. The production setting is `COLOR_BITS=8`. A gamma 2.2 lookup table is applied before bit-plane generation; this is the validated production gamma and improves perceived contrast while preserving the RGB888 transport and 8-bit-per-channel input range.

ESP-IDF 5.5.2 is the reference toolchain. After activating its environment:

```sh
make
```

Flash the device with:

```sh
make flash PORT=/dev/cu.usbmodem1101
```

For a full erase and first installation:

```sh
make erase PORT=/dev/cu.usbmodem1101
make flash PORT=/dev/cu.usbmodem1101
```

## displayd

`displayd` requires FreeType development headers and `pkg-config` because TTF/OTF fonts are rendered directly at runtime.

Build with:

```sh
make server
```

Run in the foreground:

```sh
./displayd server
```

or start it as a daemon:

```sh
./displayd start
```

The default TCP port is 5002. The local control socket is `displayd.sock`.

At startup, `displayd` reads `video.conf`. Each non-comment line contains the logical base frame followed by a file from `tmpdata/video/`:

```text
# base  file
0 luxo.raw
2150 goldrake.raw
```

Missing or invalid video entries are reported in the log without preventing the server from starting.

Running `./displayd` with no arguments shows the command help and does not start a server. `./displayd help` shows exactly the same help. `status` includes the running `displayd` version.

Launcher and control commands:

```text
./displayd
./displayd start
./displayd server
./displayd status
./displayd fonts
./displayd set <idx> <step>
./displayd clear <idx>
./displayd video load <base> <file>
./displayd video unload <base>
./displayd video list
./displayd render <des> <rgb>
./displayd stop
./displayd help
```

`fonts` reports the live FreeType cache. For every `font:size[:width]` combination it shows the selected `mono` or `gray` rendering mode, render count, cached glyph count, bitmap bytes, line metrics and the exact characters currently cached.

Example:

```text
Poppins-Regular:20 renders=2 glyphs=7 bitmap_bytes=910 line=30 ascender=21 chars="ADILPSY"
```

## Fonts

Fonts are TTF or OTF files stored in `font/`. The current set is:

```text
Lora.ttf
NotoSansMono.ttf
Poppins-Regular.ttf
Tiny5-Matrix.ttf
Tiny5-Regular.ttf
```

A `.des` text command identifies a font as:

```text
name:size
name:size:width
```

`size` is the FreeType pixel size. Without the optional third field, native proportional metrics from the font are used. With `width`, every character uses a fixed-width cell and the rendered glyph is centered inside it.

Examples:

```text
Poppins-Regular:20
Lora:14
Tiny5-Matrix:9
Tiny5-Matrix:9:6
```

`displayd` creates a cache entry only when a font/size/width combination is first used. Individual characters are rasterized with FreeType only when they are first encountered. Later renders use the cached glyph bitmap. Fonts at 9 pixels or less, and every font using an explicit fixed `width`, are rasterized as hinted monochrome bitmaps for crisp small text. Larger proportional fonts use grayscale antialiasing. The coverage is applied as alpha in RGB888; the physical device then applies the bit depth selected by `COLOR_BITS`.

The current text renderer accepts printable ASCII characters 32 through 126.

## Description and sequence languages

The system has three levels:

```text
.mat -> .seq -> 64x64 RGB888 frame
```

A display serial selects a `.mat` file. A `.mat` normally contains the name of the `.seq` program to execute. If it contains another 12-character display serial instead, the display mirrors that display.

A `.seq` describes what happens over time. Graphic blocks contain the same drawing commands accepted by the standalone `.des` renderer. A `.des` is therefore simply a single static graphic description with no timing or sequence control.

Both languages are line oriented. Blank lines and lines beginning with `#` are ignored. Keywords are case insensitive: `RECT`, `rect`, `Rect` and any other capitalization are equivalent. Uppercase is used in this README only for readability. An unknown command or any other syntax error makes the whole `.seq` invalid; the sequence is not partially executed and the associated client session is closed. The parser tokenizes a `.seq` once when the program is loaded; execution does not repeatedly parse the source text.

### Drawing commands (`.des` and graphic `.seq` blocks)

Commands are executed from top to bottom. Later commands draw over earlier commands. Coordinates refer to the 64x64 display; drawing outside `0..63` is clipped where appropriate. Colors use `RRGGBBAA` hexadecimal notation.

```text
FFFFFFFF  white, opaque
FF0000FF  red, opaque
00000000  black, transparent
```

#### TEXT

```text
TEXT x y foreground background font:size[:width] text
```

`x` and `y` refer to the visible pixel bounding box of the complete text, not to the FreeType line box. For non-negative coordinates, the first visible pixel is placed at that left/top position.

```text
x=-1  align visible right edge to column 63
x=-2  center visible text horizontally
y=-1  align visible bottom edge to row 63
y=-2  center visible text vertically
```

FreeType advances, bearings, baseline and descenders are preserved internally. A non-transparent background covers exactly the visible text bounding box.

Examples:

```text
TEXT -2 0 FFFFFFFF 00000000 Poppins-Regular:20 DISPLAY
TEXT 0 24 FFFF00FF 00000000 Tiny5-Matrix:9 ABC123
TEXT -2 45 00FFFFFF 00000000 Lora:14 Hello
```

#### RECT

```text
RECT x1 y1 x2 y2 color
```

Draws a filled rectangle.

#### LINE

```text
LINE x1 y1 x2 y2 color
```

Draws a line.

#### PIXEL

```text
PIXEL x y color
```

Draws one pixel.

#### IMAGE

```text
IMAGE image alpha
```

Loads `tmpdata/images/image.ff`. Images are 64x64 farbfeld files. `alpha` is a hexadecimal byte from `00` through `FF`.

#### COPY

```text
COPY image alpha x1 y1 x2 y2 destination_x destination_y
```

Copies a rectangular region of a farbfeld image to another position on the frame.

#### SAMPLE

```text
SAMPLE x y Vn
```

Stores one pixel RGB value in drawing color variable `V0` through `V9`.

#### AVERAGE

```text
AVERAGE x1 y1 x2 y2 Vn
```

Stores the average RGB value of a rectangle in drawing color variable `V0` through `V9`.

#### AVERAGELINE

```text
AVERAGELINE x1 y1 x2 y2 Vn
```

Stores the average RGB value sampled along a line in drawing color variable `V0` through `V9`.

A sampled drawing color can later be used wherever a color is expected:

```text
VnMaa
```

`n` is the variable number, `aa` is the hexadecimal alpha, and `M` is:

```text
n  normal
i  inverted
e  enhanced by 50%, saturated at 255
r  reduced by 50%
```

Examples:

```text
V0nFF
V1i80
```

### `.seq` program language

A graphic block starts with:

```text
UNTIL last_step
```

`last_step` is inclusive. The first block starts at step 0; every following block starts immediately after the preceding block. A program loops after its final step.

Example:

```text
UNTIL 20
WAIT 2000
TEXT -2 0 FFFFFFFF 00000000 Poppins-Regular:16 Hello

UNTIL 21
WAIT 5000
TEXT -2 0 FF0000FF 00000000 Lora:18 SALEMI
```

Steps `0..20` use the first frame every 2000 ms; step `21` uses the second frame for 5000 ms, then the program starts again from step 0.

#### WAIT

```text
WAIT milliseconds
```

Sets the display interval for that graphic block. The default is 1000 ms if `WAIT` is omitted.

#### RAND

```text
RAND variable format min max
```

Generates an integer in the inclusive range and stores its formatted text in sequence variable `@variable`.

```text
RAND 10 %04d 2001 2580
RAND 11 %d 0 44
```

#### CALC

```text
CALC variable format expression...
```

Evaluates an integer expression in Reverse Polish Notation. Operands can be integer constants or sequence variables; operators are `+`, `-`, `*`, `/`.

```text
CALC 13 %d @12 1 +
```

stores `@12 + 1` in `@13`.

#### VIDEO

```text
VIDEO last_step first_frame
```

Defines a complete video block without `UNTIL`. Video frames run at 25 fps, 40 ms per frame. The first program step in the block uses `first_frame`, and subsequent steps use consecutive logical video frames.

A program containing only a 2150-frame video loaded at base 0 is:

```text
VIDEO 2149 0
```

Multiple video ranges can be concatenated directly:

```text
VIDEO 2445 2150
VIDEO 4594 0
```

### Sequence variables

Sequence variables use `@n`, with no terminator. They can appear in drawing command arguments and inside text.

`displayd` provides these automatically:

```text
@0  display serial
@1  display-reported IPv4 address
@2  current step
@3  current program name
@4  hour, two digits
@5  minute, two digits
@6  second, two digits
@7  seconds since the current display session started
```

Variables `@8` through `@29` are available to `RAND` and `CALC` (and automatic variables may also be overwritten deliberately if desired).

For example:

```text
TEXT -2 0 FFFFFFFF 00000000 Poppins-Regular:16 @4:@5:@6
TEXT -2 20 00FF00FF 00000000 Tiny5-Matrix:9 @0
```

Sequence variables `@n` are distinct from drawing color variables `V0..V9` used by `SAMPLE`, `AVERAGE` and `AVERAGELINE`.


## Web .des editor

The same `displayd` executable also works as a CGI program. CGI mode is detected automatically and does not start the TCP daemon.

A `GET` request returns the integrated `.des` editor. A `POST` request containing `.des` text uses the same renderer and returns one planar RGB888 frame of 12288 bytes. The browser preview displays the RGB888 frame directly, without color-depth quantization.

The editor is exposed as:

```text
https://music.mazzini.org/displayd
```

## Virtual panel

`virtpanel` is an SDL2 emulator for macOS and other SDL2 systems. It connects to the same server and displays the received RGB888 frame directly, without color-depth quantization.

On macOS install SDL2 if needed:

```sh
brew install sdl2
```

Build and run with:

```sh
make virtual
make virtual-run
```

The default scale is 5 and the default virtual display code is `01`.

```sh
make virtual-run VIRTUAL_SCALE=12 VIRTUAL_CODE=03
./virtpanel [scale] [code]
```

The virtual serial is `0202020202XX`, where `XX` is the selected hexadecimal code. Close the window or press Escape to terminate the emulator.

## Runtime data

Runtime data is stored under:

```text
tmpdata/images/
tmpdata/video/
tmpdata/pgr/
```

Farbfeld images are 64x64 RGBA16 files. `displayd` reads their high RGB bytes as RGB888 before transmission; the physical device then uses the color depth selected by `COLOR_BITS`.
