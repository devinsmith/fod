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

#include "player.h"

extern unsigned char *arch_offset; // currently in main.c

// KEH: seg000:0x90F6
// Categorizes a player's condition:
//   0 = Healthy
//   1-4 = Injured
//   5 = Critical
int get_player_condition_status(struct player_rec *player)
{
  int16_t cond = (int16_t)player->condition;

  if (cond >= 1) {
    return 0;
  }
  if (cond > -60) {
    return (abs(cond) / 15) + 1;
  }
  return 5;
}

/* KEH: (seg000:7D9B) -- calculate armor class */
uint16_t player_armor_class(const struct player_rec *player)
{
  uint16_t sum = 0;

  // Look at equipted armor items.
  for (int i = 0; i < 3; i++) {
    uint8_t item_id = player->armor_idx[i];

    if (item_id != 0xFF) {
      printf("%s: 0x7DBE unimplemented (item: 0x%02X)\n", __func__, item_id);
      // Manipulates sum
      // sum += item_defs[p->items[p->eq_items[i]].item_id].armor; ?????
    }
  }

  if (sum == 0) {
    // 0x7DE1
    // Deal with player's profession? Does each profession have an armor class?
    uint8_t prof_val = arch_offset[(player->profession << 7) + 0x7A];
    return prof_val;
  }

  return sum;
}

// KEH: seg000:6F6B
// Can this player act even though they are unconcious. Acts based on Will Power (WP)
//
// Will Power is defined as mental toughness. WP is the ability to exert mind
// over matter, even to the point of remaining able to fight after you have
// been rendered unconscious.
bool player_has_willpower(const struct player_rec *player)
{
  int hp = player->condition;
  uint8_t will_power = player->attributes[3];

  if (will_power > 15) {
    hp += will_power - 15;
  }

  return hp > 0;
}
