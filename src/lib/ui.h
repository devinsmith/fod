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

#ifndef UI_H
#define UI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ui_rect {
  uint16_t x_pos; // Offset 0x0C
  uint16_t y_pos; // offset 0x0E
  uint16_t width; // offset 0x10
  uint16_t height; // offset 0x12
};

struct ui_unknown1 {
  uint16_t arg1;
  uint16_t arg2;
  uint16_t arg3;
  uint16_t arg4;
};

struct ui_unknown2 {
  uint16_t arg0; // BX
  uint16_t arg1; // BX+2
  uint16_t arg2; // BX+4
  uint16_t arg3; // BX+6
  uint16_t arg4; // BX+8
  uint16_t arg5; // BX+A
  struct ui_rect rect; // BX+C
};

// Likely 28 bytes.
struct ui_region {
  uint16_t data_00; // 0x00
  uint16_t data_02; // 0x02
  uint16_t data_04; // 0x04
  uint16_t data_06; // 0x06
  uint16_t data_08; // 0x08
  uint16_t data_0A; // 0x0A - Line start?

  // RECT structure?
  struct ui_rect rect; // offset 0x0C - 0x12

  uint16_t data_14; // offset 0x14
  uint16_t data_16; // (this is actually a function pointer) offset 0x16
  uint16_t data_24; // offset 0x18
  struct ui_rect *data_1A; // offset 0x1A
};

void ui_sub_00B0(uint16_t ax, uint16_t di, uint16_t cx, uint16_t si);
void ui_sub_034D();
void ui_draw_80_line(const uint16_t *src, uint16_t *dest);

// Font stuff
void ui_load_fonts();
void ui_set_inverse(bool inverse);
void plot_font_chr(uint8_t chr_index, int i, int line_num, int base);
void draw_border_chr(uint8_t chr_index, int i, int line_num);

// Clears out an area on the scratch buffer by setting
// the contents to black (0).
void ui_rect_clear(const struct ui_rect *r);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
