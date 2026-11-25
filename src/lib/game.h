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

#include <assert.h> // For static_assert macro
#include <stddef.h>
#include <stdint.h>

#include "player.h"

// FOD: DSEG:0x231E - 0x31DE
// KEH: DSEG:0xBEEA - ?
struct game_state {
  uint16_t saved_game;
  uint16_t do_init;

  uint8_t video_init; // 0x04

  uint8_t video_mode; // 0x06

  // 0-23 (hour of the day)
  uint8_t hour; // 0x08 (0xBEF2)
  // 0-59 (minute of the hour)
  uint8_t minute; // 0x09

  // Coordinates of the party on the map.
  uint16_t x_pos; // 0x0A-0x0B
  uint16_t y_pos; // 0x0C-0x0D

  uint32_t money; // 0x0E (shared among all party members)

  uint8_t map; // 0x30

  // Number of players in the party (0-5)
  uint8_t party_size; // 0x31 (0xBF1B)
  uint8_t party_order[5]; // 0x32

  uint8_t day_of_week; // 0x37 (0xBF21)

  struct player_rec players[5]; // players

  // KEH: DSEG:0xC5A0
  uint8_t unknown_flags_table[255];
};

extern struct game_state g_game_state;
extern const char *current_player_name;
extern int current_direction;

extern uint16_t g_wanted_id[];
extern uint16_t g_cached_id[];

bool load_game_state();
bool save_game_state();

bool is_near_player(int radius, int xpos, int ypos);
int check_party_condition(int arg0);
void set_current_player_name(int player_idx);
void set_direction(int direction);
void print_incapacitated(int player_num, uint16_t arg2);

void party_remove_member(int member);

const char *get_possessive_pronoun(uint8_t gender);
const char *get_object_pronoun(uint8_t gender);

#endif /* GAME_H */
