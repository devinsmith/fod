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

#include "anim.h"
#include "game.h"
#include "level.h"
#include "npc.h"
#include "platform.h"
#include "resource.h"
#include "tables.h"
#include "ui.h"

extern struct ui_unknown2 data_074F;
extern unsigned char *scratch;

static uint16_t word_0CCC = 0;
static struct ui_unknown2 *ptr_0CCE = &data_074F;
static struct ui_unknown2 *ptr_0CD0 = &data_074F;

// KEH DSEG:0xBEE6
uint16_t word_BEE6 = 0;

// KEH: DSEG:0x1834
uint8_t byte_1834 = 0;

// KEH: DSEG:0x1979
bool do_expand = false;

// FEH: DSEG:0x028F
// KEH: DSEG:0x1978
// Will contain 0x00 or 0xFF
static bool inverse_flag = false;

// DSEG:0x3C86
static unsigned char *font_bytes;

// FOD: DSEG:0x029C
// KEH: DSEG:0x1A4C
struct ui_region *active_region;

// KEH: DSEG:0x1A7E
static struct ui_rect middle_rect = { 0x4C, 0x58, 0x8, 0x10 };

// KEH DSEG:0xDAE6
bool bool_DAE6 = false;

// KEH: DSEG:0x1E96
static uint16_t word_1E96;
// KEH: DSEG:0x1E98
static uint16_t word_1E98;

// KEH DSEG:0x1EA4
static uint16_t map_tile_array[9 * 19];

// DSEG:0x3E66
static struct resource *border_res;

// KEH: DSEG: 0xD1D4
static uint16_t word_D1D4;

// KEH: DSEG:0x9E0A
uint16_t scroll_delay_ms[8] = {
  2400, 1000, 500, 1, 100, 15, 250, 5
};

// KEH: DSEG:0x1A4E
struct ui_rect whole_screen = {
  0, 0, 160, 200
};

// KEH: DSEG:0x1C7C
struct ui_region full_screen_region = {
  0,
  0,
  0x27,
  0x18,
  0,
  0,
  { 0, 0, 0xA0, 0xC8 },
  0x00,
  NULL,
  0x0,
  NULL
};

// KEH: DSEG:0x1AF4
static struct ui_region party_region = {
  0x01, // Initial X cursor position
  0x13, // Initial Y cursor position
  0x26, // Max X cursor position
  0x17, // Max Y cursor position
  0x01, // Cursor index x
  0x13, // Cursor index y
  { 0x04, 0x9C, 0x98, 0x28 }, // ui rect
  0x08,
  NULL,
  0x00,
  NULL
};

// Sprite overlays
// KEH: DSEG:0x9E1A
static uint16_t overlay_table[] = {
  0x0000, 0x1E68, 0x1E60, 0x1F38, 0x1E48, 0x1E50,
  0x1E58, 0x1EC8, 0x1ED0, 0x1ED8, 0x1EE0, 0x1EE8,
  0x1EF0, 0x1EF8, 0x1F00, 0x1F08, 0x1F10, 0x1F18,
  0x1F20, 0x1F28, 0x1F30
};

static void ui_sub_048B();
static void draw_border_chr(uint8_t chr_index, int i, int line_num);
static void prepare_text(const char *str);

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

void ui_set_sprite_overlay(int idx, uint16_t val)
{
  overlay_table[idx] = val;
}

uint16_t ui_get_sprite_overlay(int lookup)
{
  return overlay_table[lookup];
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

#define STR_END   0xFF
#define STR_BLANK 0x00
#define FULL_SCREEN_TEXT_LINE_OFS 4

// KEH: seg000:DDC5
// Buffer: [x][y][chr]...[0xFF]; chr 0x00 = skip cell
void ui_print_positioned_str(const uint8_t *str)
{
  struct ui_region *region = &full_screen_region;

  uint16_t saved = region->line_number;
  region->line_number = FULL_SCREEN_TEXT_LINE_OFS;

  region->cursor_index_x = str[0];
  region->cursor_index_y = str[1];
  str += 2;

  while (*str != STR_END) {
    if (*str != STR_BLANK) {
      plot_font_chr(*str,
        region->cursor_index_x,   // i
        region->line_number,      // line_num
        region->cursor_index_y);  // base
    }
    region->cursor_index_x++;
    word_D1D4++;
    str++;
  }

  region->line_number = saved;
}

// KEH: seg000:DD5F
// Prints a buffer of characters horizontally, increasing X
// Buffer: [x][y][chr][chr]...[0xFF]
static void ui_print_border_chr_hz(const uint8_t *str)
{
  uint8_t x = str[0];
  uint8_t y = str[1];
  str += 2;

  while (*str != STR_END) {
    draw_border_chr(*str, x, y);
    x++;
    str++;
  }
}

// KEH: seg000:DD92
// Prints a buffer of characters vertically, increasing Y
// Buffer: [x][y][chr][chr]...[0xFF]
static void ui_print_border_chr_vr(const uint8_t *str)
{
  uint8_t x = str[0];
  uint8_t y = str[1];
  str += 2;

  while (*str != STR_END) {
    draw_border_chr(*str, x, y);
    y++;
    str++;
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

#define LIST_PAGE_SIZE 8

// KEH: seg000:15CB
int ui_draw_scroll_list_page(void *ud,
    uint16_t scroll_offset,
    uint16_t max_count, const char *title,
    void (*cb)(uint16_t, uint16_t, void *), bool highlight,
    bool refresh)
{
  uint16_t items_shown;
  uint16_t i;

  items_shown = (scroll_offset + 8 <= max_count) ? 8 : (max_count - scroll_offset);

  ui_active_region_clear();

  if (max_count > 8) {
    // TODO add scrolling glyphs?
    printf("%s: TODO, add scrolling glyphs\n", __func__);
  }

  if (highlight) {
    ui_set_inverse(true);
  }
  ui_region_print_str(title, 2, 0);
  ui_set_inverse(false);

  for (i = 0; i < items_shown; i++) {
    cb(i + 1, scroll_offset + i, ud);
  }

  return items_shown;
}

// Sets the active region and optionally clears it.
// FOD:seg000:0x155E
// KEH:seg000:0xDB18
void ui_region_set_active(struct ui_region *arg1, bool clear)
{
  active_region = arg1;

  if (!clear) {
    return;
  }

  arg1->data_24 = 0;
  ui_active_region_clear();

  if (arg1->func_ptr != NULL) {
    // Call function pointer
    arg1->func_ptr();
  }
}

// KEH: seg000:0x1682 - process scroll-list interaction
int ui_scroll_list_menu(void *ud, uint16_t max_count,
    uint16_t *scroll_offset, const char *title,
    void (*cb)(uint16_t, uint16_t, void *), bool highlight)
{
  int shown = 0; // rows drawn on current page
  int sel = 0; // digit chosen
  bool done = false;
  bool redraw = true;

  // Offset sitting exactly at the end: back up one page
  if (*scroll_offset == max_count && max_count > 0) {
    *scroll_offset -= LIST_PAGE_SIZE;
  }

  while (!done) {
    if (redraw) {
      shown = ui_draw_scroll_list_page(ud, *scroll_offset, max_count, title, cb, highlight, true);
      redraw = false;
    }

    int key = sys_readkey();
#if 0
    if (key > 0) {
      printf("Key = 0x%02X\n", key);
    }
#endif

    if (key >= '0' && key <= '9') {
      sel = key & 0x0F;
      if (sel > 0 && sel <= shown) {
        done = true;
      }
      continue;
    }

    switch (key) {
#if 0
    case KEY_PGDN:
        if ((int)max_count - (int)*scroll_offset > shown) {
            *scroll_offset += LIST_PAGE_SIZE;
            redraw = true;
        }
        break;

    case KEY_PGUP:
        if (*scroll_offset >= LIST_PAGE_SIZE) {
            *scroll_offset -= LIST_PAGE_SIZE;
            redraw = true;
        }
        break;

    case KEY_EXTRA_A:
    case KEY_EXTRA_B:
        if (highlight)
            return key;
        break;
#endif
    case KEY_ESC:
      return key;

    default:
      if (key >= KEY_FN_LO && key <= KEY_FN_HI)
        return key - 0x46;               /* -11..-7 */
      break;
    }
  }

  return (int)*scroll_offset + sel - 1;
}

// FOD: seg000:0x168E
// KEH: 0xDE40
// Prints a string within a region, at (x,y) coordinates
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
  prepare_text(str);
}

// KEH: seg000:0x2AFC
void ui_region_print_centered_str(const char *str, int y_pos)
{
  char buffer[42];

  snprintf(buffer, sizeof(buffer), "%s", str);
  int max_char_len = active_region->rect.width / 4;
  buffer[max_char_len] = '\0';

  // Print centered based on length.
  size_t str_len = strlen(buffer);
  ui_region_print_str(buffer, (max_char_len - str_len) / 2, y_pos);
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

#define ROW_STRIDE 0xA0

// KEH: seg000:0xE18D scrolls the region up 8 rows (one text line), animated.
void ui_region_scroll_line(const struct ui_region *r)
{
  const struct ui_rect *rc = &r->rect;

  size_t dst = get_160_offset(rc->y_pos) + rc->x_pos;
  size_t src = dst;
  uint16_t rows = rc->height;
  uint16_t words = rc->width;
  int steps;

  int scroll_speed = sys_scroll_speed();

  if (scroll_speed < 5) {
    rows -= 1;  words >>= 1;  steps = 8;  src += 1 * ROW_STRIDE;
  } else if (scroll_speed < 7) {
    rows -= 2;                steps = 4;  src += 2 * ROW_STRIDE;
  } else {
    rows -= 4;  words <<= 1;  steps = 2;  src += 4 * ROW_STRIDE;
  }

  // E1DA
  while (steps--) {
    for (uint16_t row = 0; row < rows; row++) {
      memmove(scratch + dst + (size_t)row * ROW_STRIDE,
        scratch + src + (size_t)row * ROW_STRIDE, (size_t)words * 2);
    }
    memset(scratch + dst + (size_t)rows * ROW_STRIDE, 0, (size_t)words * 2);

    const struct ui_rect *ar = &active_region->rect;
    ui_region_queue(ar->x_pos, ar->width, ar->height, ar->y_pos);
    ui_sub_034D();
    if (scroll_speed != 0) {
      // Delay loop (keep any tile animations ticking)
      uint64_t end = sys_ticks() + scroll_delay_ms[scroll_speed - 1];
      do { anim_update(false); sys_delay(1); } while (sys_ticks() < end);
      // for slowing down or speeding up scolling.
      scroll_speed = sys_poll_scroll_keys();
    }
  }
}

// KEH: seg000:0xDDFD
void ui_region_newline_or_scroll()
{
  struct ui_region *si = active_region;

  si->cursor_index_x = si->initial_x_cursor_pos;
  si->cursor_index_y++;
  if (si->cursor_index_y <= si->max_y_cursor_pos) {
    return;
  }

  si->cursor_index_y = si->max_y_cursor_pos;

  if (byte_1834 != 0) {
    printf("%s:0xDE20 unhandled\n", __func__);
    exit(0);
  }

  // DE39
  ui_region_scroll_line(si);
}

char str_tag_buffer[2048];

// KEH: seg000:DB58
// Replaces '^' in strings with the current player name.
static char *expand_player_name(const char *src)
{
  char *dst = str_tag_buffer;

  for (;;) {
    char c = *src++;

    if (c == '\0') {
      *dst = '\0';          /* loc_DB87: stosb of the terminator */
      break;
    }

    if (c == '^') {
      const char *rep = current_player_name;
      while (*rep) {
        *dst++ = *rep++;
      }
    } else {
      *dst++ = c;           /* loc_DB84 */
    }
  }

  return str_tag_buffer;
}

// KEH: seg000:DB8B
static void prepare_text(const char *str)
{
  if (do_expand) {
    str = expand_player_name(str);
  }
  print_wrapped_text(str);
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
      ui_region_newline_or_scroll();
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
      ui_region_newline_or_scroll();
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

void draw_map_tile(uint16_t tile_id, int x, int y)
{
  int line_num = (y * 16) + 24;
  int rows = 16;
//  int cols = 4;

  uint16_t bx = 0;
  int sprite = tile_id & 0x7FF;
  int overlay = 0;

  if (sprite >= 512) {
    sprite -= 512;
    bx = 0x1000;
    printf("%s: 0xE0A7 unhandled, tile_id = 0x%04X 0x%04X %d %d\n", __func__,
        tile_id, tile_id & 0xF800, x, y);
  }

  // NPCs and other characters are rendered as sprite overlays on top of the
  // tile map.
  if (tile_id >= 0x800) {
    // Calculate overlay.
    uint16_t mask = tile_id & 0xF800;
    uint16_t mask_flipped = (mask & 0xFF) << 8 | mask >> 8;
    int table_lookup = mask_flipped >> 3;
    if (table_lookup > 20) {
      printf("%s: Overlay too large, check 0xE0E4\n", __func__);
      table_lookup = 20;
    }

    overlay = ui_get_sprite_overlay(table_lookup);
    // Convert from segment:offset to absolute, then index.
    overlay = (overlay * 16) / 128;
    printf("%s: Overlay number: %d\n", __func__, overlay);
  }

  int offset = sprite * 128; // 8*16 bytes
  if (bx > 0) {
    offset += bx;
  }

  unsigned char *es = scratch;
  uint16_t di = get_160_offset(line_num);
  di += (x << 3);
  di += 4; // indentation
  es += di;

  unsigned char *si = tile_res->bytes + offset;

  // copy tile "sprite" over to scratch buffer.
  // tiles are stored in 8 x 16
  for (int k = 0; k < rows; k++) {
    // copy words from ds:si to es:di
    memcpy(es, si, 8);
    si += 8;
    es += 8;

    es += 0x98; // next line
  }

  if (overlay > 0) {
    // KEH: 0xE10E
    es = scratch;
    di = get_160_offset(line_num);
    di += (x << 3);
    di += 4; // indentation
    printf("Starting di: 0x%04X\n", di);
    int index = 0;

    offset = overlay * 128; // 8*16 bytes
    si = tile_res->bytes + offset;

    for (int j = 0; j < rows; j++) {
      for (int k = 0; k < 8; k++) {
        uint8_t al = *si;
        al &= 0xF0;
        if (al != 0) {
          es[di] &= 0x0F;
          es[di] |= al;
        }
        index++;
        al = *si++;
        al &= 0x0F;
        if (al != 0) {
          es[di] &= 0xF0;
          es[di] |= al;
        }
        di++;
      }
      di += 0x98; // next line
    }
  }
}

// KEH: seg000:0xDFEE
void draw_map_center_tile()
{
  word_1E96 = 9;
  word_1E98 = 4;

  int center = (4 * 19) + 9;
  map_tile_array[center] |= 0x800;

  draw_map_tile(map_tile_array[center], 9, 4);

  ui_region_refresh(&middle_rect);
}

static const char *condition_names[] = {
  "OK ",
  "UNC", // Unconcious
  "SER", // Serious
  "CRT", // Critical
  "COM", // Comotose
  "DED"  // Dead
};

// KEH: seg000:0x07E1
void ui_draw_party_roster(uint16_t selected, bool highlight, bool full_refresh)
{
  char buf[32];
  struct ui_region *saved_region = active_region;

  ui_region_set_active(&party_region, false);
  ui_active_region_clear();

  for (int i = 0; i < g_game_state.party_size; i++) {
    struct player_rec *p = &g_game_state.players[i];

    uint16_t ac = player_armor_class(p);

    // Get ammo
    int ammo = 0;
    if (p->weapon_idx != 0xFF) {
      printf("%s: 0x9CC unimplemented (Weapon: %d)\n", __func__, p->weapon_idx);
    }

    // XXX: need to look this up properly.
    const char *wname = "Hands";

    // Look up DSEG 0x197A if weapon is not 0xFF

    snprintf(buf, sizeof(buf), "F%1d>", i + 1);
    if (highlight && selected == i) {
      inverse_flag = true;
    }

    ui_region_print_str(buf, 0, i);
    inverse_flag = false;

    // 0x890
    // Look up DSEG 0x197A is not 0xFF
    // sub_69E8(i, wname);
    //   continue;

    // 0x89D
    // column 3: name, AC, ammo
    snprintf(buf, sizeof(buf), "%-12.12s %2.1d %3.1d", p->name, (int)ac, ammo);
    ui_region_print_str(buf, 3, i);

    // column 23: MAX?
    snprintf(buf, sizeof(buf), "%3.1d", p->max_condition);
    if (p->affliction) {
      inverse_flag = true;
    }
    ui_region_print_str(buf, 23, i);
    inverse_flag = false;

    // column 27: CON -- condition text, else numeric value
    int cond = get_player_condition_status(p);
    if (cond > 0) {
      inverse_flag = true;
      ui_region_print_str(condition_names[cond], 27, i);
      inverse_flag = false;
    } else {
      snprintf(buf, sizeof(buf), "%3.1d", p->condition);
      ui_region_print_str(buf, 27, i);
    }

    // column 31: WEAPON
    // Is weapon jammed?
    // if (wsel != 0xFF && (player_slot(p, wsel)->flags & 1)) {
    //   inverse_flag = true;
    // }
    snprintf(buf, sizeof(buf), "%-7.7s", wname);
    ui_region_print_str(buf, 31, i);
    inverse_flag = false;
  }

  if (full_refresh) {
    ui_region_refresh_active();
  } else {
    ui_region_queue_rect(&active_region->rect);
  }

  ui_region_set_active(saved_region, false);
}

// KEH: seg000:DAD3
static void sub_DAD3()
{
  // 0x189B
  const uint8_t str1[] = {
    7, 5, 0x84, 0x80,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x80, 0x84, 0xFF
  };

  // 18B8
  const uint8_t str2[] = {
    7, 6, 0x82,
    0xB5, 0xB5, 0xB5, 0xB5, 0xB5,
    0xB5, 0xB5, 0xB5, 0xB5, 0xB5,
    0x82, 0xFF
  };

  // 18C7
  const uint8_t str3[] = {
    0x20, 0x6, 0x82,
    0xB5, 0xB5, 0xB5, 0xB5, 0xB5,
    0xB5, 0xB5, 0xB5, 0xB5, 0xB5,
    0x82, 0xFF
  };

  // 0x18D6
  const uint8_t str4[] = {
    7, 0x12, 0x84, 0x80,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x80, 0x84, 0xFF
  };

  // Top
  ui_print_border_chr_hz(str1);
  // Left
  ui_print_border_chr_vr(str2);
  // Right
  ui_print_border_chr_vr(str3);
  // Bottom
  ui_print_border_chr_hz(str4);
}

#define VIEW_W 19
#define VIEW_H 9

/* KEH: seg000:0xDD19 */
static void draw_map()
{
  // Draws our map 19x9 sprites
  for (int row = 0; row < VIEW_H; row++) {
    for (int col = 0; col < VIEW_W; col++) {
      draw_map_tile(map_tile_array[col + (row * 19)], col, row);
    }
  }
}

// Cell-to-display-tile conversion (extraction of DCE9 block in
// build_map_display). b2 bit 6 wins over bit 7. A plain cell
// keeps all 16 bits of word0.
static uint16_t cell_to_display_tile(const struct map_cell *cell)
{
  uint16_t w = cell->word0;

  if (cell->b2 & 0x40)
    return (w & 0x07FF) | 0x1800;
  if (cell->b2 & 0x80)                    // interaction flag
    return (w & 0x07FF) | 0x1000;
  return w;
}

// KEH: seg000:0xDC46
// Builds the tile buffer for the visible map around
// the player.
void build_map_display()
{
  const uint16_t map_w = level_map->map_width;
  const uint16_t map_h = level_map->map_height;

  uint16_t outside = level_map->filler_tile & 0x7FFF;
  printf("%s: DC51 - (outside: 0x%04X)\n", __func__, outside);

  printf("%s: DC57 - (x pos: 0x%04X)\n", __func__, g_game_state.x_pos);
  uint16_t left_edge = g_game_state.x_pos - 9; // left edge of view

  printf("%s: DC63 - (y pos: 0x%04X)\n", __func__, g_game_state.y_pos);
  uint16_t top_edge = g_game_state.y_pos - 4; // top edge of view

  uint16_t *di = map_tile_array;

  // DC79
  for (uint16_t row = 0; row < VIEW_H; row++) {
    uint16_t map_y = top_edge + row;
    for (uint16_t col = 0; col < VIEW_W; col++) {
      uint16_t map_x = left_edge + col;

      // If the coordinates are outside the map boundaries, fill with outside tile.
      // The 0x8000 tests are likely redundant but kept to match with FOD
      if (map_x >= map_w || map_y >= map_h ||
          (map_x & 0x8000) || (map_y & 0x8000)) {
        *di++ = outside;
        continue;
      }

      const struct map_cell *cell =
          &level_map->cells[(size_t)map_y * map_w + map_x];

      // DCE9
      *di++ = cell_to_display_tile(cell);
    }
  }

  // Add center portion
  int center = (VIEW_H / 2) * VIEW_W + VIEW_W / 2;
  map_tile_array[center] &= 0x7FF;
  map_tile_array[center] |= 0x800;

  draw_map();
}

// KEH: seg000:0xDD4C
// Redraws the map viewport: rebuilds the tile buffer for the area
// around the player, draws it, and refreshes that region of the screen.
void redraw_map_view(void)
{
  build_map_display();

  // KEH: DSEG:0x1A5E
  struct ui_rect data_1A5E = { 4, 24, 152 /* 19 * 8 */, 144 /* 9 * 16 */ };

  ui_region_refresh(&data_1A5E);
}

// KEH: DSEG:0x1DE8
static struct ui_region player_f1_region = {
  1,
  1,
  6,
  1,
  1,
  1,
  { 0x4, 0x8, 0x18, 0x8 }, // rect offset 0x0C-0x12
  0,
  0,
  0,
  NULL
};

// KEH: DSEG:0x1E04
static struct ui_region player_f2_region = {
  8,
  1,
  0xD,
  0x1,
  0x8,
  0x1,
  { 0x20, 0x8, 0x18, 0x8 }, // rect offset 0x0C-0x12
  0,
  0,
  0,
  NULL
};

// KEH: DSEG:0x1E20
static struct ui_region player_f3_region = {
  0xF,
  1,
  0x14,
  0x1,
  0x0F,
  0x1,
  { 0x3C, 0x8, 0x18, 0x8 }, // rect offset 0x0C-0x12
  0,
  0,
  0,
  NULL
};

// KEH: DSEG:0x1E3C
static struct ui_region player_f4_region = {
  0x16,
  1,
  0x1B,
  0x1,
  0x16,
  0x1,
  { 0x58, 0x8, 0x18, 0x8 }, // rect offset 0x0C-0x12
  0,
  0,
  0,
  NULL
};

// KEH: DSEG:0x1E58
static struct ui_region player_f5_region = {
  0x1D,
  1,
  0x22,
  0x1,
  0x1D,
  0x1,
  { 0x74, 0x8, 0x18, 0x8 }, // rect offset 0x0C-0x12
  0,
  0,
  0,
  NULL
};

static struct ui_region *player_regions[] = {
  &player_f1_region,
  &player_f2_region,
  &player_f3_region,
  &player_f4_region,
  &player_f5_region
};

// KEH: seg000:0x0A04
void ui_draw_party_names_header(bool refresh)
{
  struct ui_region *old_region = active_region;

  for (int i = 0; i < 5; i++) {
    const struct player_rec *current_player = &g_game_state.players[i];

    ui_set_inverse(false);
    ui_region_set_active(player_regions[i], false);
    ui_active_region_clear();

    if (i < g_game_state.party_size) {
      ui_region_print_centered_str(current_player->name, 0);

      if (current_player->affliction != 0) {
        // 0x0A57
        // ui_draw_affliction_marker ? (E2B8) ?
        printf("%s:0x0A57 unhandled\n", __func__);
      }
    }

    if (refresh) {
      ui_region_refresh_active();
    }
  }

  ui_region_set_active(old_region, false);
}

// KEH: DSEG:0x0142
static const char *days[] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

// KEH: DSEG:0x1C98
struct ui_region datetime_region = {
  0x01,
  0x16,
  0x05,
  0x17,
  0x01,
  0x16,
  { 0x04, 0xB0, 0x14, 0x10 }, // rect offset 0x0C-0x12
  0x00,
  0x00,
  0x00,
  NULL
};

// KEH: seg000:0x1033
void draw_day_time()
{
  struct ui_region *old_region = active_region;

  char current_time[16];

  snprintf(current_time, sizeof(current_time), "%2.2d:%.2d",
           g_game_state.hour, g_game_state.minute);

  ui_region_set_active(&datetime_region, false);

  ui_region_print_str(days[g_game_state.day_of_week], 1, 0);
  ui_region_print_str(current_time, 0, 1);

  ui_region_set_active(old_region, false);
}

// KEH: seg000:0xB452
void ui_refresh_all(void)
{
  word_BEE6 = 0;
  bool_DAE6 = false;

  ui_region_set_active(&full_screen_region, false);
  ui_active_region_clear();
  ui_region_set_active(&message_region, true);
  ui_draw_party_names_header(false);
  level_mark_entity_cells();
  build_map_display();
  draw_day_time();
  ui_region_refresh(&whole_screen);
}

void draw_borders_0()
{
  draw_borders(0);
}

// KEH: seg000:0xB3DF
void sub_B3DF(int arg)
{
  ui_region_set_active(&full_screen_region, true);
  ui_region_set_active(&region_1CEC, true);
  ui_region_set_active(g_anim_entry[0].region, true);

  ui_region_print_centered_str(npc_names[0], 0xB);

  if (arg != -1) { // or 0xFFFF
    ui_draw_party_roster(arg, true, false);
  }

  ui_region_set_active(&region_1CEC, false);
  // B441
  //sub_10772();
  printf("%s: unhandled call to 10772\n", __func__);
  ui_region_refresh(&whole_screen);
}

static struct ui_rect rect_1A66 = { 0x1C, 0x28, 0x68, 0x70 };

// KEH: DSEG:0x1B10
// Defines an area where a popup window will show up to
// give the player notice about something
struct ui_region popup_region = {
  0x08,
  0x06,
  0x1F,
  0x11,
  0x08,
  0x06,
  { 0x20, 0x30, 0x60, 0x60 },
  0x00,
  sub_DAD3,
  0x00,
  &rect_1A66
};

// KEH: DSEG:0x1AD8
struct ui_region message_region = {
  7,
  0x16,
  0x27,
  0x18,
  0x7,
  0x16,
  { 0x1C, 0xB0, 0x84, 0x18 },
  0x00,
  draw_borders_0,
  0x00,
  NULL // rect offset 0x0C-0x12
};
