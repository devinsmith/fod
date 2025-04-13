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

#include "random.h"

// DSEG:0x027E
static uint8_t rand_bytes[] = {
  0x64, 0x76, 0x85, 0x54, 0xF6,
  0x5C, 0x76, 0x1F, 0xE7, 0x12,
  0xA7, 0x6B, 0x93, 0xC4, 0x6E,
  0x1B
};

// Generates a random number in the range of 0x00-0xFF
// In the real game, this is called very often, when waiting for a key
// to be pressed (in fact it's called whenever the key input buffer is empty).
//
// seg000:173D
uint8_t game_random()
{
  // mutate our random bytes by adding them on top of each other.
  uint8_t start = rand_bytes[15];
  int carry = 0;
  int i;

  for (i = 14; i >= 0; i--) {
    uint8_t next = rand_bytes[i];
    uint16_t tmp = start + next + carry;
    if (tmp > 0xFF) {
      carry = 1;
    } else {
      carry = 0;
    }
    start = (uint8_t)tmp;
    rand_bytes[i] = start + 1;
  }
  rand_bytes[15]++;

  return rand_bytes[0];
}

// Generates a random number in the range of val1 and val2
// seg000:16FF
uint8_t game_random_range(uint16_t val1, uint16_t val2)
{
  uint8_t ret = (uint8_t)val1;

  if (val1 >= val2) {
    return ret;
  }

  do {
    ret = game_random();
  } while (ret < val1 || ret > val2);

  return ret;
}
