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

#include "tables.h"

// 0x5BF
unsigned short lookup_160_table[200];
// 0x42F
unsigned short lookup_320_table[200];

// 0x37A contains function pointers to line functions
// (probably not used in this code)
unsigned short line_fptr_table[0x51];

static void sub_02A9();

// This is just a tiny part of seg001:0014
void setup_tables()
{
  // seg001:0030
  uint16_t ax = 0;
  for (int i = 0; i < 200; i++) {
    lookup_160_table[i] = ax;
    ax += 160;
  }

  // Use value of disk1 [6] to initialize other data.
  // 0 - 0x108
  // 1 - 0x190
  // 2 - 0x1BE
  // 3 - 0x2A9
  sub_02A9();

}

static void sub_02A9()
{
  // Self modifying code
  // mov  word cs:[0353],0136
  // Normally we call 0x367, but the self modifying code means
  // we'll end up calling 0x4B8 (0x355 + 0x0136)

  // setup VGA (but done from main.c)

  // additional support tables are setup as part of seg001:0294 (potentially
  // based on saved game?)
  uint16_t ax = 0;
  for (int i = 0; i < 200; i++) {
    lookup_320_table[i] = ax;
    ax += 0x140;
  }

  ax = 0xB74; // 0 lines (RET)
  for (int i = 0; i < 0x51; i++) {
    line_fptr_table[i] = ax;
    ax -= 0x15; // each line function pointer is 0x15 bytes of assembly
  }
}

unsigned short get_160_offset(int index)
{
  return lookup_160_table[index];
}
