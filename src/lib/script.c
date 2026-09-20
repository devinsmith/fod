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

#include "game.h"
#include "hexdump.h"
#include "script.h"

// currently in main.c
extern uint8_t byte_DAE6;
extern uint16_t word_D1D8;
extern unsigned char *scr_decompressed;

// KEH: DSEG:1979
static uint8_t byte_1979 = 0;

// KEH: DSEG:0x1A32
static uint16_t word_1A32 = 0;

// KEH DSEG:0x12BB9 - flag set after consume_key
static uint8_t byte_12BB9 = 0;

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

// KEH: seg000:0xDF22
// Read one byte at cursor, advance by one, zero-extend.
static uint16_t cursor_read_u8(struct data_cursor *cursor)
{
  return cursor->base[cursor->offset++];
}

// KEH: seg000:0xDF36
// Read little-endian word at cursor, advance by 2.
static uint16_t cursor_read_u16(struct data_cursor *cursor)
{
  uint16_t v = *(uint16_t *)(cursor->base + cursor->offset);
  cursor->offset += 2;
  return v;
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
    uint8_t  selector = cursor->base[cursor->offset];
    uint8_t  width     = skip_table[selector];
    cursor->offset += width + 1;
  }
}

#define TOKEN_MAX_ARGS 5
#define TOKEN_BUF_LEN  (2 + TOKEN_MAX_ARGS)   // type, op, then up to 5 args

// KEH: seg000:0x98F4
// Processes scripted data referenced by ptr for a given event/command type.
void loc_98F4(unsigned char *ptr, int cmd_type, int party_idx, int arg4, int arg5)
{
  word_D1D8 = 0xFFFF;

  byte_12BB9 = 0;

  // Compute data pointer from index table
  uint16_t index = *(uint16_t *)ptr;
  if (index == 0xFFFF)
    return;

  printf("%s: 0x9946 unimplemented (index=0x%04X), cmd/event = %d\n",
      __func__, index, cmd_type);

  uint16_t table_val = *((uint16_t *)scr_decompressed + index);
  printf("%s: table_val 0x%04X\n", __func__, table_val);

  struct data_cursor cursor;
  cursor.base   = scr_decompressed;
  cursor.offset = table_val + word_1A32; // Is word_1A32 always 0?

  hexdump(scr_decompressed + table_val, 32);
  uint16_t unknown = (scr_decompressed[table_val + 1] << 8) | scr_decompressed[table_val];

  if ((unknown & 1) != 1) {
    printf("%s: 0x9972 unimplemented (val=0x%04X)\n", __func__, unknown);
    // Jump to B24A
  }

  // vga_pollkey?
  byte_1979 = 1;

  struct player_rec *player = &g_game_state.players[party_idx];
  if (byte_DAE6 == 0 && cmd_type == 6) {
    printf("%s: 0x9996 unimplemented (val=0x%04X)\n", __func__, unknown);
  }
  // 999F
  if (byte_DAE6 != 0) {
      printf("%s: 0x99A6 unimplemented (val=0x%04X)\n", __func__, unknown);
  }
  // 99B4
  //
  // 99CF
  uint16_t unknown2 = (scr_decompressed[table_val + 3] << 8) | scr_decompressed[table_val + 2];
  printf("%s: unknown2 = 0x%04X\n", __func__, unknown2);

  // 99EC


  struct data_cursor data_ptr = cursor;
  data_ptr.offset += 4;

  uint16_t token_buf[TOKEN_BUF_LEN] = { 0 };
  // 9A01
  sub_DF48(&data_ptr, token_buf, cmd_type);

  uint16_t token_type = token_buf[0];
  uint16_t token_op   = token_buf[1];

  printf("%s: Token type: %d, op: %d\n", __func__, token_type, token_op);

  switch (token_op) {
  case 0x1F:
    // technically this is a jump, and a jump back up to 0x99EC
    //sub_A9C0();
    break;
  }
}
