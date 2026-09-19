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

#include "script.h"

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

// Argument list of each script operand containing types of number
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
void sub_DF48(struct data_cursor *cursor, uint16_t *out, uint16_t want)
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
