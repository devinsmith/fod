/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2018-2026 Devin Smith <devin@devinsmith.net>
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
#include <stdlib.h>
#include <string.h>

#include "anim.h"
#include "compress.h"
#include "game.h"
#include "hexdump.h"
#include "resource.h"

// KEH: 0xD206, NPC offsets?
extern unsigned char *ptr_D206;

// KEH: DSEG: 0xBEC2
static uint16_t word_BEC2;

// KEH: DSEG: 0xD1E4
static uint16_t word_D1E4;

// KEH: DSEG: 0xD9C8
const char *level_ani_file = NULL;
// KEH: DSEG: 0xDAF2
unsigned char level_ani_bytes[256];

// KEH DSEG:0xBEE6
extern uint16_t word_BEE6;

extern uint16_t g_anim_count;

static uint16_t word_1E74[4] = { 0x1C28, 0x1C0C, 0x1C44, 0x1C28 };

#define ANIM_SLOTS 3

// KEH: DSEG: 0xDDE0
uint16_t g_wanted_id[ANIM_SLOTS];
// KEH: DSEG: 0xDBC8
uint16_t g_cached_id[ANIM_SLOTS];

// KEH: DSEG: 0x1A0A
uint8_t  *g_anim_buf[ANIM_SLOTS];

/* 12-byte record at DS:BEC2 + 12*i */
struct anim_entry {
  uint16_t field_0;      /* +0x00: copied from word_1E74[i + bias]            */
  uint16_t field_2;      /* +0x02: u16 read from anim data at offset 1        */
  uint8_t *data;         /* +0x04/+0x06: far ptr to decompressed anim buffer  */
  uint16_t field_8;      /* +0x08: never touched here                         */
  uint8_t  active;       /* +0x0A: set to 1                                   */
  uint8_t  field_B;      /* +0x0B: never touched here                         */
};

// KEH: DSEG: 0xBEC2
struct anim_entry g_anim_entry[ANIM_SLOTS];

// KEH: seg000:0x8880
static void sub_8880(int stat)
{
  int bias = 0;
  int slot_free[ANIM_SLOTS];          /* var_8  (1 = free)               */
  int matched[ANIM_SLOTS];            /* var_C  (wanted[i] already cached) */
  int match_slot[ANIM_SLOTS];         /* var_1A (which slot it was in)   */

  word_BEE6 = 0;

  if (stat == 0) {
    bias = 1;
  } else {
    printf("%s: 0x889E unimplemented\n", __func__);

  }
  // 0x88DD
  for (int i = 0; i < ANIM_SLOTS; i++) {
    matched[i]   = 0;
    slot_free[i] = 1;
  }

  // 0x8934
  // Pass 1: which wanted IDs are already resident in a cache slot?
  for (int i = 0; i < g_anim_count; i++) {
    for (int j = 0; j < ANIM_SLOTS; j++) {
      if (g_wanted_id[i] == g_cached_id[j]) {
        match_slot[i] = j;
        slot_free[j]  = 0;     /* pin this slot */
        matched[i]    = 1;
        break;
      }
    }
  }

  // 0x8943
  // Pass 2: load misses into free slots, then fill entries
  for (int i = 0; i < g_anim_count; i++) {
    uint16_t id = g_wanted_id[i];
    int slot;

    // 0x8A1C
    g_anim_entry[i].field_0 = word_1E74[i + bias];

    if (matched[i]) {
      slot = match_slot[i];
    } else {
      unsigned char read_buf[5000];

      // Find a free slot
      for (slot = 0; slot < ANIM_SLOTS && !slot_free[slot]; slot++)
        ;

      if (slot < ANIM_SLOTS) {
        read_indexed_file_data(level_ani_file, read_buf, id, level_ani_bytes, 0);
        hexdump(read_buf, 32);

        uint16_t low_bytes = *(uint16_t *)read_buf;
        uint16_t high_bytes = *(uint16_t *)(read_buf+ 2);
        uint32_t uncompressed_size = (high_bytes << 16) + low_bytes;
        printf("%s: Animation uncompressed size: 0x%04X, %d bytes\n", __func__, uncompressed_size, uncompressed_size);
        g_anim_buf[slot] = malloc(uncompressed_size);
        decompress(read_buf + 4, g_anim_buf[slot], uncompressed_size);
        g_cached_id[slot] = id;
        slot_free[slot]   = 0;
      }
    }
    uint8_t *p = g_anim_buf[slot];
    g_anim_entry[i].data    = p;
    g_anim_entry[i].field_2 = (uint16_t)(p[1] | (p[2] << 8));
    g_anim_entry[i].active  = 1;
  }

  word_BEE6 = stat ? 3 : 1;
  word_D1E4 = 0;
}

// KEH: seg000:0xB360
static void sub_B360(int stat, unsigned char *entry)
{
  g_anim_count = 1;

  // looks like 0x1C0C
  word_BEC2 = word_1E74[1];

  g_wanted_id[0] = stat;

  // Entry is copied to CDB0/B2, not sure if needed.
  sub_8880(0);
  printf("%s: unimplemented\n", __func__);
}

// KEH: seg000:0xD8CD
void sub_D8CD(int npc_idx)
{
  // Check if anyone is alive?
  int party_result = check_party_condition(1);
  // Sizeof npc is 0x68 bytes.
  unsigned char *entry = ptr_D206 + (npc_idx * 0x68);
  uint8_t stat = entry[0x63];
  printf("%s: unimplemented\n", __func__);

  sub_B360(stat, entry);
}
