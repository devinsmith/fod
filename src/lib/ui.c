/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2018-2020,2025 Devin Smith <devin@devinsmith.net>
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "tables.h"
#include "ui.h"

extern struct ui_unknown2 data_074F;
extern unsigned char *scratch;

static uint16_t word_0CCC = 0;
static struct ui_unknown2 *ptr_0CCE = &data_074F;
static struct ui_unknown2 *ptr_0CD0 = &data_074F;

// DSEG:0x028F
// Will contain 0x00 or 0xFF
static bool inverse_flag = false;

// DSEG:0x3C86
static unsigned char *font_bytes;

static void ui_sub_048B();

void ui_sub_00B0(uint16_t ax, uint16_t di, uint16_t cx, uint16_t si)
{
  word_0CCC++;

  // Is odd?
  if (ax & 0x1) {
    di++;
  }

  if (di & 0x01) {
    di++;
  }

  uint16_t dx = 0xA0;
  dx = dx - di;
  ptr_0CCE->rect.x_pos = dx;

  ax = ax & 0xFE;
  ax = ax / 2;
  di = di / 2;

  ptr_0CCE->arg4 = cx;
  ptr_0CCE->arg5 = di;
  ptr_0CCE->arg2 = ax;

  si = si << 1;
  di = 0x42F; // offset table
  di = di + si;
  ptr_0CCE->arg1 = di;

  di = 0x5BF; // offset table
  di = di + si;

  si = di;
  ax = ax << 1;
  si += ax;

  ptr_0CCE->arg2 = si;
  di = ptr_0CCE->arg5;


//  printf("%s is not completely finished\n", __func__);
}

void ui_sub_034D()
{
  ui_sub_048B();

  ptr_0CCE = &data_074F;
  word_0CCC = 0;
}

static void ui_sub_048B()
{
  ptr_0CD0 = &data_074F;

  // populate framebuffer
  if (word_0CCC == 0) {
    return;
  }

  word_0CCC--;

//  uint16_t si = ptr_0CD0->arg2;
//  uint16_t dx = ptr_0CD0->arg6;
//  uint16_t cx = ptr_0CD0->arg4;
  uint16_t ax = ptr_0CD0->arg3;

  // 0xCC7 = ax  4E4
  //
  ax = ptr_0CD0->arg1;
  ax = ax << 2;
/*
  bx = ptr_0CD0->arg0;
  bp = bx;

  bp += ax;
  di = bp;
  bp += 0x140;

  // ax = 0xCC7
  ds = 0x042D;
  call ax;
  */

}

// seg001:0x04E4
void ui_draw_80_line(const uint16_t *src, uint16_t *dest)
{
  for (int i = 0; i < 80; i++) {
    uint16_t src_pixel = *src++;

    // Extract components
    uint8_t low_byte = src_pixel & 0xFF;
    uint8_t high_byte = src_pixel >> 8;

    // Rotate left by 4, right by 12
    uint16_t rotated_pixel = (src_pixel << 4) | (src_pixel >> 12);
    uint8_t rotated_low = rotated_pixel & 0xFF;

    // Rotated low byte goes to high.
    uint16_t trans1 = (rotated_low << 8) | low_byte;
    uint16_t trans2 = (rotated_pixel & 0xFF00) | high_byte;

    *dest++ = trans1;
    *dest++ = trans2;
  }
}

// Clears out an area on the scratch buffer by setting
// the contents to black (0).
// seg000:17C4
void ui_rect_clear(const struct ui_rect *r)
{
  unsigned char *es = scratch;

  uint16_t di = get_160_offset(r->y_pos);
  di += r->x_pos;

  for (uint16_t i = 0; i < r->height; i++) {
    unsigned char *ptr = es + di;
    for (uint16_t j = 0; j < r->width; j++) {
      *ptr++ = '\0';
    }
    di += 160; // advance to next line.
  }
}

// seg000:0090
void ui_load_fonts()
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
    exit(1);
  }

  // sub_3BFA will expand a 16-bit number to 32 bit, but we don't need
  // to do that on modern architectures.
  fread(font_bytes, 1, size, fp);
  fclose(fp);
}

void ui_set_inverse(bool inverse)
{
  inverse_flag = inverse;
}

// seg000:0x17F2
void plot_font_chr(uint8_t chr_index, int i, int line_num, int base)
{
  bool do_xor = false;
  if (inverse_flag) {
    do_xor = true;
  }

  uint16_t ax = base;

  ax *= 8;   // << 4
  uint16_t di = ax;
  di += line_num / 2;

  di = get_160_offset(di);

  ax = i;
  ax *= 4;
  di += ax;

  unsigned char *es = scratch;
  es += di;

  unsigned char *font_si = font_bytes;
  font_si += (chr_index * 32); // 4x8 = 32 bytes

  // copy font "sprite" over to scratch buffer.
  // fonts are stored in 4 x 8
  for (int k = 0; k < 8; k++) {
    // copy words from ds:si to es:di
    uint16_t value = *((uint16_t *)font_si);
    if (do_xor) {
      value = value ^ 0xFFFF;
    }
    *((uint16_t *)es) = value;
    font_si += 2;
    es += 2;

    value = *((uint16_t *)font_si);
    if (do_xor) {
      value = value ^ 0xFFFF;
    }
    *((uint16_t *)es) = value;
    font_si += 2;
    es += 2;

    es += 0x9C; // next line
  }
}

// FOD: seg000:0x1778
// KEH: seg000:0xE141
void draw_border_chr(uint8_t chr_index, int i, int line_num)
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


