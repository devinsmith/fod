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

#ifndef ITEM_H
#define ITEM_H

#include <stdint.h>

// 6 byte entry in the loot table at 0xD9DA
struct loot_entry {
  uint16_t count;    // +0  money amount, or number of this item remaining
  uint8_t  unk2;     // +2
  uint8_t  unk3;     // +3
  uint8_t  item_id;  // +4  1 == money
  uint8_t  unk5;     // +5
} __attribute__((packed));

#define ITEM_WEAPON 0x01
#define ITEM_ARMOR  0x02

// Item definition: 24 bytes, table from globals + 0x2A
struct item_def {
  char name[20];         // NUL-terminated
  uint8_t flags;         // 0x14
  uint8_t weapon_type;   // 0x15
  uint8_t unk_16;        // 0x16
  uint8_t unk_17;        // 0x17
};

// Weapon type is 60 bytes, but mostly unknown.
struct weapon_type {
  unsigned char unknown_00[0x1A];
  uint8_t flags;      // 0x1A, bit 0 = ranged / takes ammo
  unsigned char unknown_1B[0x3C-0x1B];
};

const struct item_def *get_item_def(int item_id);
const struct weapon_type *get_weapon_type(int item_id);

#endif /* ITEM_H */
