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

#include "ui.h"

extern struct ui_unknown2 data_074F;

static uint16_t word_0CCC = 0;
static struct ui_unknown2 *ptr_0CCE = &data_074F;
static struct ui_unknown2 *ptr_0CD0 = &data_074F;

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

  printf("%s is not completely finished\n", __func__);
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
