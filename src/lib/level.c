/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2026 Devin Smith <devin@devinsmith.net>
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

#include <stdlib.h>

#include "level.h"
#include "resource.h"

unsigned char *level_map_large = NULL;
struct level_map *level_map;

// KEH: DSEG: 0xD284
const char *level_map_file = NULL;
unsigned char level_map_offsets[256];


// KEH: seg000:0x87E5
void save_level_map_if_dirty(int level)
{
  if (level == 0) {
    return;
  }

  if (!(level_map_large[5] & 0x80)) {
    return;
  }

  level_map_large[5] &= 0x7F;
  indexed_record_rw(level_map_file, level_map_large, level - 1,
      level_map_offsets, true);
}

void level_map_init()
{
  // It's unclear how large this should be.
  level_map_large = malloc(0x10000);
  level_map = (struct level_map *)level_map_large;
}

void level_map_free()
{
  free(level_map_large);
}
