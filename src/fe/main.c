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

#include "hexdump.h"
#include "fileio.h"
#include "resource.h"
#include "tables.h"
#include "ui.h"
#include "vga.h"

static const int GAME_WIDTH = 320;
static const int GAME_HEIGHT = 200;

// Likely 28 bytes.
struct unknown_302 {
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

// DSEG:0x0076
static uint16_t word_0076 = 0;

// DSEG:0x0274
static const char *data_274 = "KEH.EXE";

// DSEG:0x028F
// Will contain 0x00 or 0xFF
static uint8_t byte_028F = 0;

// DSEG:0x029C
static struct unknown_302 *ptr_029C;

// DSEG:0x029E
static struct ui_rect data_029E = { 0, 0, 160, 200 };

// DSEG:0x02A6
// 92 x 88 rectangle
static struct ui_rect data_02A6 = { 4, 8, 0x30, 0x60 };

// DSEG:0x02E6
static struct unknown_302 unknown_2E6 = {
  0xE0,   // 00
  1,      // 02
  0x26,   // 04
  0x0C,   // 06
  0x0E,   // 08
  1,      // 0A
  { 0x38, 8, 0x64, 0x60 }, // rect offset 0x0C-0x12
  0,      // 14
  0,      // 16
  0,      // 18
  NULL,   // 1A
};

// DSEG:0x0302
static struct unknown_302 unknown_302 = {
  0,     // 00
  0x0E,  // 02
  0x27,  // 04
  0x13,  // 06
  0,     // 08
  0x0E,  // 0A
  { 0, 0x70, 0xA0, 0x30 }, // rect offset 0x0C-0x12
  0,     // 14
  0,     // 16
  0,     // 18
  NULL   // 1A
};

// DSEG:0x31E
static struct unknown_302 unknown_31E = {
  1,   // 00
  1,   // 02
  0xC, // 04
  0xB, // 06
  1,   // 08
  1,   // 0A
  { 4, 8, 0x30, 0x60 }, // Rect offset 0x0C - 0x12
  0,   // 14
  0,   // 16
  0,   // 18
  &data_02A6 // 1A
};

static char *word_33A;
static char *word_33C;
static char *word_33E;

// DSEG:0x0424
static unsigned char video_mode = 2;

// FOD does most of it's work in a large allocated buffer
// DSEG:0x042D
unsigned char *scratch;
static unsigned char *scratch_01; // DSEG:0x029A
static unsigned char *scratch_02; // DSEG:0x0292
static unsigned char *scratch_03; // DSEG:0x0296

// DSEG:0x074F
struct ui_unknown2 data_074F = { 0 };

// DSEG:0x2310
static struct unknown_302 *ptr_2310;

// DSEG:0x2312
static uint16_t word_2312 = 0;

// DSEG:0x2314
static uint16_t word_2314 = 0;

// DSEG:0x2316
static struct resource *gani_res;

// DSEG:0x231A
static uint8_t byte_231A = 0;
// DSEG:0x231C
static uint16_t word_231C = 0;

// DSEG:0x231E - 0x31DE
static unsigned char disk1_bytes[3776];

static uint16_t word_33DE = 0;
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

static void sub_14D5(struct ui_rect *input);
static void sub_14FF(int offset);
static void sub_1548();
static void ui_region_set_active(struct unknown_302 *arg1, bool clear);
static void ui_active_region_clear();
static void sub_159E();
static void plot_font_str(const char *str, int len);
static void plot_font_chr(uint8_t chr_index, int i, int line_num, int base);
static void sub_168E(const char *str, int arg2, int arg3);
static void sub_1778(uint8_t chr_index, int i, int line_num);

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

  // sub_3BFA will expand a 16-bit number to 32 bit, but we don't need
  // to do that on modern architectures.
  fread(font_bytes, 1, size, fp);
  fclose(fp);
}

// seg000:1439
// This is approximately what sub_1439 would do.
void game_mem_alloc()
{
  // Fountain of Dreams does most of its work within this scratch
  // buffer. I'm not sure if it really needs to be this big but that's how much
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

  disk1_bytes[49] = 0; // Number of players in party

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

    word_2314 = 0;

    uint8_t ah = gani_res->bytes[2];
    uint8_t cl = gani_res->bytes[1];

    uint16_t ax = ah << 8 | cl;
    word_2312 = ax; // 0x1089 ?
    byte_231A = 1;
  }
  word_231C = 1;
}

// seg001:0x0F80
static void seg001_sub_0F80(struct resource *res, int res_offset,
    int line_num, int arg2)
{
  unsigned char *es = scratch;

  uint16_t di = get_160_offset(line_num);

  uint16_t ax = arg2;
  ax = ax / 2;
  di += ax; // indentation
  unsigned char *ds = res->bytes;
  uint16_t si = res_offset;

  ax = 0;
  uint8_t img_width = ds[si];
  uint8_t img_height = ds[si+1];

  uint16_t line_width = 160;
  line_width -= img_width;
  si += 4;

  for (int i = 0; i < img_height; i++) {
    // repe movsw
    // copy words from ds:si to es:di cx times, increase si/di
    memcpy(es + di, ds + si, img_width);
    si += img_width;
    di += img_width;

    di += line_width;
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
  uint16_t initial_offset = word_2314;
  uint16_t gani_offset = word_2312;
  uint16_t bx = word_2312;

  gani_offset++;

  uint8_t al = gani_res->bytes[bx];
  uint8_t num_runs = al;

  struct ui_rect *bp02 = &ptr_2310->rect;

  for (int i = 0; i < num_runs; i++) {
    bx = gani_offset;
    gani_offset++;
    al = gani_res->bytes[bx];

    // push ax
    // Takes 5 arguments
    // these come from 31E rectangle struct
    seg001_sub_0FCD(al, initial_offset, gani_res, bp02->x_pos * 2, bp02->y_pos);
  }

  if (arg1 == 0) {
//    sub_14D5(bp02->off);
    printf("%s:0x055A not finished\n", __func__);
    exit(1);
  }

}

static void sub_1631()
{
  struct unknown_302 *si = ptr_029C;

  if (si->data_1A != NULL) {
    sub_14D5(si->data_1A);
  } else {
    sub_14D5(&si->rect);
  }
}

static void sub_0010(const char *arg1, uint16_t arg2)
{
  char output[42];

  snprintf(output, sizeof(output), "%s", arg1);

  struct unknown_302 *bx = ptr_029C;
  uint16_t ax = bx->rect.width;

  uint16_t cx = 2;
  ax = ax >> cx;
  uint16_t si = ax;

  output[si] = '\0';

  uint16_t len = strlen(output);

  ax = bx->rect.width;

  cx = 2;
  ax = ax >> cx;

  ax = ax - len;
  ax = ax >> 1;

  // push ax
  sub_168E(output, ax, arg2);
}

// seg000:0x071B
// 2 arguments
// ex. 0x0033, 0x01CC = "Welcome"
void sub_071B(uint16_t arg1, const char *arg2)
{
  // 2 parameters
  ui_region_set_active(&unknown_31E, true);

  sub_044E(arg1);
  sub_04EA(1);

  sub_0010(arg2, 0xB);
  sub_1631();
}

// seg000:0x07A0
// Draws skills.
static void sub_07A0(int profession)
{
  uint16_t var_4;
  uint16_t var_8;
  uint8_t var_A;
  uint8_t var_C;

  var_8 = 0;
  var_4 = 0;

  char *prof_ptr = (char *)arch_bytes + (profession << 7);
  sub_071B(0x0033, prof_ptr);

  ui_region_set_active(&unknown_2E6, true);

  sub_0010("Skills", 0);
  sub_0010("Active      Passive", 1);

  var_8 = 0;
  var_4 = 0;

  uint8_t al;
  char skill[128];

  for (int i = 0; i < 0x10; i++) {

    // Active skills
    al = prof_ptr[i+0x38];
    var_C = al;
    if (al != 0) {
      uint16_t ax = i * 0x16;
      ax += 294;

      snprintf(skill, sizeof(skill), "%1d %-10.10s", var_C, hds_bytes + ax);
      sub_168E(skill, 0xD, var_8 + 3);
      var_8++;

    }

    // Passive skills
    al = prof_ptr[i + 0x48];
    var_A = al;
    if (al != 0) {
      uint16_t ax = i * 0x16;
      ax += 646;

      snprintf(skill, sizeof(skill), "%1d %-10.10s", var_A, hds_bytes + ax);
      sub_168E(skill, 0xD, var_4 + 3);
      var_4++;
    }
  }

  // 802
}

// Select a profession. The profession is returned to the caller.
// seg000:A65
static int choose_profession()
{
  ui_region_set_active(&unknown_302, false);
  ui_active_region_clear();

  sub_0010("Choose a Profession:", 1);

  // 0xA92
  for (int i = 0; i < 3; i++) {
    char profession[30];

    int ax = i;

    ax = ax << 7; // 128 bytes long
    char *prof_ptr = (char *)arch_bytes + ax;

    // 3C84
    snprintf(profession, sizeof(profession), "%1d)%-13.13s", i + 1, prof_ptr);

    printf("%s\n", profession);
    sub_168E(profession, 4, i + 3);
    if (i >= 2) {
      // 0xB07
      continue;
    } else {
      // 0xACD
      ax = i;
      ax = ax << 7;
      prof_ptr = (char *)arch_offset + ax + 0x180;
      snprintf(profession, sizeof(profession), "%1d)%-13.13s", i + 4, prof_ptr);

      printf("%s\n", profession);

      sub_168E(profession, 0x17, i + 3);
    }
  }

  sub_1631();

  screen_draw(scratch);

  uint8_t key;
  do {
     key = vga_waitkey();
     if (key == 0x1B) {
       // ESC
       return -1;
     }
  } while (key < '1' || key > '5');

  return key - '1';
}

// seg000:0x0B41
static void sub_B41(uint16_t arg1, int profession)
{
  char *prof_ptr = (char *)arch_bytes + (profession << 7);

  uint16_t val1 = (prof_ptr[0x72] << 8) | prof_ptr[0x73];
  uint16_t val2 = (prof_ptr[0x70] << 8) | prof_ptr[0x71];
  sub_16FF(val1, val2);
}

// seg000:0x0B86
static void sub_B86(int arg1, int profession)
{
  char *prof_ptr = (char *)arch_bytes + (profession << 7);

  uint16_t ax = 0x14C;
  ax *= arg1;
  ax += 58; // disk1 offset 0x2358 - 0x231E

  uint16_t var_6 = ax;

  uint16_t bx = var_6;
  uint8_t al;
  // BAC
  // Zero out something.
  for (int i = 0; i < 0xC; i++) {
    bx = var_6;

    disk1_bytes[bx + i + 0x18] = 0;
  }

  // BC4
  for (int i = 0; i < 7; i++) {
    al = prof_ptr[i+0x14];
    bx = var_6;
    disk1_bytes[bx + i + 0x18] = al;
  }
  // BDC
  bx = var_6;
  al = prof_ptr[0x33];
  disk1_bytes[bx + 0x23] = al;

  // BED
  for (int i = 0; i < 0x10; i++) {
    al = prof_ptr[i+0x38];
    bx = var_6;
    disk1_bytes[bx + i + 0x24] = al;

    disk1_bytes[bx + i + 0x0124] = 0;
    al = prof_ptr[i+0x48];
    disk1_bytes[bx + i + 0x34] = al;
    disk1_bytes[bx + i + 0x0134] = 0;
  }

  disk1_bytes[bx + 0x58] = 0;
  disk1_bytes[bx + 0x48] = profession;
  al = prof_ptr[0x7B];
  disk1_bytes[bx + 0x51] = al;
  disk1_bytes[bx + 0x50] = 1;

  word_0076 = 0;
}

// seg000:101B
// Add a character
static int sub_101B()
{
  // Check party size
  if (disk1_bytes[49] == 3) {
    // Don't add any more characters
    return 0;
  }

  // 102C
  int profession = choose_profession();
  if (profession == -1) {
     return 0;
  }

  word_33DE = profession;
  int old_party_size = disk1_bytes[49];
  disk1_bytes[49] = old_party_size + 1;

  // Initialize some data
  sub_B86(old_party_size, profession);

  sub_07A0(profession);

  uint16_t bx = old_party_size;
  bx = bx << 1;

  // push word ptr[bx+3D88]
  // 0x24A4 is NOT CORRECT
  sub_B41(0x24A4, profession);

  printf("Selected profession: %d\n", profession);

  return 0;
}

static void sub_1164(const char *str)
{
  char output[60];

  ui_active_region_clear(); // Clear area

  snprintf(output, sizeof(output), "There's no one to %s!", str);

  sub_0010(output, 2);
  sub_1631();
  screen_draw(scratch);
  vga_waitkey();
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
  video_mode = disk1_bytes[6];
  printf("Video Mode: %d\n", video_mode);

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
  ui_region_set_active(&unknown_302, false);

  do {
    // Only if it's a new game
    sub_071B(0x0033, "Welcome");

    ui_region_set_active(&unknown_2E6, false);
    ui_active_region_clear();
    sub_1631();

    ui_region_set_active(&unknown_302, false);
    ui_active_region_clear();

    sub_0010("Choose a function:", 1);

    sub_168E("A)dd member      R)emove member", 5, 3);
    sub_168E("E)dit member     P)lay the game", 5, 4);
    sub_1631();

    screen_draw(scratch);

    uint8_t key = vga_waitkey();
    // 0x12D1
    printf("Key pressed: 0x%02X\n", key);

    if (key >= 'a' && key <= 'z') {
      key -= 0x20;  // Transform to uppercase
    }

    if (key == 'A') {
      // 0x1347
      sub_101B();
      // jmp loc_1341
    } else if (key <= 'A') {
      // 12EF
    } else {
      // jmp 13C5
      if (key == 'E') {
        if (disk1_bytes[49] == 0) {
          // 138C
          sub_1164("edit");
        } else {
        }
      } else if (key == 'P') {
        // 1391
        if (disk1_bytes[49] == 0) {
          ui_active_region_clear();
          sub_0010("It's tough out there!", 1);
          sub_0010("You should take somebody with you.", 3);
          sub_1631();
          screen_draw(scratch);
          vga_waitkey();
        } else {
          // sub_3E0
          // jmp 121B
        }
        
      } else if (key == 'R') {
        if (disk1_bytes[49] == 0) {
          sub_1164("remove");
        } else {

        }
      }
    }

    //
    // 12EF:
    if (key >= ';') {
      // 12F7
    }
    // jmp 13D7

    // 13D7
#if 0
    if (var_A != 0) {

    }
#endif
    // jmp 1262
  } while (1);

  free(scratch);

  vga_end();

  return 0;
}

static void sub_14B3(struct ui_rect *input)
{
  uint16_t ax = input->x_pos;  // 0
  uint16_t di = input->width;  // A0
  uint16_t cx = input->height; // C8
  uint16_t si = input->y_pos;  // 0

  // sub_05B0:00B0
  ui_sub_00B0(ax, di, cx, si);
}

// seg000:0x14D5
static void sub_14D5(struct ui_rect *input)
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

// Sets the active region and optionally clears it.
// seg000:0x155E
static void ui_region_set_active(struct unknown_302 *arg1, bool clear)
{
  ptr_029C = arg1;

  if (!clear) {
    return;
  }

  arg1->data_24 = 0;
  ui_active_region_clear();

  if (arg1->data_16 != 0) {
    // Call function pointer
    printf("%s:0x1586 unhandled\n", __func__);
  }
}

// Clears the rectangle associated with the active region
// seg000:0x1593
static void ui_active_region_clear()
{
  // Get the rectangle from the active region
  const struct ui_rect *si = &ptr_029C->rect;
  ui_rect_clear(si);
}

// seg000:0x159E
static void sub_159E(const char *str)
{
  word_33A = str;
  word_33C = str;
  word_33E = str;

  // 15AA
  struct unknown_302 *si = ptr_029C;

  // Not exactly correct, there's checking for certain new line characters.
  plot_font_str(str, strlen(str));
#if 0

  char al = *str;
  if (al == '\0') {
    // jmp sub_1602
  }
  if (al == 0x0D) {
    // sub_1602();
    // push di
    // call sub_164F
    // pop di
    // inc di
    // jmp loc_159E
  }
  // 0x15C4
  // push ax
  // cx = dx
  // cx -= word_33A
  uint16_t ax = si->data_08;
  ax += cx;
  if (ax <= si->data_04) {
    // 15D5
    if (al != ' ') {

    }
  }
  // 15E8
#endif

}

// seg000:0x1614
static void plot_font_str(const char *str, int len)
{
  // di = str
  // al = es:di
  // cx = len
  struct unknown_302 *si = ptr_029C;

  for (int i = 0; i < len; i++) {
    plot_font_chr(str[i], si->data_08, si->data_14, si->data_0A);
    si->data_08++;
  }
}

// seg000:0x168E
// 3 arguments
// "Welcome", 2, 0xB
static void sub_168E(const char *str, int arg2, int arg3)
{
  struct unknown_302 *si = ptr_029C;

  if (arg2 != -1) {
    si->data_08 = arg2 + si->data_00;
  }
  // 0x16AC
  if (arg3 != -1) {
    si->data_0A = arg3 + si->data_02;
  }
  // 0x16BC
  sub_159E(str);
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

// seg000:0x17F2
static void plot_font_chr(uint8_t chr_index, int i, int line_num, int base)
{
  uint8_t bl = byte_028F;

  if (bl != 0) {
    printf("%s:0x1839 not finished\n", __func__);
    exit(1);
  }

  uint16_t ax = base;

  ax *= 8;   // << 4
  uint16_t di = ax;
  di += line_num;

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
    memcpy(es, font_si, 4);
    font_si += 4;
    es += 4;

    es += 0x9C; // next line
  }
}

static void sub_32C2(char *data, uint16_t val)
{

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
