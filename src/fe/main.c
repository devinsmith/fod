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

#include "fileio.h"
#include "game.h"
#include "hexdump.h"
#include "random.h"
#include "resource.h"
#include "tables.h"
#include "ui.h"
#include "vga.h"

static const int GAME_WIDTH = 320;
static const int GAME_HEIGHT = 200;

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

// DSEG:0x0076
static uint16_t word_0076 = 0;

// DSEG:0x0274
static const char *data_274 = "KEH.EXE";

// DSEG:0x7A
struct attr_coordinates {
  int x;
  int y;
  const char *str;
};

// DSEG:0x7A
static const struct attr_coordinates attributes[] = {
  { 0x15, 0x02, "ST:" },
  { 0x15, 0x03, "IQ:" },
  { 0x15, 0x04, "DX:" },
  { 0x15, 0x05, "WP:" },
  { 0x1E, 0x02, "AP:" },
  { 0x1E, 0x03, "CH:" },
  { 0x1E, 0x04, "LK:" }
};

// DSEG:0x028F
// Will contain 0x00 or 0xFF
static uint8_t inverse_flag = 0;

// DSEG:0x029C
static struct ui_region *active_region;

// DSEG:0x029E
static struct ui_rect data_029E = { 0, 0, 160, 200 };

// DSEG:0x02A6
// 92 x 88 rectangle
static struct ui_rect data_02A6 = { 4, 8, 0x30, 0x60 };

// DSEG:0x02CA
static struct ui_region unknown_2CA = {
  1,   // 00
  0x15,      // 02
  0x26,   // 04
  0x17,   // 06
  1,   // 08
  0x15,      // 0A
  { 0x4, 0xAC, 0x98, 0x18 }, // rect offset 0x0C-0x12
  0x08,   // 14
  0,   // 16
  0,   // 18
  NULL    // 1A
};

// DSEG:0x02E6
static struct ui_region unknown_2E6 = {
  0x0E,   // 00
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
static struct ui_region unknown_302 = {
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
static struct ui_region unknown_31E = {
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
static struct ui_region *ptr_2310;

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

// DSEG:0x33DE
static uint16_t active_profession = 0;

static uint16_t word_35E4 = 0xFFFF;

// DSEG:0x37E6
static unsigned char *arch_bytes;

// DSEG:0x37E8
static unsigned char hds_bytes[1180];

// DSEG:0x3C84
static unsigned char *arch_offset;
// DSEG:0x3C86
static unsigned char *font_bytes;

// DSEG:0x3D88 (offsets to memory in disk1 by character)
// Each character is 332 bytes apart.
static uint16_t disk1_bytes_offsets[] = {
  58,   /* 0x2358 - 0x231E */
  390,  /* 0x24A4 - 0x231E */
  722,  /* 0x25F0 - 0x231E */
  1054, /* 0x273C - 0x231E */
  1386  /* 0x2888 - 0x231E */
};

// DSEG:0x3E66
static struct resource *border_res;

static void sub_14B3(struct ui_rect const *input);
static void sub_14D5(struct ui_rect *input);
static void sub_14FF(int offset);
static void sub_1548();
static void ui_region_set_active(struct ui_region *arg1, bool clear);
static void ui_active_region_clear();
static void sub_159E();
static void plot_font_str(const char *str, int len);
static void plot_font_chr(uint8_t chr_index, int i, int line_num, int base);
static void ui_region_print_str(const char *str, int arg2, int arg3);
static void sub_1778(uint8_t chr_index, int i, int line_num);
static void sub_3290(int char_num, const char *name);

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
  struct ui_region *si = active_region;

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

  struct ui_region *bx = active_region;
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
  ui_region_print_str(output, ax, arg2);
}

// seg000:0x0618
static void sub_618(int char_index, int arg2, int arg3)
{
  struct ui_region *original_region = active_region;

  ui_region_set_active(&unknown_2CA, false);
  ui_active_region_clear();

  uint8_t party_size = g_game_state.party_size; // party size.
  // Display characters.
  for (int i = 0; i < party_size; i++) {
    char player_id[16];
    snprintf(player_id, sizeof(player_id), "F%1d>", i + 1);
    if (arg2 != 0 && char_index == i) {
      inverse_flag = 0xFF;
    }

    ui_region_print_str(player_id, 0, i);
    inverse_flag = 0;

    uint8_t al = g_game_state.players[i].profession; // profession
    char name_prof[64];

    snprintf(name_prof, sizeof(name_prof), "%-16.16s - %-16.16s",
             g_game_state.players[i].name, arch_offset + (al * 128));

    ui_region_print_str(name_prof, 3, i);
  }

  if (arg3 == 0) {
    sub_14B3(&active_region->rect);
  } else {
    sub_1631();
  }


  // 6B8

  // 0x6DC
  // Restore original region
  ui_region_set_active(original_region, false);
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

// seg000:0x0751
// Draws CON:
static void draw_con(int chr_index)
{
  ui_region_set_active(&unknown_2E6, false);

  char con_value[32];

  //uint8_t con = disk1_bytes[char_offset + 0x44];
  uint8_t con = g_game_state.players[chr_index].condition;
  snprintf(con_value, sizeof(con_value), "Con:%d", con);

  ui_region_print_str(con_value, 6, 0xB);

  sub_1631();

  ui_region_set_active(&unknown_2CA, false);
}

// seg000:0x07A0
// Draws the skills associated with the active profession.
static void draw_profession_skills()
{
  uint16_t var_4;
  uint16_t var_8;
  uint8_t var_A;
  uint8_t var_C;

  int profession = active_profession;

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
      ui_region_print_str(skill, 0, var_8 + 3);
      var_8++;

    }

    // Passive skills
    al = prof_ptr[i + 0x48];
    var_A = al;
    if (al != 0) {
      uint16_t ax = i * 0x16;
      ax += 646;

      snprintf(skill, sizeof(skill), "%1d %-10.10s", var_A, hds_bytes + ax);
      ui_region_print_str(skill, 0xD, var_4 + 3);
      var_4++;
    }
  }

  // 802
}

// seg000:089E
static void show_attribute_points(int char_number)
{
  ui_region_set_active(&unknown_302, false);
  uint8_t al = g_game_state.players[char_number].attribute_points;

  char attr_points[16];
  snprintf(attr_points, sizeof(attr_points), "%2.2d->", al);
  unknown_302.data_14 = 8;
  ui_region_print_str(attr_points, 0x24, 0);
  unknown_302.data_14 = 0;
  sub_1631();

  ui_region_set_active(&unknown_2CA, false);
}

// seg000:0x8F5
static void sub_8F5(int char_number, int attr_num)
{
  int var_2 = 0;

  ui_region_set_active(&unknown_302, false);
  ui_active_region_clear();

  // 91E
  uint16_t ax = var_2;

  // Display 5 professions on the left.
  for (int i = 0; i < 5; i++) {
    ax = i;
    ax = ax << 7; // 128 bytes long

    char *prof_ptr = (char *)arch_bytes + ax;

    char profession[30];
    snprintf(profession, sizeof(profession), "%1d)%-13.13s", i + 1, prof_ptr);

    ui_region_print_str(profession, 0, i + 1);
  }

  // 959

  unknown_302.data_14 = 8;
  ui_region_print_str("Attribute Pts: <-", 0x13, 0);
  unknown_302.data_14 = 0;

  for (int i = 0; i < 7; i++) {
    char attr_str[30];
    char attr_val[20];

    snprintf(attr_str, sizeof(attr_str), "%3s", attributes[i].str);
    ui_region_print_str(attr_str, attributes[i].x, attributes[i].y);

    uint8_t al = g_game_state.players[char_number].attributes[i];

    snprintf(attr_val, sizeof(attr_val), "%2u", al);

    if (i == attr_num) {
      inverse_flag = 0xFF; // Makes it inverse (or bold)
    }
    ui_region_print_str(attr_val, attributes[i].x + 3, attributes[i].y);
    inverse_flag = 0;
  }

  // AOC
  ui_region_print_str("Sex:", 0x1D, 5);
  if (attr_num == 7) {
    inverse_flag = 0xFF; // Makes it inverse (or bold)
  }

  // 5
  // 0x22
  uint8_t gender = g_game_state.players[char_number].gender;
  const char *gender_val = "Male";
  if (gender != 1) {
    gender_val = "Female";
  }

  ui_region_print_str(gender_val, 0x22, 5);
  inverse_flag = 0;

  draw_con(char_number);

  show_attribute_points(char_number);
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
    ui_region_print_str(profession, 4, i + 3);
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

      ui_region_print_str(profession, 0x17, i + 3);
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
static void set_con_val(uint16_t arg1, int profession)
{
  char *prof_ptr = (char *)arch_bytes + (profession << 7);

  uint16_t val1 = (prof_ptr[0x71] << 8) | prof_ptr[0x70];
  uint16_t val2 = (prof_ptr[0x73] << 8) | prof_ptr[0x72];
  uint8_t rnd_val = game_random_range(val1, val2);

  // Set CON to rnd_val??
  // Why is CON in 3 places?
  g_game_state.players[arg1].condition = rnd_val;
  g_game_state.players[arg1].max_condition = rnd_val;
//  disk1_bytes[arg1 + 0x46] = rnd_val;
//  disk1_bytes[arg1 + 0x44] = rnd_val;
//  disk1_bytes[arg1 + 0x122] = rnd_val;
}

// seg000:0x0B86
static void init_player(int arg1, int profession)
{
  const char *prof_ptr = (char *)arch_bytes + (profession << 7);

  uint8_t al;
  // BAC
  // Zero out attributes.
  for (int i = 0; i < 0xB; i++) {
    g_game_state.players[arg1].attributes[i] = 0;
  }
  g_game_state.players[arg1].attribute_points = 0;

  // BC4
  // Copy default attributes from profession.
  for (int i = 0; i < 7; i++) {
    al = prof_ptr[i+0x14];
    g_game_state.players[arg1].attributes[i] = al;
  }
  // BDC
  al = prof_ptr[0x33];
  g_game_state.players[arg1].attribute_points = al;

  // BED
  // Copy skills (active and passive) from profession.
  for (int i = 0; i < 16; i++) {
    al = prof_ptr[i+0x38];
    g_game_state.players[arg1].active_skills[i] = al;
    //disk1_bytes[bx + i + 0x0124] = 0; ?
    al = prof_ptr[i+0x48];
    g_game_state.players[arg1].passive_skills[i] = al;
    //disk1_bytes[bx + i + 0x0134] = 0; ?
  }

  g_game_state.players[arg1].affliction = 0;
  g_game_state.players[arg1].profession = profession;
  al = prof_ptr[0x7B];
  g_game_state.players[arg1].unknown_51 = al;
  g_game_state.players[arg1].gender = 1;


  word_0076 = 0;
}

// seg000:0x0C58
static void sub_C58(int current_char, int stat_id)
{
  int profession = active_profession;

  char *prof_ptr = (char *)arch_offset + (profession * 128);

  if (stat_id < 7) {
    // C80
    if (g_game_state.players[current_char].attribute_points == 0) {
      // No attribute points to distribute
      return;
    }

    uint8_t current_stat = g_game_state.players[current_char].attributes[stat_id];
    if (current_stat > prof_ptr[stat_id + 0x20]) {
      // Don't allow incrementing stat above professions max
      return;
    }
    g_game_state.players[current_char].attributes[stat_id] = current_stat + 1;
    g_game_state.players[current_char].attribute_points--;
  } else {
    // CA7
    uint8_t current_gender = g_game_state.players[current_char].gender;
    if (current_gender == 1) {
      current_gender = 2;
      word_0076 = 5;
    } else {
      current_gender = 1;
      word_0076 = 0;
    }
    g_game_state.players[current_char].gender = current_gender;
    draw_profession_skills();
  }

  // CD3
  sub_8F5(current_char, stat_id);
}

// seg000:0x0CE5
static void sub_0CE5(int current_char, int stat_id)
{
  int profession = active_profession;

  char *prof_ptr = (char *)arch_offset + (profession * 128);

  if (stat_id < 7) {
    // DOD
    if (g_game_state.players[current_char].attribute_points >= prof_ptr[0x33]) {
      // Don't allow incrementing attribute points above the max allowed for a given profession.
      return;
    }

    uint8_t current_stat = g_game_state.players[current_char].attributes[stat_id];
    if (current_stat <= prof_ptr[stat_id + 0x14]) {
      // Don't allow decrementing stat below professions min
      return;
    }

    g_game_state.players[current_char].attributes[stat_id] = current_stat - 1;
    g_game_state.players[current_char].attribute_points++;
  } else {
    uint8_t current_gender = g_game_state.players[current_char].gender;
    if (current_gender == 1) {
      current_gender = 2;
      word_0076 = 5;
    } else {
      current_gender = 1;
      word_0076 = 0;
    }
    g_game_state.players[current_char].gender = current_gender;
    draw_profession_skills();
  }

  sub_8F5(current_char, stat_id);
}

// seg000:0x0D75
static int sub_D75(int current_char)
{
  int var_A = 0;

  sub_8F5(current_char, 0);
  sub_618(current_char, 1, 1);

  size_t namelen = strlen(g_game_state.players[current_char].name);
  uint8_t name_end = 0x1B;
  g_game_state.players[current_char].name[namelen] = (char)name_end;
  g_game_state.players[current_char].name[namelen + 1] = 0x00;

  bool dirty = false;

  unsigned int last_time = sys_ticks();
  while (1) {
    if (dirty) {
      sub_1631();
      screen_draw(scratch);
      dirty = false;
    }

    // Check keyboard input buffer
    uint8_t key = vga_pollkey(220);
    if (key != 0 && key != 0xFE) {
      // a key has been pressed.

      // Special cases for different keys
      if (key == 0xFF) {
        // Escape key
        g_game_state.players[current_char].name[namelen] = 0x00;
        sub_618(current_char, 1, 1);
        return current_char;
      } else if (key == 0xFD) {
        // Up arrow
        var_A += 7;
        var_A = var_A % 8;
        sub_8F5(current_char, var_A);
        dirty = true;
      } else if (key == 0xFA) {
        // Right arrow
        sub_C58(current_char, var_A);
        dirty = true;
      } else if (key == 0xFB) {
        // Left arrow
        sub_0CE5(current_char, var_A);
        dirty = true;
      } else if (key == 0xFC) {
        // Down arrow
        var_A++;
        var_A = var_A % 8;
        sub_8F5(current_char, var_A);
        dirty = true;
      } else if (key >= 0x3B && key <= 0x3D) {
        // Switch player with F1, F2, F3
        int key_num = key - 0x3B;

        if (key_num < g_game_state.party_size) {
          int old_char = current_char;

          active_profession = g_game_state.players[key_num].profession;
          current_char = key_num;
          word_0076 = 0;
          draw_profession_skills();
          sub_8F5(current_char, var_A);

          g_game_state.players[old_char].name[namelen] = 0x00;
          sub_618(current_char, 1, 1);

          namelen = strlen(g_game_state.players[current_char].name);
          name_end = 0x1B;
          g_game_state.players[current_char].name[namelen] = (char)name_end;
          g_game_state.players[current_char].name[namelen + 1] = 0x00;
          dirty = true;
        }
      } else if (key == 0x08) {
        if (namelen > 0) {
          // Backspace
          name_end = 0x1B;
          g_game_state.players[current_char].name[namelen] = ' ';
          g_game_state.players[current_char].name[namelen - 1] = (char)0x1B;

          ui_region_print_str(g_game_state.players[current_char].name, 3, current_char);
          dirty = true;
          namelen--;
        }
      } else if (key >= 0x31 && key <= 0x35) {
        // Switch profession (or re-roll) with 1, 2, 3, 4, 5
        int new_prof = key - 0x31;
        if (new_prof != active_profession) {
          // Change profession
          active_profession = new_prof;

          init_player(current_char, active_profession);
          draw_profession_skills();
        }

        // Reroll stats
        set_con_val(current_char, active_profession);
        sub_8F5(current_char, var_A);

        sub_618(current_char, 1, 1);
        ui_region_set_active(&unknown_2CA, false);
        dirty = true;
      } else {
        if (namelen < 12) {
          g_game_state.players[current_char].name[namelen] = (char)key;

          namelen++;
          dirty = true;
          ui_region_print_str(g_game_state.players[current_char].name, 3, current_char);
        }
      }
    }

    unsigned int now_time = sys_ticks();
    if (now_time - last_time >= 300) {
      last_time = now_time;

      if (name_end == 0x1B) {
        name_end = 0x20;
      } else {
        name_end = 0x1B;
      }

      g_game_state.players[current_char].name[namelen] = (char)name_end;
      ui_region_print_str(g_game_state.players[current_char].name, 3, current_char);

      sub_1631();
      screen_draw(scratch);
    }

    if (!dirty) {
      sys_delay(10);
    }
  }
}

// seg000:101B
// Add a character
static int sub_101B()
{
  // Check party size
  if (g_game_state.party_size == 3) {
    // Don't add any more characters
    return 0;
  }

  // 102C
  int profession = choose_profession();
  if (profession == -1) {
     return 0;
  }

  active_profession = profession;
  uint8_t current_char = g_game_state.party_size;
  g_game_state.party_size = current_char + 1;

  // Initialize some data
  init_player(current_char, profession);

  draw_profession_skills();

  set_con_val(current_char, profession);
  sub_3290(current_char, "Ojnab Bob");
  return sub_D75(current_char);
}

// seg000:1083
// Remove a character
static int sub_1083(int char_num)
{
  ui_region_set_active(&unknown_302, false);
  ui_active_region_clear();

  sub_0010("Really remove this member?", 1);
  sub_0010("Y)es  N)o", 3);

  sub_1631();
  screen_draw(scratch);
  uint8_t key = vga_waitkey();
  // 0x10C9
  if (key >= 'a' && key <= 'z') {
    key -= 0x20;  // Transform to uppercase
  }

  if (key == 'Y') {

    if (char_num < g_game_state.party_size) {
      // need to move characters up.
      //
      for (int start = char_num; start < (g_game_state.party_size - 1); start++) {
        memcpy(&g_game_state.players[start], &g_game_state.players[start + 1], sizeof(struct player_rec));
      }
    }

    g_game_state.party_size--;
    g_game_state.players[g_game_state.party_size].name[0] = '\0';
    char_num--;

    sub_618(char_num, 1, 1);
  }

  ui_region_set_active(&unknown_2CA, false);
  return char_num;
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

  bool saved_game = load_game_state();
  printf("Saved game: %s\n", saved_game ? "true" : "false");
  video_mode = g_game_state.video_mode;
  printf("Video Mode: %d\n", g_game_state.video_mode);

  sub_02E5(saved_game);

  // Register VGA driver.
  video_setup();

  if (vga_initialize(GAME_WIDTH, GAME_HEIGHT) != 0) {
    return 1;
  }

  game_mem_alloc();

  word_231C = 0;
  word_35E4 = 0xFFFF;
  active_profession = 0;

  do_title();

  sub_1548();

  // 14D5 and screen_draw are similar
  sub_14D5(&data_029E);
  screen_draw(scratch);

//  int local_val = 0;
  ui_region_set_active(&unknown_302, false);

  int current_char = 0;

  do {
    // Only if it's a new game
    sub_071B(0x0033, "Welcome");

    ui_region_set_active(&unknown_2E6, false);
    ui_active_region_clear();
    sub_1631();

    ui_region_set_active(&unknown_302, false);
    ui_active_region_clear();

    sub_0010("Choose a function:", 1);

    ui_region_print_str("A)dd member      R)emove member", 5, 3);
    ui_region_print_str("E)dit member     P)lay the game", 5, 4);
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
      current_char = sub_101B();
      // jmp loc_1341
    } else if (key <= 'A') {
      // 12EF
    } else {
      // jmp 13C5
      if (key == 'E') {
        if (g_game_state.party_size == 0) {
          // 138C
          sub_1164("edit");
        } else {
          // 136E
          active_profession = g_game_state.players[current_char].profession;
          uint8_t current_gender = g_game_state.players[current_char].gender;
          if (current_gender == 1) {
            word_0076 = 5;
          } else {
            word_0076 = 0;
          }
          draw_profession_skills();
          current_char = sub_D75(current_char);
        }
      } else if (key == 'P') {
        // 1391
        if (g_game_state.party_size == 0) {
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
        if (g_game_state.party_size == 0) {
          sub_1164("remove");
        } else {
          // 1353
          current_char = sub_1083(current_char);
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

static void sub_14B3(const struct ui_rect *input)
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
static void ui_region_set_active(struct ui_region *arg1, bool clear)
{
  active_region = arg1;

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
  const struct ui_rect *si = &active_region->rect;
  ui_rect_clear(si);
}

// seg000:0x159E
static void sub_159E(const char *str)
{
  word_33A = str;
  word_33C = str;
  word_33E = str;

  // 15AA
  struct ui_region *si = active_region;

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
  struct ui_region *si = active_region;

  for (int i = 0; i < len; i++) {
    plot_font_chr(str[i], si->data_08, si->data_14, si->data_0A);
    si->data_08++;
  }
}

// seg000:0x168E
// Prints a string within a region, at (x,y) coordinates
// "Welcome", 2, 0xB
static void ui_region_print_str(const char *str, int arg2, int arg3)
{
  struct ui_region *si = active_region;

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
  uint8_t bl = inverse_flag;
  bool do_xor = false;

  if (bl != 0) {
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

static void sub_3290(int char_num, const char *name)
{
  // XXX: This might not be correct.
  strncpy(g_game_state.players[char_num].name, name, 12);
  g_game_state.players[char_num].name[12] = '\0';
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
