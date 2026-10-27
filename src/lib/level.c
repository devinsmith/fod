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

#include "game.h"
#include "level.h"
#include "resource.h"

unsigned char *level_map_large = NULL;
struct level_map *level_map;

// KEH: DSEG: 0xD284
const char *level_map_file = NULL;
unsigned char level_map_offsets[256];

// KEH: DSEG: 0xD286
struct map_cell *level_map_player_pos;

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

static inline void cell_set_value(struct map_cell *c, uint16_t val)
{
  c->word0 = (c->word0 & ~CELL_HI5_MASK)
           | (uint16_t)((val & 0x1F) << CELL_HI5_SHIFT);
}

// KEH: seg000:0x2AB2
// Set the 5 bit field (bits 11-15 of word0) of the cell at (x, y)
void level_set_cell(int x, int y, uint16_t val)
{
  struct map_cell *cell = map_cell_at(level_map, x, y);   // map = (struct level_map *)level_map_large
  if (!cell) {
    return;
  }

  cell_set_value(cell, val);
}

// KEH: seg000:0xD19
// Place an NPC at map cell (x, y) if nearby.
void level_set_cell_nearby(int radius, int x, int y, uint16_t val)
{
  struct map_cell *cell = map_cell_at(level_map, x, y);
  if (!cell) {
    return;
  }

  cell->word0 &= ~CELL_HI5_MASK;
  if (is_near_player(radius, x, y)) {
    cell_set_value(cell, val);
  }
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
