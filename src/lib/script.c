/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2018-2020,2025-2026 Devin Smith <devin@devinsmith.net>
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
#include <string.h>

#include "anim.h"
#include "cursor.h"
#include "game.h"
#include "hexdump.h"
#include "level.h"
#include "npc.h"
#include "platform.h"
#include "script.h"
#include "sfx.h"
#include "ui.h"

// currently in main.c
extern bool bool_DAE6;
extern uint8_t byte_1834;
extern uint16_t word_D1D8;
extern unsigned char *scr_decompressed;
extern unsigned char *level_map_large;

// currently in ui.c
extern struct ui_region *active_region;

// KEH: DSEG:0x1A32
static uint16_t word_1A32 = 0;

// KEH: DSEG:0xC5A0
static uint8_t unknown_flags_table[255] = { 0 };

// KEH DSEG:0x12BB9 - flag set after consume_key
static uint8_t byte_12BB9 = 0;

// KEH: DSEG:0x1CD0
static struct ui_region region_1CD0 = {
  0x0E, // Initial X cursor position
  0x01, // Initial Y cursor position
  0x26, // Max X cursor position
  0x0C, // Max Y cursor position
  0x0E, // Cursor index x
  0x01, // Cursor index y
  { 0x38, 0x8, 0x64, 0x60 }, // ui rect
  0x00,
  NULL,
  0x00,
  NULL
};

// Number of script opcodes is 76?

// KEH: DSEG:0x1981-0x19CC
static const uint8_t skip_table[76] = {
  4, 2, 1, 1, 3, 3, 2, 2, 0, 3, 2, 2, 4, 2, 0, // 1981-198F
  1, 1, 3, 1, 4, 3, 2, 0, 1, 3, 2, 4, 2, 5, 7, // 1990-199E
  5, 1, 2, 0, 3, 2, 2, 1, 0, 1, 0, 4, 2, 2, 2, // 199F-19AD
  1, 0, 3, 0, 0, 2, 0, 4, 2, 1, 1, 2, 1, 3, 3, // 19AE-19BC
  0, 2, 1, 3, 2, 2, 2, 0, 0, 1, 3, 4, 0, 3, 1, // 19BD-19CB
  1 // 19CC
};

typedef enum { ARG_END = 0, ARG_U8, ARG_U16 } arg_type_t;

// Argument list of each script operand containing types and number
// of arguments.
static const uint8_t script_op_args[][6] = {
  /* 0 */  { ARG_END },
  /* 1 */  { ARG_U8, ARG_END },
  /* 2 */  { ARG_U8, ARG_U8, ARG_END },
  /* 3 */  { ARG_U16, ARG_END },
  /* 4 */  { ARG_U8, ARG_U8, ARG_U8, ARG_END },
  /* 5 */  { ARG_U8, ARG_U16, ARG_END },
  /* 6 */  { ARG_U8, ARG_U8, ARG_U8, ARG_U8, ARG_END },
  /* 7 */  { ARG_U8, ARG_U8, ARG_U16, ARG_END },
  /* 8 */  { ARG_U16, ARG_U16, ARG_END },
  /* 9 */  { ARG_U8, ARG_U8, ARG_U8, ARG_U16, ARG_END },
  /* 10 */ { ARG_U8, ARG_U8, ARG_U8, ARG_U16, ARG_U16, ARG_END },
};

// KEH: seg000:0x8F3E
// Given a cursor pointing at the start of a packed table of null-terminated
// strings, skip past `index` of them and return a char* to the start of
// string.
char *get_nth_string(struct data_cursor *cursor, uint16_t index)
{
  char *p = (char *)cursor->base + cursor->offset;

  while (index != 0) {
    // Find string end.
    while (*p != '\0') {
      p++;
    }
    p++;   // step past the null terminator, onto the next string
    index--;
  }

  return p;
}

// KEH: seg000:0xA79A
static void sub_A79A(struct data_cursor *data, uint16_t index)
{
  ui_region_newline_or_scroll();
  ui_region_newline_or_scroll();
  active_region->data_24 = 0;

  char *str = get_nth_string(data, index);

  ui_region_print_str(str, -1, -1);
  ui_region_refresh_active();
}

// KEH: seg000:0x8F9D
void sub_8F9D(struct data_cursor *string_table, uint16_t index)
{
  ui_region_set_active(bool_DAE6 ? &region_1CD0 : &encounter_region, true);
  active_region->data_24 = active_region->max_y_cursor_pos - active_region->initial_y_cursor_pos;
  byte_1834 = 1;
  const char *s = get_nth_string(string_table, index);
  ui_region_print_str(s, 0, 0);
}

static void sub_987A(struct data_cursor *data, uint16_t index)
{
  struct ui_region *saved_region = active_region;
  sub_8F9D(data, index);
  ui_region_refresh_active();

  // Wait for confirmation key.
  while (sys_readkey() != KEY_CONFIRM);

  // loc_9898
  ui_region_set_active(saved_region, false);
  byte_1834 = 0;
}

// KEH: seg000:0x8B08
// sub_8B08: find the first NPC whose byte at 0x49 equals id.
// Returns the base pointer to the NPC or NULL if none match.
unsigned char *sub_8B08(uint16_t id, uint16_t *out_index)
{
  uint8_t npc_count = level_map_large[9];

  for (uint16_t i = 0; i < npc_count; i++) {
    unsigned char *entry = npc_base + (i * 0x68);
    if (entry[0x49] == id) {
      *out_index = i;
      return entry;
    }
  }
  return NULL;
}

// KEH: seg000:0x7E09
static void sub_7E09()
{
  level_map_large[5] |= 0x80;
}

// KEH: seg000:0xDF48
static void sub_DF48(struct data_cursor *cursor, uint16_t *out, uint16_t want)
{
  uint16_t raw    = cursor_read_u8(cursor);
  uint16_t masked = raw & 0x0F;

  uint16_t *o = out;
  *o++ = masked;

  if (masked == want) {
    *o++ = cursor_read_u8(cursor);

    uint16_t chain_idx = raw >> 3;
    const uint8_t *args = script_op_args[chain_idx / 2];

    for (const uint8_t *a = args; *a != ARG_END; a++) {
      *o++ = (*a == ARG_U8) ? cursor_read_u8(cursor) : cursor_read_u16(cursor);
    }
  } else {
    uint8_t selector = cursor->base[cursor->offset];
    uint8_t width    = skip_table[selector];
    cursor->offset += width + 1;
  }
}

// KEH: seg000:0xD8CD
void sub_D8CD(int npc_idx)
{
  // Check if anyone is alive?
  int party_result = check_party_condition(1);
  // Sizeof npc is 0x68 bytes.
  unsigned char *entry = npc_base + (npc_idx * 0x68);
  uint8_t stat = entry[0x63];

  sub_B360(stat, entry);

  ui_draw_party_roster(party_result, true, true);
  bool_DAE6 = true;
}

#define CELL_HI5_SHIFT 11
#define CELL_HI5_MASK  0xF800u

// Returns the cell at (x, y), or NULL if out of range.
static inline struct map_cell *map_cell_at(struct level_map *m, int x, int y)
{
  if (x < 0 || x >= m->hdr.map_width || y < 0 || y >= m->hdr.map_height) {
    return NULL;
  }
  return &m->cells[(size_t)y * m->hdr.map_width + x];
}

// KEH: seg000:0x2AB2
// Set the 5 bit field (bits 11-15 of word0) of the cell at (x, y)
static void sub_2AB2(int x, int y, uint16_t val)
{
  struct map_cell *cell = map_cell_at(level_map, x, y);   // map = (struct level_map *)level_map_large
  if (!cell) {
    return;
  }

  cell->word0 = (cell->word0 & ~CELL_HI5_MASK)
                | (uint16_t)((val & 0x1F) << CELL_HI5_SHIFT);
}

#define TOKEN_MAX_ARGS 5
#define TOKEN_BUF_LEN  (2 + TOKEN_MAX_ARGS)   // type, op, then up to 5 args

// KEH: seg000:0x98F4
// Processes scripted data referenced by ptr for a given event/command type.
int loc_98F4(unsigned char *ptr, int cmd_type, int party_idx, int arg4,
    int arg5)
{
  word_D1D8 = 0xFFFF;

  byte_12BB9 = 0;

  // Compute data pointer from index table
  uint16_t index = *(uint16_t *)ptr;
  if (index == 0xFFFF) {
    return 0;
  }

  uint16_t table_val = *((uint16_t *)scr_decompressed + index);
  printf("%s: script offset: 0x%04X\n", __func__, table_val);

  struct data_cursor cursor;
  cursor.base   = scr_decompressed;
  cursor.offset = table_val + word_1A32; // Is word_1A32 always 0?

  hexdump(scr_decompressed + table_val, 64);
  uint16_t unknown = (scr_decompressed[table_val + 1] << 8) | scr_decompressed[table_val];

  uint16_t bitmask = 1 << (uint8_t)cmd_type;
  if (!(unknown & bitmask)) {
    return 0;
  }

  // vga_pollkey?
  do_expand = true;
  set_current_player_name(party_idx);

  if (!bool_DAE6 && cmd_type == 6) {
    sub_D8CD(arg4);
  }

  // 999F
  if (bool_DAE6) {
    // 99A6
    // check_party_condition stores the result in -0xA2, which we use as party_idx.
    party_idx = check_party_condition(1);
  }

  // 99B4
  //
  // 99CF
  uint16_t script_op_len = (scr_decompressed[table_val + 3] << 8) | scr_decompressed[table_val + 2];
  printf("%s: script_op_len = 0x%04X (%d bytes, including header)\n", __func__, script_op_len, script_op_len);
  // 99EC
  struct data_cursor data_ptr = cursor;
  data_ptr.offset += 4;
  int end_offset = cursor.offset + script_op_len;


  while (data_ptr.offset < end_offset) {
    uint16_t npc_id;
    unsigned char *npc_base;
    struct data_cursor tmp_cursor;

    uint16_t token_buf[TOKEN_BUF_LEN] = { 0 };
    // 9A01
    sub_DF48(&data_ptr, token_buf, cmd_type);

    uint16_t token_type = token_buf[0]; // 0xD0
    uint16_t token_op   = token_buf[1]; // 0xCE

    // arg[0] = bp-0CCh
    // arg[1] = bp-0CAh
    // arg[2] = bp-0C8h

    switch (token_op) {
    case 0x17:
      // A79A:
      sub_A79A(&data_ptr, token_buf[2]);
      break;
    case 0x1F:
      play_sound(token_buf[2]);
      break;
    case 0x39:
      // A7E6
      tmp_cursor.base = cursor.base;
      tmp_cursor.offset = cursor.offset + *(uint16_t *)(cursor.base + cursor.offset + 2);
      sub_987A(&tmp_cursor, token_buf[2]);
      break;
    case 0x34:
      // AF85
      npc_base = sub_8B08(token_buf[2], &npc_id);
      // npc_base is in [bp-60h]
      if (npc_base != NULL) {
        uint8_t val = npc_base[0x49];
        if ((unknown_flags_table[val] & 0x02) == 0) {
          // AFB4
          if (token_op == 0x34) {
            sub_2AB2(npc_base[0x5B], npc_base[0x5C], 0);
            // AFD0
            npc_base[0x5B] = npc_base[0xCA];
            npc_base[0x5C] = npc_base[0xC8];
            uint8_t al = npc_base[0x65];
            // push al
            // push 0x5C
            // push 0x5B
            // push 0x5F
//            sub_D19(
            //

          } else {
            // 0xB00C ?
          }

        }

      }
      break;

    case 0x3A:
      // B063
      npc_base = sub_8B08(token_buf[2], &npc_id);
      if (npc_base != NULL) {
        // Set some NPC value at 0x66 to token_buf[3]
        *(uint16_t *)(npc_base + 0x66) = token_buf[3];
        sub_7E09();
      }
      break;
    default:
      // Check out 0xB1A7 for script op codes.
      printf("%s: Unhandled token operation: 0x%02X (token_type: 0x%02X)\n", __func__, token_op, token_type);
      return 0;
      break;
    }
  }

  return 1;
}
