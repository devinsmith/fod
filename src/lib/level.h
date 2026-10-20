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

#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Map header structure (overlays the start of level_map_large).
// The map data is stored as: header (14 bytes) + padding + tile rows (each tile
// is 8 bytes at offset 0x414).
struct level_map_header {
  uint16_t map_width;       // 0x00 - tiles wide
  uint16_t map_height;      // 0x02 - tiles high
  uint16_t filler_tile;     // 0x04 - tile used for out-of-bounds display
  uint8_t  minute_inc;      // 0x06 - minute advancement per tick on overworld
  uint8_t  unknown_07;      // 0x07
  uint8_t  unknown_08;      // 0x08
  uint8_t  npc_count;       // 0x09 - number of NPC entries
  uint8_t  unknown_0A;      // 0x0A
  uint8_t  creature_count;  // 0x0B - number of creature entries (ptr_D1DE)
  uint8_t  unknown_0C;      // 0x0C
  uint8_t  boundary_msg;    // 0x0D - movement-blocked message index
} __attribute__((packed));

// Within level map, cells start at 0x414.
// 8 bytes per cell, unknown what each means.
struct map_cell {
  uint16_t word0;
  uint8_t b2;
  uint8_t b3;
  uint16_t w4;
  uint16_t w6;
};

struct level_map {
 struct level_map_header hdr;                  // 0x00 - 0x0D
 uint8_t  unknown_0E[0x406 - 0x0E];            // 0x0E - 0x405 (1016 bytes), still unknown
 uint16_t npc_offset;                          // 0x406
 uint16_t offset_408;                          // 0x408
 uint16_t offset_40A;                          // 0x40A
 uint16_t offset_40C;                          // 0x40C
 uint16_t unknown_40E[3];                      // 0x40E, 0x410, 0x412: maybe more offsets
 struct map_cell cells[];                      // 0x414
};

extern unsigned char *level_map_large;
extern struct level_map *level_map;
extern const char *level_map_file;
extern unsigned char level_map_offsets[256];

void save_level_map_if_dirty(int level);
void level_map_init();
void level_map_free();

#ifdef __cplusplus
}
#endif

#endif /* LEVEL_H */
