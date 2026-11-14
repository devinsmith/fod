/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2025-2026 Devin Smith <devin@devinsmith.net>
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "anim.h"
#include "cmdmenu.h"
#include "compress.h"
#include "fileio.h"
#include "game.h"
#include "hexdump.h"
#include "level.h"
#include "npc.h"
#include "platform.h"
#include "random.h"
#include "resource.h"
#include "script.h"
#include "tables.h"
#include "ui.h"

static const int GAME_WIDTH = 320;
static const int GAME_HEIGHT = 200;

struct level_meta {
  const char *disk_name;
  const char *disk_id;
  const char *map_file;
  const char *script_file;
  const char *ani_file;
};

// KEH: DSEG:0x0042
static struct level_meta k1_level = {
  "Disk 1",
  "Disk1",
  "kmap",
  "kscr",
  "kani"
};

// KEH: DSEG:0x005E
static struct level_meta k2_level = {
  "Disk 2",
  "Disk2",
  "kmap",
  "kscr",
  "kani"
};

// KEH: DSEG:0x007A
static struct level_meta f3_level = {
  "Disk 3",
  "Disk3",
  "fmap",
  "fscr",
  "fani"
};

// KEH: DSEG:0x0096
static struct level_meta d4_level = {
  "Disk 4",
  "Disk4",
  "dmap",
  "dscr",
  "dani"
};

static struct level_meta *levels[] = {
  &k1_level,
  &k2_level,
  &f3_level,
  &d4_level
};

// DSEG:0x0076
static uint16_t word_0076 = 0;

// DSEG:0x7A
struct attr_coordinates {
  int x;
  int y;
  const char *str;
};

struct unknown_C6A0 {
  uint16_t count;
};

struct unknown_C6A0 data_C6A0 = {
  0
};

// KEH: DSEG:0x00E6 - pointer to encounter placement array (12-byte entries).
// The array layout: [2-byte count][12-byte entry 0][12-byte entry 1]...
// In the disassembly this points to 0xC6A0 which always has a value of 0.
static struct unknown_C6A0 *word_00E6 = &data_C6A0;

// KEH: DSEG:0x00EA
static uint8_t byte_00EA = 0;

// KEH: DSEG:0x00EC
// Cached map table index (from map_ids look-up)
static uint16_t word_00EC = 0;

// KEH: DSEG:0x0125 - Map table indicies (size unknown, at least 28 (0x1C) bytes ?)
static const uint8_t map_ids[] = {
  0, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3,
  2, 3, 2, 3, 3, 3, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 2
};

// KEH: DSEG: 0xCDAC
static const char *level_phys_label = NULL;

// KEH: DSEG:D1E3
static uint8_t byte_D1E3;

// KEH: DSEG:D1E8
static uint8_t byte_D1E8;

// KEH: DSEG:0xDBC6
static uint8_t byte_DBC6;

// KEH: DSEG:D9DA
static unsigned char *ptr_D9DA = NULL;

// KEH: DSEG: 0xD9D8
static uint16_t word_D9B8 = 0;

// KEH: DSEG: 0xD9D4
static uint16_t word_D9D4 = 0;

// KEH: DSEG: 0xD9BC
static const char *level_scr_file = NULL;

static unsigned char level_scr_bytes[256];

// KEH: DSEG: 0xDBD2
unsigned char *scr_decompressed = NULL;

// KEH: DSEG: 0xDBD6
static uint16_t word_DBD6;

// KEH: DSEG: 0xDDDA
static uint16_t word_DDDA;

// KEH: DSEG: 0xDDDC
uint16_t g_anim_count; // how many anims wanted (0..3)

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

// FOD: DSEG:0x029C
// KEH: DSEG:0x1A4C
extern struct ui_region *active_region;

// DSEG:0x029E
static struct ui_rect data_029E = { 0, 0, 160, 200 };

// DSEG:0x02A6
// 92 x 88 rectangle
static struct ui_rect data_02A6 = { 4, 8, 0x30, 0x60 };

// DSEG:0x02CA
static struct ui_region unknown_2CA = {
  1,    // 00
  0x15, // 02
  0x26, // 04
  0x17, // 06
  1,    // 08
  0x15, // 0A
  { 0x4, 0xAC, 0x98, 0x18 }, // rect offset 0x0C-0x12
  0x08, // 14
  0,    // 16
  0,    // 18
  NULL  // 1A
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
  NULL,      // 16
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
  NULL,     // 16
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
  NULL,   // 16
  0,   // 18
  &data_02A6 // 1A
};

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

// DSEG:0x3C84
unsigned char *arch_offset;

// KEH DSEG:0xBEE8
static uint16_t word_BEE8 = 0;

// KEH DSEG:0x1D126
static uint16_t word_1D126 = 0;

// KEH DSEG:0xCFC4
static uint16_t word_CFC4 = 0;

// KEH DSEG:0x1EC04
static uint32_t dword_1EC04 = 0;
// KEH DSEG:0x1E4BE
static uint8_t byte_1E4BE = 0;
// KEH DSEG:0x1F01A
static uint16_t word_1F01A = 0;

// KEH DSEG:0x1D138
static uint16_t word_1D138 = 0;
// KEH DSEG:0x1D13A
static uint16_t word_1D13A = 0;
// KEH DSEG:0x1EC1A - far pointer to encounter/item data (array of 6-byte entries)
static unsigned char *ptr_1EC1A = NULL;

// KEH DSEG:0x1E41A - base offset for item table lookups
static uint16_t word_1E41A = 0;

// KEH DSEG:0x2518 - table of pointers to player/entity data (indexed by player num)
static uint16_t player_data_table[6];
// KEH DSEG:0x1ACE - table of encounter sprite/layout values
static uint16_t table_1ACE[6];
// KEH DSEG:0x3240 - table mapping encounter result to dword_1EC1A index
static uint16_t table_3240[100];

// KEH DSEG:0xD9E6 - sprintf buffer (probably not actually 256 bytes) but
// it isn't clear what the actual size was.
static char str_buffer[256];

// KEH DSEG:0xD1D8
uint16_t word_D1D8 = 0;

static void sub_1548();
static void sub_3290(int char_num, const char *name);
static void sub_39FE(int arg1, int arg2);

static void sub_27CC(int arg0);
static int sub_D5BA(void);
static int sub_4F1A(int arg0);
static void sub_5691(int arg0, int arg1);
static void sub_138D(int arg0);

static void sub_1834(uint16_t cell, int arg2, int npc_num);
static void set_party_position(int x, int y);
static void sub_12E9(void);
static void sub_1339(void);
static void sub_7FA8(int x, int y);
static void sub_113F(int arg0);
static void sub_92D5(void);
static void sub_2B70(int arg0);
static void sub_2B93(int arg0);
static void sub_109D(int arg0);
static void sub_8FFA(unsigned char *entity_ptr, int val);
static void sub_7854(void);
static void sub_1766(void);
static void sub_E674(void);
static void sub_10720(void);
static void sub_8827(void);
static void print_movement_msg(int msg_index);
static int sub_54D6(void);
static void sub_EF7(void);
static void sub_E29(unsigned char *arg1, unsigned char *arg2,
                    uint8_t arg3, uint8_t arg4, uint16_t arg5, uint8_t arg6);
static void sub_E2B8(uint16_t arg0, int arg1);
static int16_t sub_7ED0(int x, int y);
static int16_t sub_7F34(uint16_t arg0, int x, int y);
static void sub_7B02(struct player_rec *player, uint16_t arg1, uint16_t arg2);
static int sub_6D6B(uint16_t arg0);
static void sub_6D96(uint16_t arg0);
static void sub_D6AA(void *ptr, uint16_t arg1);
static void sub_22(int arg);

static void do_title()
{
  struct resource *title_res = resource_load(RESOURCE_TITLE, 0, 0);

  screen_draw(title_res->bytes);

  vga_waitkey();

  resource_release(title_res);
}

// seg000:1439
// This is approximately what sub_1439 would do.
void game_mem_alloc()
{
  // Fountain of Dreams does most of its work within this scratch
  // buffer. I'm not sure if it really needs to be this big, but that's how much
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

  ui_load_fonts();

  memset(scratch, 0, 32000);
}

// seg000:02E5
static void sub_02E5(bool saved_game)
{
  if (!ui_load_res()) {
    // Count on ui_load_res to display an error as it may load
    // multiple resources.
    exit(1);
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

  hds_res = resource_load(RESOURCE_HDSPCT, 0, 0);
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

// seg000:0x03E0
static void save_players()
{
  g_game_state.saved_game = 1;
  g_game_state.do_init = 1;

  for (int i = 0; i < g_game_state.party_size; i++) {
    g_game_state.players[i].unknown_49 = -6;

    // 0x2ACE ++
    // set to 0x01
  }

  save_game_state();
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
      ui_set_inverse(true);
    }

    ui_region_print_str(player_id, 0, i);
    ui_set_inverse(false);

    uint8_t al = g_game_state.players[i].profession; // profession
    char name_prof[64];

    snprintf(name_prof, sizeof(name_prof), "%-16.16s - %-16.16s",
             g_game_state.players[i].name, arch_offset + (al * 128));

    ui_region_print_str(name_prof, 3, i);
  }

  if (arg3 == 0) {
    ui_region_queue_rect(&active_region->rect);
  } else {
    ui_region_refresh_active();
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
  ui_region_refresh_active();
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
  ui_region_refresh_active();
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

      snprintf(skill, sizeof(skill), "%1d %-10.10s", var_C, hds_res->bytes + ax);
      ui_region_print_str(skill, 0, var_8 + 3);
      var_8++;

    }

    // Passive skills
    al = prof_ptr[i + 0x48];
    var_A = al;
    if (al != 0) {
      uint16_t ax = i * 0x16;
      ax += 646;

      snprintf(skill, sizeof(skill), "%1d %-10.10s", var_A, hds_res->bytes + ax);
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
  unknown_302.line_number = 8;
  ui_region_print_str(attr_points, 0x24, 0);
  unknown_302.line_number = 0;
  ui_region_refresh_active();

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

  unknown_302.line_number = 8;
  ui_region_print_str("Attribute Pts: <-", 0x13, 0);
  unknown_302.line_number = 0;

  for (int i = 0; i < 7; i++) {
    char attr_str[30];
    char attr_val[20];

    snprintf(attr_str, sizeof(attr_str), "%3s", attributes[i].str);
    ui_region_print_str(attr_str, attributes[i].x, attributes[i].y);

    uint8_t al = g_game_state.players[char_number].attributes[i];

    snprintf(attr_val, sizeof(attr_val), "%2u", al);

    if (i == attr_num) {
      ui_set_inverse(true); // Makes it inverse (or bold)
    }
    ui_region_print_str(attr_val, attributes[i].x + 3, attributes[i].y);
    ui_set_inverse(false);
  }

  // AOC
  ui_region_print_str("Sex:", 0x1D, 5);
  if (attr_num == 7) {
    ui_set_inverse(true); // Makes it inverse (or bold)
  }

  // 5
  // 0x22
  uint8_t gender = g_game_state.players[char_number].gender;
  const char *gender_val = "Male";
  if (gender != 1) {
    gender_val = "Female";
  }

  ui_region_print_str(gender_val, 0x22, 5);
  ui_set_inverse(false);

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

  ui_region_refresh_active();

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
  g_game_state.players[arg1].unknown_11 = al;
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
      ui_region_refresh_active();
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

      ui_region_refresh_active();
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

  ui_region_refresh_active();
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
  ui_region_refresh_active();
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
  platform_setup();

  if (vga_initialize(GAME_WIDTH, GAME_HEIGHT) != 0) {
    return 1;
  }

  game_mem_alloc();

  word_231C = 0;
  word_35E4 = 0xFFFF;
  active_profession = 0;

  do_title();
  if (!saved_game) {
    sub_1548();

    // 14D5 and screen_draw are similar
    ui_region_refresh(&data_029E);

    ui_region_set_active(&unknown_302, false);

    int current_char = 0;

    do {
      // Only if it's a new game
      sub_071B(0x0033, "Welcome");

      ui_region_set_active(&unknown_2E6, false);
      ui_active_region_clear();
      ui_region_refresh_active();

      ui_region_set_active(&unknown_302, false);
      ui_active_region_clear();

      sub_0010("Choose a function:", 1);

      ui_region_print_str("A)dd member      R)emove member", 5, 3);
      ui_region_print_str("E)dit member     P)lay the game", 5, 4);
      ui_region_refresh_active();

      uint8_t key = vga_waitkey();
      // 0x12D1
      printf("Key pressed: 0x%02X\n", key);

      if (key >= 'a' && key <= 'z') {
        key -= 0x20;  // Transform to uppercase
      }

      if (key == 'A') {
        // 0x1347
        current_char = sub_101B();
      } else if (key == 'E') {
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
          ui_region_refresh_active();
          vga_waitkey();
        } else {
          //save_players();
          break;

          // Not done
        }
      } else if (key == 'R') {
        if (g_game_state.party_size == 0) {
          sub_1164("remove");
        } else {
          // 1353
          current_char = sub_1083(current_char);
        }
      } else if (key >= 0x3B && key <= 0x3D) {
        // Switch player with F1, F2, F3
        int key_num = key - 0x3B;

        if (key_num < g_game_state.party_size) {
          current_char = key_num;
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
      }
    } while (1);
  }

  printf("Game start!\n");
  ui_region_refresh(&data_029E);

  // 140E
  //
  sub_39FE(0x274, 0x26C);

  free(scratch);

  vga_end();

  return 0;
}


// seg000:0x1548
static void sub_1548()
{
  draw_borders(5000);
}

static void sub_3290(int char_num, const char *name)
{
  struct player_rec *p = &g_game_state.players[char_num];

  strncpy(p->name, name, sizeof(p->name) - 1);
  p->name[sizeof(p->name) - 1] = '\0';
}

// KEH: seg000:0xD9CF
static void show_welcome_message()
{
  active_region = &message_region;

  draw_borders_0();

  // Text is at KEH:DSEG:0x1835
  print_wrapped_text("Welcome to the beautiful island of Florida!\r");
}

static void sub_104C4(int arg)
{
  if (word_00E6 == NULL) {
    return;
  }

  uint16_t count = word_00E6->count;

  if (count > 0) {
    printf("%s: unimplemented, count = %d\n", __func__, count);
  }
}

// KEH: seg000:0x8C
// Loads and initializes map/area data for a given area index (arg0).
// Caches the area index and map index to avoid redundant file loads.
// Parses the map header to compute pointers for encounters, items, etc.
static void sub_8C(int arg0)
{
  if (arg0 == byte_00EA) {
    return;
  }

  save_level_map_if_dirty(byte_00EA);
  byte_00EA = arg0;

  uint16_t map_index = map_ids[arg0];
  printf("%s: Loading map: %d\n", __func__, map_index);

  // If map index changed, load the map data
  if (word_00EC != map_index) {
    word_00EC = map_index;

    level_map_file = levels[map_index]->map_file;
    level_scr_file = levels[map_index]->script_file;
    level_ani_file = levels[map_index]->ani_file;
    level_phys_label = levels[map_index]->disk_id;

    read_file(level_map_file, level_map_offsets, 112);
    read_file(level_scr_file, level_scr_bytes, 112);
    read_file(level_ani_file, level_ani_bytes, 212);
  }

  // Load decompressed map data from level_map_file into level_map_large
  // arg0_byte - 1 is the sub-index within the level_map_file file
  indexed_record_rw(level_map_file, level_map_large, arg0 - 1, level_map_offsets, false);
  printf("Level map large:\n");
  hexdump(level_map_large, 32);

  sub_104C4(arg0);

  // Parse header relative offsets into absolute pointers within level_map_large.
  // The map header at offset 0x404 has 5 uint16_t offsets, each added to the
  // base of level_map_large to form a data pointer.
  unsigned char *base = level_map_large;

  // [0x404] encounter/event data inside level_map_large
  word_D9B8 = *(uint16_t *)(base + 0x404);

  // [0x406] used by sub_D8CD: array of 0x68-byte npc entries
  npc_base = level_map_large + *(uint16_t *)(base + 0x406);

  // [0x408]
  word_D9D4 = *(uint16_t *)(base + 0x408);

  // [0x40A]
  ptr_D1DE = level_map_large + *(uint16_t *)(base + 0x40A);

  // [0x40C] encounter/item data
  ptr_D9DA = level_map_large + *(uint16_t *)(base + 0x40C);

  // Compute level_map_player_pos from party position
  {
    uint16_t map_width = level_map->map_width;
    uint16_t tile_index = (uint16_t)(g_game_state.y_pos * map_width + g_game_state.x_pos);
    level_map_player_pos = &level_map->cells[tile_index];
  }

  sub_22(arg0);
}

// Displays a boundary/blocked-movement message by index (1-based).
// arg_0 == 0 does nothing.
static const char *boundary_messages[] = {
  "\rYou see miles of impassable ocean and you decide to turn back.\r",
  "\rThere are roads around here somewhere!\r",
  "\rTry using the road, friend.\r",
  "\r            hic!\r",
};

// KEH: seg000:0xE376
static void print_movement_msg(int msg_index)
{
  if (msg_index == 0) {
    return;
  }

  if (msg_index < 1 || msg_index > (int)(sizeof(boundary_messages) / sizeof(boundary_messages[0]))) {
    return;
  }

  ui_region_print_str(boundary_messages[msg_index - 1], -1, -1);
}

// KEH: seg000:0x2C0F
// Returns 0 (blocked) or 1 (passable)
static int is_walkable(int x, int y)
{
  if (y < 0 || y >= level_map->map_height) {
    return 0;
  }
  if (x < 0 || x >= level_map->map_width) {
    return 0;
  }

  uint16_t map_width = level_map->map_width;
  int si = (y * map_width + x) << 3;
  if (!(level_map_large[0x416 + si] & 1))
    return 0;

  int count = level_map_large[9];
  for (int i = 0; i < count; i++) {
    unsigned char *entry = npc_base + (i * 0x68);
    if (entry[0x5B] == (uint8_t)x && entry[0x5C] == (uint8_t)y)
      return 0;
  }

  count = level_map_large[0xB];
  for (int i = 0; i < count; i++) {
    unsigned char *entry = ptr_D1DE + (i * 0x66);
    if ((entry[9] & 0xF) != 0 && entry[2] == (uint8_t)x && entry[3] == (uint8_t)y)
      return 0;
  }

  return 1;
}

// KEH: seg000:0x1834
static void sub_1834(uint16_t map_pos, int arg2, int npc_num)
{
  word_D1D8 = arg2;
  dword_D9B4 = map_pos; // ???
  word_BEC0 = npc_num;
  word_D9D0 = 0;

  while (word_D1D8 != 0xFFFF) {
    if (word_D1D8 == 8)
      break;

    loc_98F4(dword_D9B4, word_D1D8, word_D9D0, word_BEC0, 0);

    if (bool_DAE6 && word_D1D8 == 0) {
      ui_refresh_all();
    }
  }

  sub_1766();
}

// KEH: seg000:0x251E
static void set_party_position(int x, int y)
{
  g_game_state.x_pos = x;
  g_game_state.y_pos = y;
  word_BEE8 = 0;
  dword_1EC04 = 0;
}

// KEH: seg000:0x12E9
static void sub_12E9(void)
{
  struct ui_region *saved_region = active_region;

  game_random_range(1, 100);
  sub_113F(0);

  if (word_BEE8 > 0) {
    if ((word_CFC4 % word_BEE8) == 0) {
      sub_92D5();
    }
  }

  sub_2B93(0);
  ui_region_set_active(saved_region, 0);
}

// seg000:0x54D6
// Sums attribute[10] from living (condition > 0) party members,
// divides by 5, with a minimum result of 1.
static int sub_54D6(void)
{
  int sum = 0;

  for (int i = 0; i < g_game_state.party_size; i++) {
    if ((int16_t)g_game_state.players[i].condition > 0) {
      sum += g_game_state.players[i].attributes[10];
    }
  }

  int result = sum / 5;
  if (result == 0) {
    result = 1;
  }
  return result;
}

// KEH: seg000:7685
static int sub_7685()
{
  int var_2 = 0;

  if (level_map_large[8] != 0) {
    printf("%s unhandled, level_map 8, not 0\n", __func__);
  }

  return var_2;
}

// KEH: seg000:0x1339
static void sub_1339(void)
{
  sub_EF7();
  level_mark_entity_cells();
  word_DDDA = sub_4F1A(0);

  if (word_DDDA == 0) {
    if (sub_7685() != 0) {
      printf("%s: unimplemented (0x1356)\n", __func__);
    }
    // 0x136F
    word_DDDA = sub_4F1A(0);
    if (word_DDDA == 0) {
      return;
    }
  }

  // Loc:137F

  printf("%s: unimplemented 137F\n", __func__);
}


// seg000:0xEF7
// Processes NPC and creature "attack ticks" using the party threat level
// computed by sub_54D6. Each NPC/creature has a counter that, when it
// exceeds the threat level, triggers a call to sub_E29.
static void sub_EF7(void)
{
  int threat = sub_54D6();

  // Process NPC encounters (ptr_D206, entry size 0x68)
  {
    uint8_t npc_count = level_map_large[9];

    for (int i = 0; i < npc_count; i++) {
      unsigned char *entry = npc_base + (i * 0x68);

      if (entry[0x5D] < 0x79)
        continue;
      if (entry[0x60] == 0xFF)
        continue;

      uint16_t si = *(uint16_t *)&hds_res->bytes[0x3E8];
      entry[0x64] += entry[si + 0x18];

      while (entry[0x64] >= threat) {
        sub_E29(&entry[0x5B], &entry[0x5C],
                entry[0x5E], entry[0x5F],
                *(uint16_t *)&entry[0x44], entry[0x65]);
        entry[0x64] -= threat;
      }
    }
  }

  // Process creature encounters (ptr_D1DE, entry size 0x66)
  {
    uint8_t creature_count = level_map_large[0x0B];

    for (int i = 0; i < creature_count; i++) {
      unsigned char *entry = ptr_D1DE + (i * 0x66);

      if ((entry[9] & 0x0F) == 0)
        continue;
      if ((entry[9] & 0xF0) != 0)
        continue;

      entry[0x0A] += entry[0x0B];

      while (entry[0x0A] >= threat) {
        sub_E29(&entry[2], &entry[3],
                entry[4], entry[5],
                1, entry[0x0C]);
        entry[0x0A] -= threat;
      }
    }
  }
}


// seg000:0xE29
static void sub_E29(unsigned char *arg1, unsigned char *arg2,
                    uint8_t arg3, uint8_t arg4, uint16_t arg5, uint8_t arg6)
{
  printf("%s: unimplemented\n", __func__);
}


// KEH: seg000:0xE2B8 - animate/display encounter sprite
static void sub_E2B8(uint16_t arg0, int arg1)
{
  printf("%s: unimplemented (arg0=0x%04X, arg1=%d)\n", __func__, arg0, arg1);
}

// KEH: seg000:0x7ED0 - process position/direction, returns > 0 to continue loop
static int16_t sub_7ED0(int x, int y)
{
  printf("%s: unimplemented (x=%d, y=%d)\n", __func__, x, y);
  return 0;
}

// KEH: seg000:0x7F34 - process treasure/item reward step
static int16_t sub_7F34(uint16_t arg0, int x, int y)
{
  printf("%s: unimplemented (arg0=%u, x=%d, y=%d)\n", __func__, arg0, x, y);
  return 0;
}

// KEH: seg000:0x7B02 - display item/message
static void sub_7B02(struct player_rec *player, uint16_t arg1, uint16_t arg2)
{
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0x6D6B - check item restriction
static int sub_6D6B(uint16_t arg0)
{
  printf("%s: unimplemented (arg0=0x%04X)\n", __func__, arg0);
  return 0;
}

// KEH: seg000:0x6D96 - handle restricted item
static void sub_6D96(uint16_t arg0)
{
  printf("%s: unimplemented (arg0=0x%04X)\n", __func__, arg0);
}

// KEH: seg000:0xD6AA - add item to inventory
static void sub_D6AA(void *ptr, uint16_t arg1)
{
  printf("%s: unimplemented\n", __func__);
}

// Loads level script?
static void sub_22(int arg)
{
  g_cached_id[0] = 0xFFFF;
  g_cached_id[1] = 0xFFFF;
  g_cached_id[2] = 0xFFFF;

  // We know indexed_record_rw can only read a max of 64k.
  unsigned char *scr_data = malloc(1 << 16);

  indexed_record_rw(level_scr_file, scr_data, arg - 1, level_scr_bytes, false);
  hexdump(scr_data, 32);

  // First two bytes are length?
  word_DBD6 = *((uint16_t *)scr_data);
  printf("Total uncompressed size: %d\n", word_DBD6);

  if (scr_decompressed != NULL) {
    free(scr_decompressed);
  }
  scr_decompressed = malloc(word_DBD6);
  decompress(scr_data + 4, scr_decompressed, word_DBD6);
  free(scr_data);

  hexdump(scr_decompressed, 32);
}

void sub_7E5B(uint16_t val1, uint16_t var2, void *ud)
{
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0x7FA8
// Handles encounters/interactions at a map tile position.
// If the tile has flag 0x80 set, processes encounter logic (enemies, treasure, etc.)
// and refreshes the map display.
//
// XXX/TODO: THIS IS VERY BROKEN
static void sub_7FA8(int x, int y)
{
  const char *var_12;
  uint8_t var_E;
  uint8_t var_4;
  uint16_t var_A;
  uint16_t var_8;
  int16_t var_6;
  struct player_rec *player;
  uint16_t var_14;
  uint16_t var_16;

  // Calculate tile entry offset from level_map_large
  // offset = (y * map_width + x) * 8 + 0x414
  uint16_t map_width = level_map->map_width;
  uint16_t tile_offset = y * map_width + x;
  level_map_player_pos = &level_map->cells[tile_offset];

  // Check if this tile has the interaction flag (bit 7 of byte at offset +2)
  if ((level_map_player_pos->b2 & 0x80) == 0) {
    return;
  }

  // Initialize locals
  var_12 = NULL;
  var_E = 0;
  var_4 = 0;

  // Get encounter/entity index for this tile
  var_A = check_party_condition(1);
  var_14 = var_A;

  // Look up player/entity data pointer and encounter layout value
  player = &g_game_state.players[var_A];
  sub_E2B8(table_1ACE[var_A], 1);

  // Set active region to encounter UI
  ui_region_set_active(&popup_region, true);

  // Main encounter loop
  while (1) {
    // Check if interaction is complete
    if (var_E != 0)
      goto exit_loop;

    // If encounter changed, re-lookup data and update animations
    if (var_A != var_14) {
      sub_E2B8(table_1ACE[var_14], 1);
      sub_E2B8(table_1ACE[var_A], 1);
      var_14 = var_A;
      player = &g_game_state.players[var_A];
    }

    // TODO:Process interaction with the entity BROKEN
    var_8 = ui_scroll_list_menu(player, var_6, NULL, "You find these items:", sub_7E5B, false);

    if ((int16_t)var_8 < 0) {
      // Negative result: special case
      if (var_8 == 0xFFFF) {
        var_E = 1;
        goto loop_continue;
      }
      // Call menu/dialog handler
      handle_fkeys(var_8 + 0x46, &var_A, 1, 0, 1);
      goto loop_continue;
    }

    // Positive result: look up item/treasure index
    var_8 = table_3240[var_8];

    // Calculate pointer to item data entry (each entry is 6 bytes)
    unsigned char *item_data = ptr_1EC1A + ((uint32_t)var_8 * 6);

    if (item_data[4] == 1) {
      // Money reward: add to 32-bit counter
      {
        uint16_t money_add = *(uint16_t *)item_data;
        uint32_t old_val = ((uint32_t)word_1D13A << 16) | word_1D138;
        old_val += money_add;
        word_1D138 = (uint16_t)old_val;
        word_1D13A = (uint16_t)(old_val >> 16);
      }

      var_6 = sub_7F34(var_8, x, y);
      if (var_6 == 0) {
        var_E = 1;
      }
      goto loop_continue;
    }

    // Non-money reward (item)
    var_16 = item_data[4];

    // Display item/graphic
    sub_7B02(player, word_1E41A + var_16 * 0x18 + 0x2A, 0);

    // Check if item is restricted
    if (sub_6D6B(var_16)) {
      sub_6D96(var_16);
      goto loop_continue;
    }

    // Check inventory space (counter at offset 0x52, max 0x20 items)
    // XXX: THIS IS NOT CORRECT!!
    {
      printf("%s: This code is a bit broken, needs review\n", __func__);
      unsigned char *player_data = (unsigned char *)(intptr_t)player;

      if (player_data[0x52] < 0x20) {
        // Add item to inventory at offset 0x62 + count * 6
        sub_D6AA(player_data + 0x62 + player_data[0x52] * 6, var_16);
        player_data[0x52]++;

        // Re-load item_data and decrement available count
        item_data = ptr_1EC1A + ((uint32_t)var_8 * 6);
        (*(uint16_t *)item_data)--;

        if (*(uint16_t *)item_data == 0) {
          // Out of this item, process next
          var_6 = sub_7F34(var_8, x, y);
          if (var_6 == 0) {
            var_E = 1;
          }
        }
        goto loop_continue;
      }

      // Inventory full - display message with entity name
      snprintf(str_buffer, sizeof(str_buffer), "%s", player->name);
      ui_display_msg(str_buffer);
    }

loop_continue:
    var_6 = sub_7ED0(x, y);
    if (var_6 <= 0)
      break;
  }

exit_loop:
  // Clean up encounter display
  sub_E2B8(table_1ACE[var_14], 1);

  if (var_6 == 0) {
    var_4 = 1;
  }

  // Restore message region and refresh map
  ui_region_set_active(&message_region, false);
  redraw_map_view();

  // If encounter was completed, mark tile as cleared
  if (var_4 != 0) {
    sub_1834(level_map_player_pos->script_offset, 5, 0);
  }
}

// KEH: seg000:0x113F
// Processes encounter entities: handles special effects, health regeneration,
// death/respawn, and counts living entities. Loops until at least one entity
// has health > 0, calling sub_2B93 to spawn new waves when all are dead.
static void sub_113F(int arg0)
{
  uint16_t var_4;   // loop flag (1 = done)
  uint16_t var_10;  // tick divisor: 5 if arg0!=0, else 10
  uint16_t var_2;   // multiplier: 1 if arg0!=0, else map_header.minute_inc
  uint16_t var_8;   // count of entities with health > 0

  // --- Initialization ---
  var_4 = 0;

  if (arg0 != 0) {
    var_10 = 5;
  } else {
    var_10 = 10;
  }

  if (arg0 != 0) {
    var_2 = 1;
  } else {
    var_2 = level_map->minute_inc;
  }

  // KEH: 0x12DB (do/while ?)
  while (var_4 == 0) {
    // KEH: 0x1176
    sub_109D(arg0);

    var_8 = 0;

    // Iterate over all the characters.
    // KEH: 0x1212
    for (int i = 0; i < g_game_state.party_size; i++) {

      // Check affliction 0x58
      if (g_game_state.players[i].affliction & 0x7F) {
        // Handle affliction?
        printf("%s: affliction? 0x%02X unimplemented 0x1233\n", __func__, g_game_state.players[i].affliction);
#if 0
        var_C = 0;
        while (var_C < 7) {
          if (entity[0x58] & (1 << var_C)) {
            // Compute offset into hds_bytes: var_C * 0x16 + 0x3EC
            uint16_t hds_offset = var_C * 0x16 + 0x3EC;

            // divisibility check: word_CFC4 % (hds_bytes[hds_offset] * var_2)
            uint16_t divisor = (uint16_t)hds_bytes[hds_offset] * var_2;
            if (divisor != 0 && (word_CFC4 % divisor) == 0) {
              uint16_t hds_val = hds_bytes[hds_offset + 1];
              sub_8FFA(entity, hds_val);
            }
          }
          var_C++;
        }
#endif
      }

      // KEH: 0x1294
      // --- Health processing (only when word_CFC4 is divisible by var_10) ---
      if ((word_CFC4 % var_10) == 0) {
        printf("%s: health? unimplemented 0x1294\n", __func__);
#if 0
        // 0x12AE
        var_6 = *(uint16_t *)&entity[0x44];

        // Determine health threshold var_E
        if (entity[0x54] == 0) {
          var_E = -15;
        } else {
          if (entity[0x21] == 0) {
            var_E = 0;
          } else {
            var_E = -15;
          }
        }

        // When arg0 != 0: adjust health for entity[0x1B] bonus
        if (arg0 != 0) {
          if (entity[0x1B] > 15) {
            var_6 += entity[0x1B] - 15;
          }
          var_E = 0;
        }

        // Compare health against threshold
        if (var_6 > var_E) {
          // Entity is alive - check if it can regenerate health
          if (!(entity[0x58] & 0x7F)) {
            if (entity[0x54] == 0 || entity[0x21] != 0) {
              // Regenerate health: increment, cap at entity[0x46]
              uint16_t health = *(uint16_t *)&entity[0x44];
              health++;
              if (health > *(uint16_t *)&entity[0x46]) {
                health = *(uint16_t *)&entity[0x46];
              }
              *(uint16_t *)&entity[0x44] = health;

              // If health just reached 1, reset to entity[0x122]
              if (*(uint16_t *)&entity[0x44] == 1) {
                *(uint16_t *)&entity[0x44] = *(uint16_t *)&entity[0x122];
              }
            }
          }
        } else {
          // Entity is dead/depleted
          sub_8FFA(entity, 1);
        }
#endif
      }

      // Count entities with health > 0
      // 0x1203
      if (g_game_state.players[i].condition > 0) {
        var_8++;
      }
    }

    // Check if any entities are still alive
    if (var_8 != 0) {
      var_4 = 1;  // Done - at least one entity alive
    } else {
      sub_2B93(arg0);  // Spawn next wave, continue loop
    }
  }
}

// KEH: seg000:0x92D5
static void sub_92D5(void)
{
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0x109D
// Advances the game clock by a number of minutes. Also increments the frame
// counter (word_CFC4). When arg0 == 0, redraws the date/time UI and calls
// sub_7854 if the hour changed.
static void sub_109D(int arg0)
{
  uint8_t minute_inc;

  if (arg0 != 0) {
    minute_inc = 1;
  } else {
    minute_inc = level_map->minute_inc;
  }

  word_CFC4++;

  uint8_t old_hour = g_game_state.hour;

  // Add fractional minutes to accumulator
  g_game_state.minute += minute_inc % 60;

  // Compute hours to add (from increment + overflow from minute accumulator)
  uint8_t hours_to_add = minute_inc / 60;
  hours_to_add += g_game_state.minute / 60;

  // Total hours, wrap at 24
  uint8_t total_hours = old_hour + hours_to_add;
  g_game_state.hour = total_hours % 24;

  // Normalize minute accumulator to 0-59
  g_game_state.minute = g_game_state.minute % 60;

  // Check if the hour wrapped (old hour was later in the day)
  if (old_hour > g_game_state.hour) {
    g_game_state.day_of_week = (g_game_state.day_of_week + 1) % 7;
  }

  if (arg0 == 0) {
    draw_day_time();
    ui_region_refresh(&datetime_region.rect);
    if (old_hour != g_game_state.hour) {
      sub_7854();
    }
  }
}

// KEH: seg000:0x8FFA
static void sub_8FFA(unsigned char *entity_ptr, int val)
{
  printf("%s: unimplemented (entity_ptr=%p, val=%d)\n", __func__,
         (void *)entity_ptr, val);
}

// KEH: seg000:0x7854
static void sub_7854(void)
{
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0x2B93
// Checks if all entities in the encounter are dead (condition <= -15).
// If all are dead, sets conditions to -65 and spawns next wave when arg0 != 0,
// then always calls sub_2B70 to handle death processing.
static void sub_2B93(int arg0)
{
  uint16_t var_2;  // count of entities with condition > -15

  var_2 = 0;

  // Count how many entities have condition > -15 (alive)
  for (int i = 0; i < g_game_state.party_size; i++) {
    struct player_rec *entity =
        &g_game_state.players[player_data_table[i]];
    if ((int16_t)entity->condition > (int16_t)0xFFF1) {
      var_2++;
    }
  }

  // If at least one entity is alive, return
  if (var_2 != 0) {
    return;
  }

  // All entities are dead
  if (arg0 != 0) {
    // Set all conditions to -65 (0xFFBF)
    for (int i = 0; i < g_game_state.party_size; i++) {
      struct player_rec *entity =
          &g_game_state.players[player_data_table[i]];
      entity->condition = 0xFFBF;
    }
    ui_draw_party_roster(0, false, true);
  }

  sub_2B70(arg0);
}

// KEH: seg000:0x2B70
static void sub_2B70(int arg0)
{
  printf("%s: unimplemented (arg0=%d)\n", __func__, arg0);
}

// KEH: seg000:0x1766
static void sub_1766(void)
{
  if (word_D1D8 == 8) {
    printf("%s: 0x1777 unimplemented\n", __func__);
  }

  // 1825
  if (bool_DAE6) {
    ui_refresh_all();
  }
}

// KEH: seg000:0x253F
static void sub_253F(int direction, int arg2)
{
  ui_region_set_active(&message_region, false);
  ui_active_region_clear();

  ui_region_refresh_active();

  bool bVar7 = false;

  for (int i = 0; i < g_game_state.party_size; i++) {
    if (g_game_state.players[i].condition > 0 &&
        g_game_state.players[i].unknown_57 != 0) {
      bVar7 = true;
      break;
    }
  }

  // One member of the party is drunk
  if (arg2 == 0 && bVar7) {
    uint8_t rnd_val = game_random_range(1, 100);
    if (rnd_val < 11) {
      // Move in a random direction.
      direction = game_random_range(1, 4);
      print_movement_msg(4);
    }
  }

  int local_x = g_game_state.x_pos;
  int local_y = g_game_state.y_pos;

  switch (direction) {
  case 1: // up arrow
    local_y--;
    break;
  case 2: // down arrow
    local_y++;
    break;
  case 3: // right arrow
    local_x++;
    break;
  case 4: // left arrow
    local_x--;
    break;
  default:
    break;
  }

  set_direction(direction);

  if (local_x >= 0 && local_x < ((uint16_t *)&level_map_large)[0] &&
      local_y >= 0 && local_y < ((uint16_t *)&level_map_large)[1]) {

    // CSEG:0x262A
    byte_D1E3 = g_game_state.x_pos;
    byte_D1E8 = g_game_state.y_pos;

    // calculate location
    uint16_t map_width = level_map->map_width;
    map_width *= g_game_state.y_pos;
    map_width += g_game_state.x_pos;

    level_map_player_pos = &level_map->cells[map_width];
    struct map_cell *saved_pos = level_map_player_pos;

    // byte_CDAE = 1;
    byte_DBC6 = 0;
    draw_map_center_tile();

    sub_1834(level_map_player_pos->script_offset, 1, 0);

    // Recalculate level_map_player_pos for new position
    uint16_t new_offset = level_map->map_width;
    new_offset *= local_y;
    new_offset += local_x;
    level_map_player_pos = &level_map->cells[new_offset];

    // Check if position changed
    bool position_changed = (saved_pos != level_map_player_pos);

    if (!position_changed) {
      // Position didn't change, skip movement
      sub_138D(arg2);
    } else {
      // Position changed, perform movement
      byte_D1E3 = local_x;
      byte_D1E8 = local_y;

      // Recalculate level_map_player_pos for new position
      uint16_t final_offset = level_map->map_width;
      final_offset *= local_y;
      final_offset += local_x;
      level_map_player_pos = &level_map->cells[final_offset];

      // 0x26F3
      // Loop through ptr_D206 to find NPC at local_x, local_y
      int npc_count = level_map_large[9];
      for (int i = 0; i < npc_count; i++) {
        unsigned char *entry = npc_base + (i * 0x68);
        if (entry[0x5B] == (uint8_t)local_x && entry[0x5C] == (uint8_t)local_y) {
          entry[0x50] &= 0x7F;
          uint16_t entry_66 = *((uint16_t *)(entry + 0x66));
          sub_1834(entry_66, 6, i);
          break;
        }
      }

      // Check if tile is walkable
      if (is_walkable(local_x, local_y)) {
        set_party_position(local_x, local_y);
        redraw_map_view();
      }

      sub_12E9();
      sub_1834(level_map_player_pos->script_offset, 0, 0);

      if (arg2 == 0) {
        sub_1339();
      }

      redraw_map_view();
    }

    sub_7FA8(g_game_state.x_pos, g_game_state.y_pos);
  } else {
    print_movement_msg(level_map_large[0x0D]);
    sub_138D(0);
    return;
  }
}

// KEH: seg000:0x2813
// Main game loop - extracted from keh_main.asm
static void sub_39FE(int arg1, int arg2)
{
  int16_t key_signed;
  uint8_t key_pressed;
  int16_t var_10;
  int saved_region;
  int i;

  tile_res = resource_load(RESOURCE_TILES, 0, 0);

  level_map_init();

  // KEH: seg000:2820 - sub_1BE equivalent
  // Load byte_1D15A and use it to index into a table, pass to sub_8C
  sub_8C(0x1C);

  show_welcome_message();

  // KEH: seg000:2839-283F
  ui_draw_party_names_header(false);

  // KEH: seg000:2842
  draw_day_time();

  // KEH: seg000:2845
  level_mark_entity_cells();

  // KEH: seg000:2848
  build_map_display();

  // KEH: seg000:284B-2852 - refresh screen
  ui_region_refresh(&whole_screen);

  // KEH: seg000:2855-2897 - Initialize loop variables
  word_1D126 = 0;

  // Initialize 3-element array to 0xFFFF
  // KEH: seg000:2860-2872
  for (i = 0; i < 3; i++) {
    // word ptr [bx-2438h] in disassembly - array of 3 words
    // This appears to be related to some game state tracking
  }

  word_CFC4 = 0;
  word_BEE8 = 0;
  dword_1EC04 = 0;
  byte_1E4BE = 0;
  bool done = false;

  // KEH: seg000:2897 - Main game loop
  while (!done) {
    bool should_exit = vga_poll_events(); // Pump SDL events, fill key buffer.
    if (should_exit) {
      break;
    }

    if (vga_pollkey(0)) {
      key_pressed = vga_waitkey();
      if (key_pressed == 0) {
        break;
      }
      // Sign-extend key to 16-bit for comparison (like cbw in asm)
      key_signed = (int16_t)(int8_t)key_pressed;

      // KEH: seg000:28B1-28D6 - Key dispatch
      if (key_pressed == 0xFD) {
        // 0xFFFD = UP arrow (0xFD) -> direction 1
        // KEH: seg000:29BB-29D9
        sub_253F(1, 0);
      } else if (key_pressed == 0xFA) {
        // 0xFFFA = RIGHT arrow (0xFA) -> direction 3
        // KEH: seg000:29CB-29D9
        sub_253F(3, 0);
      } else if (key_pressed == 0xFB) {
        // 0xFFFB = LEFT arrow (0xFB) -> direction 4
        // KEH: seg000:29D3-29D9
        sub_253F(4, 0);
      } else if (key_pressed == 0xFC) {
        // 0xFFFC = DOWN arrow (0xFC) -> direction 2
        // KEH: seg000:29C3-29D9
        sub_253F(2, 0);
      } else if (key_signed > 0) {
        // Positive values are letter/function keys
        // KEH: seg000:29E0
        if (key_pressed >= 0x3B && key_pressed <= 0x3F) {
          // F-keys (F1-F5): 0x3B-0x3F
          // KEH: seg000:299C-29B8
          int fkey_index = key_pressed - 0x3B;
          // byte_1D15B would be max party index - check bounds
          if (fkey_index < g_game_state.party_size) {
            var_10 = show_character_sheet(fkey_index, false);
            sub_27CC(var_10);
          }
        } else if (key_signed == -1) {
          // 0xFFFF = ESC (0xFF) -> exit/no action
          // KEH: seg000:2A0B
          sub_138D(0);
        } else if (key_pressed == 'A') {
          // KEH: seg000:28D9-28E3
          var_10 = sub_D5BA();
          sub_27CC(var_10);
        } else if (key_pressed == 'E') {
          // KEH: seg000:28E6-2905
          word_1F01A = sub_4F1A(1);
          if (word_1F01A > 0) {
            sub_5691(1, 1);
          }
        } else if (key_pressed == 'Q' || key_pressed == 'q') {
          // KEH: seg000:2908-299A - Quit menu
          uint8_t var_4 = 0;
          saved_region = (int)(intptr_t)active_region;

          while (!var_4) {
            // Set up quit menu region
            ui_region_set_active(&popup_region, true);
            ui_active_region_clear();

            // Display quit menu
            ui_region_print_str(
                "Your choice?\r\r\r"
                "1. Quit (without saving)\r"
                "3. Quit (save game)\r"
                "5. Just save!\r\r"
                "N)evermind",
                0, 0);

            ui_region_refresh_active();

            key_pressed = vga_waitkey();

            // Uppercase conversion
            key_signed = (int16_t)(int8_t)key_pressed;
            // Check if lowercase (bit 0x20 set in attribute table)
            if (key_pressed >= 'a' && key_pressed <= 'z') {
              key_pressed -= 0x20;
              key_signed = (int16_t)(int8_t)key_pressed;
            }

            if (key_pressed == '1') {
              // Quit without saving
              done = true;
              var_4 = 1;
            } else if (key_pressed == '3') {
              // Quit with saving
              done = true;
              // sub_87E5 + sub_8827 would save the game
              sub_8827();
              var_4 = 1;
            } else if (key_pressed == '5') {
              // Continue (save and return to game)
              sub_8827();
              var_4 = 1;
            } else if (key_pressed == 'N' || key_signed == -1) {
              // Cancel - return to game
              var_4 = 1;
            }
          }

          // Restore region
          ui_region_set_active((struct ui_region *)(intptr_t)saved_region, false);
          redraw_map_view();
        }
      }
      // else: unknown key, no action (falls through to loop check)
    }
  }

  // KEH: seg000:2A1E - Exit sequence
  sub_E674();
  sub_10720();

  resource_release(tile_res);
  level_map_free();
}

// KEH: seg000:0x189E
static void sub_189E(int player_idx, uint8_t arg2, uint16_t mode, uint8_t arg6)
{
  struct player_rec *p = &g_game_state.players[player_idx];

  struct map_cell *cell = &level_map->cells[p->unknown_148];

  bool ok = loc_98F4(cell->script_offset, mode, player_idx, p->cmd_arg, arg2);
  if (ok) {
    printf("%s: OK unimplemented\n", __func__);
  } else {
    ui_region_print_str("\rThat has no effect.\r", -1, -1);
    ui_region_refresh_active();
  }
}

static void sub_7CB()
{
  if (byte_DBC6 == 0) {
    return;
  }

  // 0x7D2
  printf("%s: 0x7D2 unimplemented\n", __func__);
}

// KEH: seg000:0x1CEA
static void sub_1CEA(int player_idx, uint8_t arg)
{
  struct player_rec *p = &g_game_state.players[player_idx];

  switch (p->cmd_type) {
  case 0x0A:
    sub_189E(player_idx, 0, 3, arg);
    break;
  case 0x0B:
    printf("%s: Unhandled case 0x0B\n", __func__);
    break;
  case 0x0E:
    printf("%s: Unhandled case 0x0E\n", __func__);
    break;
  default:
    break;
  }
}

// KEH: seg000:0x27CC
// Processes result after an action (movement, interaction, etc.)
static void sub_27CC(int player_idx)
{
  ui_refresh_all();
  if (player_idx == -1) {
    return;
  }

  // 27D8
  byte_DBC6 = 0;
  sub_1CEA(player_idx, 0);
  sub_7CB();
}

// KEH: seg000:0xD5BA
// 'A' key action handler (likely "Attack" or "Action")
// Returns a value passed to sub_27CC for post-action processing
static int sub_D5BA(void)
{
  // TODO: Implement action handler
  printf("%s: unimplemented\n", __func__);
  return 0;
}

// KEH: seg001:03C9
static int sub_103C9()
{
  // Clear out some table.
  for (int i = 0; i < g_game_state.party_size; i++) {
    // table_CFCA[i] = 0
  }

  for (int i = 0; i < g_game_state.party_size; i++) {
    if (g_game_state.players[i].weapon_idx != 0xFF) {
      printf("%s: unimplemented (handle weapon?)\n", __func__);
    }
  }

  return 1;
}

// KEH: seg000:0x4F1A
static int sub_4F1A(int arg0)
{
  uint16_t var_24 = 0;
  g_anim_count = 0;
  uint16_t var_0A = 0;
  uint16_t var_0E = 0;

  g_wanted_id[var_0E] = 0xFFFF;

  // Populate a table with empty strings.
  for (int i = 0; i < 3; i++) {
    npc_names[i] =  "            ";
  }

  var_0A = sub_103C9();
  var_0E = 0;

  // Jump to 0x5068
  int num_npcs = level_map_large[9];
  //hexdump(ptr_D206, 32);

  // Check NPC interactions.
  for (int i = 0; i < num_npcs; i++) {
    // Each NPC is 0x68 bytes.
    unsigned char *npc_entry = npc_base + (i * 0x68);
    if (npc_entry[0x5D] > 0x3C) {
      printf("%s: unimplemented entry? (%d) Jump to loc_5094\n", __func__,
          npc_entry[0x5D]);
    }
  }

  // LOC: 50E4
  int num_creatures = level_map_large[0xB];
  //hexdump(ptr_D1DE, 32);
  for (int i = 0; i < num_creatures; i++) {
    // Each creature is 0x66 bytes.
    unsigned char *creature_entry = ptr_D1DE + (i * 0x66);
    if ((creature_entry[9] & 0xF) != 0) {
      printf("%s: unimplemented creature entry? (%d) Jump to loc_52FE\n",
          __func__,
          creature_entry[9]);
    }
  }
  if (var_24 > 1) {
    printf("%s: unimplemented (%d) 0x5317\n", __func__, var_24);
  }

  return var_24;
}

// KEH: seg000:0x5691
// Called after 'E' action if something was selected
static void sub_5691(int arg0, int arg1)
{
  // TODO: Implement display/update after edit
  (void)arg0;
  (void)arg1;
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0x138D
// Cleanup/transition function called on exit
static void sub_138D(int arg0)
{
  // TODO: Implement cleanup
  printf("%s: unimplemented\n", __func__);
  (void)arg0;
}


// KEH: seg000:0xE674
// Exit cleanup - saves/restores state
static void sub_E674(void)
{
  // TODO: Implement exit cleanup
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0x10720
// Final exit cleanup before program termination
static void sub_10720(void)
{
  // TODO: Implement final cleanup
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0x8827
// Save game handler
static void sub_8827(void)
{
  save_players();
}

