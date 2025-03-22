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
// 0x37A (another table, purpose unknown)


// This is just a tiny part of seg001:0014
void setup_tables()
{
  uint16_t ax = 0;
  for (int i = 0; i < 200; i++) {
    lookup_160_table[i] = ax;
    ax += 160;
  }

  // additional support tables are setup as part of seg001:0294 (potentially
  // based on saved game?)
  ax = 0;
  for (int i = 0; i < 200; i++) {
    lookup_320_table[i] = ax;
    ax += 0x140;
  }
}

unsigned short get_160_offset(int index)
{
  return lookup_160_table[index];
}
