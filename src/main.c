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

// DSEG:0x029E
static struct ui_unknown1 data_029E = { 0, 0, 0xA0, 0xC8 };

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

// DSEG:0x231E - 0x31DE
static unsigned char disk1_bytes[3776];

// DSEG:0x37E6
static unsigned char *arch_bytes;
// DSEG:0x37E8
static unsigned char hds_bytes[1180];
// DSEG:0x3C84
static unsigned char *arch_offset;
// DSEG:0x3C86
static unsigned char *font_bytes;
// DSEG:0x3E66
static unsigned char *border_bytes;

static void sub_14D5(struct ui_unknown1 *input);
static void sub_1548();

static void title_draw(const struct resource *title)
{
  const uint16_t *src = (const uint16_t *)title->bytes;
  uint16_t *dest = (uint16_t *)vga_memory();

  // Processes 16000 compressed pixel groups
  // (16000 * 2 bytes for source -> 16000 * 4 bytes to destination)
  // Every short (2 bytes) defines 4 bytes of the output.
  for (int i = 0; i < 200; i++) {

    ui_draw_80_line(src, dest);
    src += 80;
    dest += 160;
  }
}

static void do_title()
{
  struct resource *title_res = resource_load(RESOURCE_TITLE);

  hexdump(title_res->bytes, 32);
  title_draw(title_res);
  vga_update();

  vga_waitkey();

  resource_release(title_res);
}

static void sub_1778(uint8_t al, int i, int j)
{
  printf("%s - 0x%02X %d %d\n", __func__, al, i, j);

  uint16_t di = i << 4;
  di = get_160_offset(di);

  di += (j << 2);

  unsigned char *es = scratch;
  es += di;

  int ax = al << 5;
  unsigned char *si = font_bytes;
  si += ax;

  for (int k = 0; k < 8; k++) {
    // copy words from ds:si to es:di
    memcpy(es, si, 4);
    si += 4;
    es += 4;

    es += 0x9C;
  }
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
  printf("Font size: 0x%04X\n", size);
  size = size << 5;

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
static void sub_02E5()
{
  FILE *fp = fopen("borders", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't read borders, exiting!\n");
    exit(1);
  }

  fseek(fp, 0x1388, SEEK_SET);
  fread(border_bytes, 1, 1000, fp);
  fclose(fp);

  // archtype
  fp = fopen("archtype", "rb");
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
    fprintf(stderr, "Couldn't read borders, exiting!\n");
    exit(1);
  }

  fread(hds_bytes, 1, sizeof(hds_bytes), fp);
  fclose(fp);
}

/* seg000:0x14FF */
void sub_14FF(int offset)
{
  unsigned char *p = border_bytes + offset;

  for (int j = 0; j < 25; j++) {
    for (int i = 0; i < 40; i++) {
      uint8_t al = *p++;
      if (al != 0) {
        sub_1778(al, i, j);
      }
    }
  }
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

  // Store in 3E66
  border_bytes = malloc(1000);

  sub_02E5();

  // Register VGA driver.
  video_setup();

  if (vga_initialize(GAME_WIDTH, GAME_HEIGHT) != 0) {
    return 1;
  }

  game_mem_alloc();

  /*
  word_231C = 0;
  word_35E4 = 0xFFFF;
  word_33DE = 0;
  */

  do_title();

  sub_1548();

  sub_14D5(&data_029E);

  struct resource *borders = resource_load_sz(RESOURCE_BORDERS, 0x1388, 0x3E8);

  hexdump(borders->bytes, 64);

  sub_14FF(0);

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

// seg000:0x1548
static void sub_1548()
{
  sub_14FF(0);
}

