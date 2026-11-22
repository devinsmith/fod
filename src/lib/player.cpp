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

#include "game.h"
#include "player.h"
#include "skill.h"
#include "ui.h"

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
    uint8_t item_id = player->equipped_armor[i];

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

// KEH: seg000:0x969F
bool skill_use_check(uint16_t player_idx, int skill)
{
  struct player_rec *p = &g_game_state.players[player_idx];
  bool improved = false;

  // Active skills (0-15) are controlled by aptitude, passive (16+) by intelligence
  int attr_idx = (skill > 15) ? ATTR_INTELLIGENCE : ATTR_APTITUDE;

  p->skill_uses[skill]++;
  const struct skill_def *sd = get_skill_def(skill);
  if (p->skill_uses[skill] >= sd->threshold) {
    uint8_t attr = p->attributes[attr_idx];

    // Level can only rise while it is below (attribute - 15)
    //
    if (attr > 15 && (uint16_t)(attr - 15) > p->skills[skill]) {
      p->skills[skill]++;
      improved = true;
    }
    p->skill_uses[skill] = 0;
  }

  return improved;
}

#define NO_ITEM   0xFF      /* also "not equipped" in the equip bytes */

// remove item at `slot` if value == 0xFF,
// otherwise reset `slot` to a new item whose id is `value`.
static void fix_equipped_ref(uint8_t *e, uint16_t slot, bool removing)
{
  if (*e == NO_ITEM)
    return;
  if (*e == slot)
    *e = NO_ITEM;                 /* the affected item was equipped: unequip it */
  else if (*e > slot && removing)
    (*e)--;                       /* later slots shift down by one */
}

// KEH: seg001:060D
// Remove or set an item at a certain slot.
void inv_remove_or_set_item(uint16_t player_idx, uint16_t slot, uint16_t value)
{
  struct player_rec *p = &g_game_state.players[player_idx];
  bool removing = (value == 0xFF);

  fix_equipped_ref(&p->equipped_weapon, slot, removing);
  for (int i = 0; i < 3; i++) {
    fix_equipped_ref(&p->equipped_armor[i], slot, removing);
  }

  if (removing) {
    p->inventory_count--;
    for (uint16_t i = slot; i < p->inventory_count; i++) {
      p->items[i] = p->items[i + 1];      // 6-byte records
    }
  } else {
    // XXX: This seems buggy, why aren't the other properties set?
    p->items[slot].item_id = (uint8_t)value;
    p->items[slot].ammo_count = 0;
    p->items[slot].props[0] = 0;
  }
}

// KEH: seg000:0x6F9D
void skill_up_message(struct player_rec *p, uint16_t skill, bool flag)
{
  const struct skill_def *sd = get_skill_def(skill);

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%s now has a %s level of %d.",
            p->name, sd->name, p->skills[skill]);

  if (flag) {
    ui_region_print_str(str_buffer, -1, -1);
    ui_region_newline_or_scroll();
    ui_region_newline_or_scroll();
  } else {
    ui_region_set_active(&message_region, 0);
    ui_region_newline_or_scroll();
    ui_region_newline_or_scroll();
    ui_region_print_str(str_buffer, -1, -1);
    ui_region_refresh_active();
  }
}
