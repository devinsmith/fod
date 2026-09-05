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

#include "resource.h"
#include "tables.h"
#include "ui.h"
#include "vga.h"

extern struct ui_unknown2 data_074F;
extern unsigned char *scratch;

static uint16_t word_0CCC = 0;
static struct ui_unknown2 *ptr_0CCE = &data_074F;
static struct ui_unknown2 *ptr_0CD0 = &data_074F;

// FEH: DSEG:0x028F
// KEH: DSEG:0x1978
// Will contain 0x00 or 0xFF
static bool inverse_flag = false;

// DSEG:0x3C86
static unsigned char *font_bytes;

// FOD: DSEG:0x029C
// KEH: DSEG:0x1A4C
struct ui_region *active_region;

// DSEG:0x3E66
static struct resource *border_res;

static void ui_sub_048B();

// FOD: 0x00B0
// KEH: 0x077F
// Adds a new region to an existing queue of regions.
// These will be processed later on in ui_sub_048B
void ui_region_queue(uint16_t ax, uint16_t di, uint16_t cx, uint16_t si)
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

// FOD: seg000:0x17F2
// KEH: seg000:0xE308
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
static void draw_border_chr(uint8_t chr_index, int i, int line_num)
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

// FEH: seg000:0x14B3
// KEH: seg000:0xD9F5
void ui_region_queue_rect(const struct ui_rect *input)
{
  uint16_t ax = input->x_pos;  // 0
  uint16_t di = input->width;  // A0
  uint16_t cx = input->height; // C8
  uint16_t si = input->y_pos;  // 0

  // sub_05B0:00B0
  ui_region_queue(ax, di, cx, si);
}

// FEH: seg000:0x14D5
// KEH: seg000:0xDA17
// Screen refresh with input rectangle.
void ui_region_refresh(struct ui_rect *input)
{
  // This refreshes a fraction of the screen, but we can just flush the
  // whole screen and let SDL handle it.
  ui_region_queue_rect(input);

  ui_sub_034D();

  screen_draw(scratch);
}

// FOD: DSEG:0x1631
// KEH: DSEG:0xDC28
void ui_region_refresh_active()
{
  struct ui_region *si = active_region;

  if (si->data_1A != NULL) {
    ui_region_refresh(si->data_1A); // refresh sub-region
  } else {
    ui_region_refresh(&si->rect); // refresh entire region
  }
}

// Load UI specific resources
bool ui_load_res()
{
  border_res = resource_load(RESOURCE_BORDERS, 0, 0);
  if (border_res == NULL) {
    fprintf(stderr, "Couldn't read borders, exiting!\n");
    return false;
  }

  return true;
}

void screen_draw(const unsigned char *bytes)
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

// FOD: seg000:0x14FF
// KEH: seg000:0xDA41
void draw_borders(int offset)
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
        draw_border_chr(al, i, j);
      }
    }
  }
}

// Clears the rectangle associated with the active region
// seg000:0x1593
void ui_active_region_clear()
{
  // Get the rectangle from the active region
  const struct ui_rect *si = &active_region->rect;
  ui_rect_clear(si);
}

// KEH: seg000:15CB
int ui_draw_scroll_list_page(struct player_rec *player,
    uint16_t scroll_offset,
    uint16_t max_count, const char *title,
    void (*cb)(uint16_t, uint16_t, struct player_rec *), bool highlight,
    bool refresh)
{
  uint16_t items_shown;
  uint16_t i;

  items_shown = (scroll_offset + 8 <= max_count) ? 8 : (max_count - scroll_offset);

  ui_active_region_clear();

  if (max_count > 8) {
    // TODO add scrolling glyphs?
  }

  if (highlight) {
    ui_set_inverse(true);
  }
  ui_region_print_str(title, 2, 0);
  ui_set_inverse(false);

  for (i = 0; i < items_shown; i++) {
    cb(i + 1, scroll_offset + i, player);
  }

  return items_shown;
}

// FOD: seg000:0x168E
// KEH: 0xDE40
// Prints a string within a region, at (x,y) coordinates
// "Welcome", 2, 0xB
void ui_region_print_str(const char *str, int x_pos, int y_pos)
{
  struct ui_region *si = active_region;

  if (x_pos != -1) {
    si->cursor_index_x = x_pos + si->initial_x_cursor_pos;
  }
  // 0x16AC
  if (y_pos != -1) {
    si->cursor_index_y = y_pos + si->initial_y_cursor_pos;
  }
  // 0x16BC
  print_wrapped_text(str);
}

// FOD: seg000:0x1614
// KEH: seg000:0xDC0B
static void plot_font_str(const char *str, int len)
{
  // di = str
  // al = es:di
  // cx = len
  struct ui_region *si = active_region;

  for (int i = 0; i < len; i++) {
    plot_font_chr(str[i], si->cursor_index_x, si->line_number, si->cursor_index_y);
    si->cursor_index_x++;
  }
}


// KEH: seg000:0xDDFD
static void reset_offsets()
{
  struct ui_region *si = active_region;

  si->cursor_index_x = si->initial_x_cursor_pos;
  si->cursor_index_y++;
  if (si->cursor_index_y <= si->max_y_cursor_pos) {
    return;
  }

  printf("%s:0xDE12 unhandled\n", __func__);
  exit(0);
}

// FOD: seg000:0x159E
// KEH: seg000:0xDB95
// This routine prints strings within the active region
// by actively looking for line breaks, spaces, etc. and printing them
// out individually.
void print_wrapped_text(const char *str)
{
  int len = (int)strlen(str);
  int max_x = active_region->max_x_cursor_pos;   /* [si+4] */
  int cursor_x = active_region->cursor_index_x;  /* [si+8] */

  int line_start = 0; /* 1E90 */
  int next_start = 0; /* 1E92 */
  int line_end   = 0; /* 1E94 */
  int i = 0;           /* di */

  while (i <= len) {
    char c = (i < len) ? str[i] : '\0';

    if (c == '\0') {
      /* jz short sub_DBF9 -> falls into sub_DBFD -> implicit return */
      line_end = i;
      if (line_end != line_start) {
          plot_font_str(str + line_start, line_end - line_start);
      }
      break;
    }

    if (c == '\r') {
      /* DBAC-DBB9: print pending line, reset, skip the '\r', restart */
      line_end = i;
      if (line_end != line_start) {
          plot_font_str(str + line_start, line_end - line_start);
      }
      reset_offsets();
      i++;
      line_start = next_start = line_end = i;
      continue;
    }

    /* loc_DBBB: would adding this char exceed the available width? */
    int chars_so_far = i - line_start; /* cx = di - word_130D0 */
    if (cursor_x + chars_so_far > max_x) {
      /* loc_DBDF: width overflow */
      if (c == ' ') {
        /* the overflow char is itself a space: break right here */
        line_end = i;
        i++;
        next_start = i;
      } else if (next_start == line_start) {
        /*
         * Safety fallback (deliberate deviation from the original):
         * no space has been seen anywhere in this line, so the
         * original would print nothing and reset di back to
         * line_start forever. Instead, hard-break right here at
         * max_width so we always make forward progress.
         */
        line_end = i;
        next_start = i;
      }
      /* loc_DBED: print span (skipped if zero-length), then advance */
      if (line_end != line_start) {
        plot_font_str(str + line_start, line_end - line_start);
      }
      reset_offsets();
      i = next_start;
      line_start = next_start;
      line_end = next_start;
      continue;
    }

    /* fits within width: track spaces for later backtracking */
    if (c == ' ') {
      line_end = i;
      i++;
      next_start = i;
    } else {
      i++;
    }
  }
}
