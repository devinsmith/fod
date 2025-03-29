/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2025 Devin Smith <devin@devinsmith.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "common/bufio.h"
#include "common/hexdump.h"
#include "fileio.h"
#include "resource.h"
#include "tables.h"
#include "ui.h"
#include "vga.h"

static const int GAME_WIDTH = 320;
static const int GAME_HEIGHT = 200;

struct unknown_302 {
  // RECT structure?
  uint16_t x_pos; // Offset 0x0C
  uint16_t y_pos; // offset 0x0E
  uint16_t width; // offset 0x10
  uint16_t height; // offset 0x12

  uint16_t data_16; // (this is actually a function pointer) offset 0x16
  uint16_t data_24; // offset 0x18
};

// DSEG:0x0274
static const char *data_274 = "KEH.EXE";

// DSEG:0x029C
static struct unknown_302 *ptr_029C;

// DSEG:0x029E
static struct ui_unknown1 data_029E = { 0, 0, 0xA0, 0xC8 };

// DSEG:0x0302
static struct unknown_302 unknown_302;

// DSEG:0x31E
static struct unknown_302 unknown_31E = { 4, 8, 0x30, 0x60, 0, 0 };

// DSEG:0x0424
static unsigned char unknown1 = 2;

// FOD does most of it's work in a large allocated buffer
// DSEG:0x042D
static unsigned char *scratch;
static unsigned char *scratch_01; // DSEG:0x029A
static unsigned char *scratch_02; // DSEG:0x0292
static unsigned char *scratch_03; // DSEG:0x0296

// DSEG:0x074F
struct ui_unknown2 data_074F = { 0 };

// DSEG:0x2310
static struct unknown_302 *ptr_2310;

// DSEG:0x2312
static uint16_t word_2312 = 0;

// DSEG:0x2314-0x2316
static struct resource *gani_res;

// DSEG:0x231A
static uint8_t byte_231A = 0;
// DSEG:0x231C
static uint16_t word_231C = 0;

// DSEG:0x231E - 0x31DE
static unsigned char disk1_bytes[3776];

static uint16_t word_35E4 = 0xFFFF;

// DSEG:0x37E6
static unsigned char *arch_bytes;
// DSEG:0x37E8
static unsigned char hds_bytes[1180];
// DSEG:0x3C84
static unsigned char *arch_offset;
// DSEG:0x3C86
static unsigned char *font_bytes;
// DSEG:0x3E66
static struct resource *border_res;

static void sub_14D5(struct ui_unknown1 *input);
static void sub_14FF(int offset);
static void sub_1548();
static void sub_155E(struct unknown_302 *arg1, uint16_t arg2);
static void sub_1593();
static void sub_1778(uint8_t chr_index, int i, int line_num);
static void clear_rectangle(const struct unknown_302 *r);

static void screen_draw(const unsigned char *bytes)
{
  const uint16_t *src = (const uint16_t *)bytes;
  uint16_t *dest = (uint16_t *)vga_memory();

  // Processes 16000 compressed pixel groups
  // (16000 * 2 bytes for source -> 16000 * 4 bytes to destination)
  // Every short (2 bytes) defines 4 bytes of the output.
  for (int i = 0; i < 200; i++) {

    ui_draw_80_line(src, dest);
    src += 80;
    dest += 160;
  }

  vga_update();
}

static void do_title()
{
  struct resource *title_res = resource_load(RESOURCE_TITLE, 0, 0);

  hexdump(title_res->bytes, 32);
  screen_draw(title_res->bytes);

  vga_waitkey();

  resource_release(title_res);
}


#if 0
// seg000:0x3BFA
// Inputs CL ? (5)
void sub_3BFA(int counter)
{
  if (counter == 0) {
    return;
  }

  for (int i = 0; i < counter; i++) {
    int carry = (ax & 0x8000) ? 1 : 0;
    ax = ax << 1;
    dx = (dx << 1) | carry;
  }
}
#endif

// seg000:0090
//
void sub_90()
{
  unsigned char font_size[2];

  FILE *fp = fopen("font", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't read font, exiting!\n");
    exit(1);
  }

  fread(font_size, 1, sizeof(font_size), fp);

  int size = (font_size[1] << 8) | font_size[0];
  printf("Total fonts in font file: %d\n", size);
  size = size << 5; // 4 * 8

  font_bytes = malloc(size);
  if (font_bytes == NULL) {
    fclose(fp);
    fprintf(stderr, "Couldn't read font, exiting!\n");
  }

  // sub_3BFA will expand a 16 bit number to 32 bit, but we don't need
  // to do that on modern architectures.
  fread(font_bytes, 1, size, fp);
  fclose(fp);
}

// seg000:1439
// This is approximately what sub_1439 would do.
void game_mem_alloc()
{
  // Fountain of Dreams does most of its work within this scratch
  // buffer. I'm not sure it really needs to be this big but that's how much
  // is allocated in the disassembly.
  scratch = malloc(286544); // 286,544 bytes.
  if (scratch == NULL) {
    fprintf(stderr, "You do not have enough memory to run Fountain of Dreams.\n");
    exit(0);
  }

  scratch_01 = scratch + 0x07d0;
  scratch_02 = scratch_01 + 0x043A;
  scratch_03 = scratch_02 + 0x01CA;

  setup_tables();

  sub_90();

  memset(scratch, 0, 32000);
}


// seg000:0105
static int sub_0105()
{
  FILE *fp = fopen("disk1", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't read disk1, exiting!\n");
    exit(1);
  }

  fread(disk1_bytes, 1, sizeof(disk1_bytes), fp);
  fclose(fp);

  uint16_t word2 = (disk1_bytes[3] << 8) | disk1_bytes[2];
  uint16_t saved_game = (disk1_bytes[1] << 8) | disk1_bytes[0];

  // TODO:
  // This does a number of other manipulations of disk1_bytes based on whether
  // we're dealing with a saved game or not. For a new game, we have to
  // initialize a number of data components to 0.

  if (word2 != 0) {
    fprintf(stderr, "Second word of disk1 is not 0, unhandled (CS: 0x014D)!\n");
    exit(1);
  }
  return saved_game != 0;
}

// seg000:02E5
static void sub_02E5(bool saved_game)
{
  border_res = resource_load(RESOURCE_BORDERS, 0x1388, 1000);
  if (border_res == NULL) {
    fprintf(stderr, "Couldn't read borders, exiting!\n");
    exit(1);
  }

  if (saved_game) {
    return;
  }

  // archtype
  FILE *fp = fopen("archtype", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't read archtype, exiting!\n");
    exit(1);
  }

  long arch_size = file_size(fp);
  printf("archtype %ld bytes\n", arch_size);

  arch_bytes = malloc(arch_size);
  fread(arch_bytes, 1, arch_size, fp);

  fclose(fp);

  uint16_t arch4 = (arch_bytes[5] << 8) | arch_bytes[4];
  printf("arch[4] = 0x%04X\n", arch4);
  arch_offset = arch_bytes += arch4;

  fp = fopen("hdspct", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't read hdspct, exiting!\n");
    exit(1);
  }

  fread(hds_bytes, 1, sizeof(hds_bytes), fp);
  fclose(fp);
}

// seg000:0x044E
static void sub_044E(uint16_t arg1)
{
  ptr_2310 = &unknown_31E;

  uint16_t ax = word_35E4;
  if (arg1 != ax) {
    // 0x462
    ax = arg1;
    word_35E4 = ax;

    // Open and decompress GANI
    gani_res = resource_load(RESOURCE_GANI, 0, 0);

    uint8_t ah = gani_res->bytes[2];
    uint8_t cl = gani_res->bytes[1];

    uint16_t ax = ah << 8 | cl;
    word_2312 = ax;
    byte_231A = 1;
  }
  word_231C = 1;
}

// seg001:0x0F80
static void seg001_sub_0F80(struct resource *res, int res_offset,
    int arg2, int line_num)
{
  unsigned char *es = scratch;

  uint16_t di = get_160_offset(line_num);

  uint16_t ax = arg2;
  ax = ax / 2;
  di += ax;
  unsigned char *ds = res->bytes;
  uint16_t si = res_offset;
  
  ax = 0;
  uint16_t dx = 0;
  uint8_t al = ds[si];
  uint8_t dl = ds[si+1];

  uint16_t bx = 0xA0;
  bx = bx - al;
  si += 4;

  for (int i = 0; i < dl; i++) {
    uint16_t cx = al;
    // repe movsw
    // copy words from ds:si to es:di cx times, increase si/di
    memcpy(es + di, ds + si, cx);
    si += cx;
    di += cx;

    di += bx;
  }
}

// seg001:0x0FCD
static void seg001_sub_0FCD(uint8_t input,
    uint16_t res_offset,
    struct resource *r,
    uint16_t arg4,
    uint16_t arg5)
{
  unsigned char *es = r->bytes;
  uint16_t di = res_offset;
  uint16_t bx = input;

  bx = bx << 1;
  bx += 3;
  di += es[bx+di];
  uint8_t al = es[di+3];
  uint16_t ax = al + arg4;
  uint16_t save_ax = ax;
  // save ax
  ax = 0;
  al = es[di+2];
  ax = al + arg5;

  // push ax
  // push es
  // push di
  seg001_sub_0F80(r, di, save_ax, ax);
}

// seg000:0x04EA
static void sub_04EA(uint16_t arg1)
{
  uint16_t gani_offset = word_2312;
  uint16_t bx = word_2312;

  gani_offset++;

  uint8_t al = gani_res->bytes[bx];
  uint8_t bp0E = al;

//  uint16_t bp02 = ptr_2310;
  uint16_t bp08 = 0;

  // 0x54E
  al = bp0E;
  if (al > bp08) {
    // ptr_2310
    // push ptr_2310->y_pos;
    // ax = ptr_2310->x_pos;
    // ax = ax << 1;
    // push ax;
    // push gani_res
    // push gani_res offset (0)
    bx = gani_offset;
    gani_offset++;
    al = gani_res->bytes[bx];

    // push ax
    // Takes 5 arguments
    // these come from 31E rectangle struct
    seg001_sub_0FCD(al, 0, gani_res, 8, 8);
  }


  // decoding GANI
#if 0
  uint16_t ax = word_2312;
  ax++;

  uint8_t al = gani_res->bytes + word_2312;

//  uint16_t new_ax = word_2310;
//  new_ax += 0xC;
  uint16_t bp08 = 0;

  if (al > bp08) {
    //uint16_t bx = new_ax;
    // 

    //sub_05AE_0FCD();
  }
#endif



  printf("%s:0x04EA not finished\n", __func__);
  exit(1);
}

static void sub_1631()
{
  printf("%s:0x1631 not finished\n", __func__);
  exit(1);
}

// seg000:1D34
static void sub_1D34()
{

}

// seg000:22B4
static void sub_22B4()
{
  uint16_t ax = 0x164;
  sub_1D34();
}

// sprintf ?
// looking for % characters
static void sub_22BD()
{
  // 2B54
}

// seg000:3338
// takes 3 variables
static void sub_3338()
{
  // setting up some structure
  sub_22B4();
}

static void sub_0010(const char *arg1, uint16_t arg2)
{
  // push si
  // push arg1
  // push 0x70
  // lea ax, [bp-2a] // local variable or gani?
  // push ax
  sub_3338(); // <-- sprintf?
              //
  //sub_32C2();
  printf("%s:0x0010 not finished\n", __func__);
  exit(1);
}

// seg000:0x071B
// 2 arguments
// ex. 0x0033, 0x01CC = "Welcome"
void sub_071B(uint16_t arg1, const char *arg2)
{
  // 2 parameters
  sub_155E(&unknown_31E, 1);

  sub_044E(arg1);
  sub_04EA(1);

  uint16_t ax = 0xB;
  sub_0010(arg2, 0xB);
  sub_1631();
}


// seg000:0x1191
int main(int argc, char *argv[])
{
  if (!rm_init()) {
    fprintf(stderr, "Failed to initialize resource manager, exiting!\n");
    return 1;
  }

  int saved_game = sub_0105();
  printf("Saved game: %d\n", saved_game);
  unknown1 = disk1_bytes[6];
  printf("Unknown1: %d\n", unknown1);

  sub_02E5(saved_game);

  // Register VGA driver.
  video_setup();

  if (vga_initialize(GAME_WIDTH, GAME_HEIGHT) != 0) {
    return 1;
  }

  game_mem_alloc();

  word_231C = 0;
  word_35E4 = 0xFFFF;
  /*
  word_33DE = 0;
  */

  do_title();

  sub_1548();

  // 14D5 and screen_draw are similar
  sub_14D5(&data_029E);
  screen_draw(scratch);

//  int local_val = 0;
  sub_155E(&unknown_302, 0);

  // Only if it's a new game
  sub_071B(0x0033, "Welcome");

  vga_waitkey();

  free(scratch);

  vga_end();

  return 0;
}

static void sub_14B3(struct ui_unknown1 *input)
{
  uint16_t ax = input->arg1;
  uint16_t di = input->arg3;
  uint16_t cx = input->arg4;
  uint16_t si = input->arg2;

  // sub_05B0:00B0
  ui_sub_00B0(ax, di, cx, si);
}

// seg000:0x14D5
static void sub_14D5(struct ui_unknown1 *input)
{
  sub_14B3(input);

  ui_sub_034D();
}

/* seg000:0x14FF */
static void sub_14FF(int offset)
{
  unsigned char *p = border_res->bytes + offset;

  // Draws border segments in 4x8 tiles.
  // 40*4 = 160 (expanded to 320 in screen draw)
  // 25*8 = 200
  for (int j = 0; j < 25; j++) {
    for (int i = 0; i < 40; i++) {
      uint8_t al = *p++;
      if (al != 0) {
        // al = font index
        sub_1778(al, i, j);
      }
    }
  }
}

// seg000:0x1548
static void sub_1548()
{
  sub_14FF(0);
}

// seg000:0x155E
static void sub_155E(struct unknown_302 *arg1, uint16_t arg2)
{
  // arg1 is likely a pointer
  ptr_029C = arg1;
  if (arg2 == 0) {
    return;
  }

  arg1->data_24 = 0;
  sub_1593();

  if (arg1->data_16 != 0) {
    // Call function pointer
    printf("%s:0x1586 unhandled\n", __func__);
  }
}

// seg000:0x1593
static void sub_1593()
{
  struct unknown_302 *si = ptr_029C;
  //si += 0x000C; // Advance 12 bytes in to "rect" structure
  clear_rectangle(si);
}

// seg000:1778
static void sub_1778(uint8_t chr_index, int i, int line_num)
{
  uint16_t ax = line_num << 3; // multiply by 8 because a font sprite is 8 lines high.
  uint16_t di = get_160_offset(ax);

  di += (i << 2);

  unsigned char *es = scratch;
  es += di;

  unsigned char *si = font_bytes;
  si += (chr_index * 32); // 4x8 = 32 bytes

  // copy font "sprite" over to scratch buffer.
  // fonts are stored in 4 x 8
  for (int k = 0; k < 8; k++) {
    // copy words from ds:si to es:di
    memcpy(es, si, 4);
    si += 4;
    es += 4;

    es += 0x9C; // next line
  }
}

// Clears out an area on the scratch buffer by setting
// the contents to black (0).
// seg000:17C4
static void clear_rectangle(const struct unknown_302 *r)
{
  unsigned char *es = scratch;

  uint16_t di = get_160_offset(r->y_pos);
  di += r->x_pos;

  for (uint16_t i = 0; i < r->height; i++) {
    unsigned char *ptr = es + di;
    for (uint16_t j = 0; j < r->width; j++) {
      *ptr++ = '\0';
    }
    di += 0xA0; // advance to next line.
  }
}

// takes KEH.EXE and 0x2E ?
static void sub_33AA()
{
}

static void sub_338E()
{
  // ah, 3d<F10>
  // int 21
}

static void sub_33D4()
{
}

// 0x3A14
// 3 arguments
static void sub_3A14()
{
  // sub_1D34()
  //

}

static void sub_3A1D()
{
  // sub_33D4()

}
