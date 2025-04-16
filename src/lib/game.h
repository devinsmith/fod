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

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct item_rec {
  uint8_t item_id;

  // usually extra properties are number of ammo for bullets, etc.
  uint8_t props[5];
};

struct player_rec {
  char name[12];

  uint8_t profession;

  uint8_t strength;
  uint8_t intelligence;
  uint8_t dexterity;
  uint8_t willpower;
  uint8_t aptitude;
  uint8_t charisma;
  uint8_t luck;

  uint8_t attribute_points; // 0x23

  uint8_t active_skills[16]; // 0x24
  uint8_t passive_skills[16]; // 0x34

  uint16_t condition;
  uint16_t max_condition;

  // Value of 0xFF = no item equitped
  // other values are item IDs
  uint8_t eq_items[4];

  uint8_t rank;

  uint8_t unknown_8C; // 0x8C

  uint8_t affliction; // bit encoded
  /*  00000001 - Poisoned
      00000010 - Irradiated
      00000100 - Rabid
      00001000 - Envenomed
      00010000 - Condition 5
      00100000 - Condition 6
      01000000 - Condition 7
      10000000 - Mutant
   */

  struct item_rec items[32];
};

struct game_state {
  uint16_t saved_game;
  uint16_t do_init;

  uint8_t video_mode; // 0x06

  // 0-23 (hour of the day)
  uint8_t hour; // 0x08
  // 0-59 (minute of the hour)
  uint8_t minute; // 0x09

  uint32_t money; // 0x0E

  // Number of players in the party (0-5)
  uint8_t party_size; // 0x31
  uint8_t party_order[5]; // 0x32

  struct player_rec players[5]; // players
};

extern struct game_state g_game_state;

bool load_game_state();

#ifdef __cplusplus
}
#endif

#endif /* GAME_H */
