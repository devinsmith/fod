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
#include "ui.h"

struct game_state g_game_state;

static unsigned char disk1[3776];
static size_t disk1_size = 0;
static size_t disk1_offset = 0;

// KEH: DSEG:0x1A36
static uint16_t party_sprite_positions[] = {
  0x1E70,
  0x1E68,
  0x1E78,
  0x1E80,
  0x1E90,
  0x1E88,
  0x1E98,
  0x1EA0
};

// KEH: DSEG:0x1A48
int current_direction = 0;

// KEH: DSEG:0x1A4A
const char *current_player_name;

static void read_bytes(void *buffer, size_t count)
{
  if (count + disk1_offset > disk1_size) {
    fprintf(stderr, "Failed to read %zu bytes of game state data at pos: %zu\n",
            count, disk1_offset);
    exit(EXIT_FAILURE);
  }

  memcpy(buffer, &disk1[disk1_offset], count);
  disk1_offset += count;
}

static void write_bytes(FILE *fp, const void *buffer, size_t count)
{
  size_t bytes_written = fwrite(buffer, 1, count, fp);
  if (bytes_written != count) {
    fprintf(stderr, "Failed to write %zu bytes to file\n", count);
    fclose(fp);
    exit(EXIT_FAILURE);
  }
}

/* Read a uint8_t value */
static uint8_t read_uint8()
{
  uint8_t value;
  read_bytes(&value, sizeof(uint8_t));
  return value;
}

static void write_uint8(FILE *fp, uint8_t value)
{
  write_bytes(fp, &value, sizeof(uint8_t));
}

/* Read a uint16_t value */
static uint16_t read_uint16()
{
  uint8_t bytes[2];

  read_bytes(bytes, 2);
  /* Assuming little-endian storage in the file */
  return (uint16_t)((bytes[1] << 8) | bytes[0]);
}

/* Write a uint16_t value in little-endian format */
static void write_uint16(FILE *fp, uint16_t value)
{
  uint8_t bytes[2];

  /* Convert to little-endian format */
  bytes[0] = (uint8_t)(value & 0xFF);
  bytes[1] = (uint8_t)((value >> 8) & 0xFF);

  write_bytes(fp, bytes, 2);
}

/* Read a uint32_t value */
static uint32_t read_uint32()
{
  uint8_t bytes[4];

  read_bytes(bytes, 4);

  /* Assuming little-endian storage in the file */
  return (uint32_t)((bytes[3] << 24) |
      (bytes[2] << 16) | (bytes[1] << 8) | bytes[0]);
}

static void advance_reader(size_t count)
{
  disk1_offset += count;
}

static void jump_to_offset(size_t offset)
{
  disk1_offset = offset;
}

/* Function to read an item_rec from file */
static void read_item_rec(struct item_rec *item)
{
  item->item_id = read_uint8();
  item->ammo_type = read_uint8();
  item->ammo_count = read_uint8();

  for (int i = 0; i < 2; i++) {
    item->props[i] = read_uint8();
  }

  item->flags = read_uint8();
}

/* Function to read a player_rec from a file */
static void read_player_rec(struct player_rec *player)
{
  /* Read player name (12 chars + NUL) */
  read_bytes(player->name, sizeof(player->name));
  player->name[sizeof(player->name) - 1] = '\0';

  /* Read player stats */
  player->profession = read_uint8();
  player->unknown_0F = read_uint8();

  player->unknown_10 = read_uint8();
  player->unknown_11 = read_uint8();

  read_bytes(player->unknown_12, sizeof(player->unknown_12));

  read_bytes(player->attributes, sizeof(player->attributes));
  /* Assuming attributes are stored in the order:
   * Strength, Intelligence, Dexterity, Willpower, Aptitude,
   * Charisma, Luck, Unknown (0x59), Unknown (0x5A),
   * Unknown (0x5B), Unknown (0x5C)
   */
  player->attribute_points = read_uint8();

  /* Read active skills */
  for (int i = 0; i < 16; i++) {
    player->skills[i] = read_uint8();
  }

  /* Read passive skills */
  for (int i = 0; i < 16; i++) {
    player->skills[i + 16] = read_uint8();
  }

  /* Read condition values */
  player->condition = read_uint16();
  player->max_condition = read_uint16();

  player->unknown_48 = read_uint8();
  player->unknown_49 = read_uint8();

  player->weapon_idx = read_uint8();

  /* Read equipped armor items */
  for (int i = 0; i < 3; i++) {
    player->armor_idx[i] = read_uint8();
  }

  /* Read rank and affliction */
  player->rank = read_uint8();
  player->unknown_4F = read_uint8();
  // Gender starts at 0x50
  player->gender = read_uint8();
  player->unknown_51 = read_uint8();
  player->inventory_count = read_uint8();

  advance_reader(4); // 0x8D, 0x8E, 0x8F, 0x90
  player->unknown_57 = read_uint8();
  player->affliction = read_uint8();

  advance_reader(9); // 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B

  /* Read inventory items */
  for (int i = 0; i < 32; i++) {
    read_item_rec(&player->items[i]);
  }

  player->unknown_122 = read_uint8();
  player->unknown_123 = read_uint8();
  read_bytes(player->skill_uses, sizeof(player->skill_uses));
  player->cmd_type = read_uint8();
  player->cmd_arg = read_uint8();
  player->unknown_146 = read_uint8();
  player->selected_item = read_uint8();
  player->unknown_148 = read_uint32();
}

// seg000:0x0105
bool load_game_state()
{
  FILE *fp = fopen("disk1", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Couldn't read disk1, exiting!\n");
    exit(1);
  }

  fread(disk1, 1, sizeof(disk1), fp);
  fclose(fp);
  disk1_offset = 0;
  disk1_size = sizeof(disk1);

  memset(&g_game_state, 0, sizeof(g_game_state));

  // The original game probably read from a file into a struct, but this is
  // not endian safe, and it also seems that there is padding bytes that are
  // saved and read as well.

  g_game_state.saved_game = read_uint16();
  g_game_state.do_init = read_uint16();
  g_game_state.video_init = read_uint8();
  jump_to_offset(0x6);
  g_game_state.video_mode = read_uint8();

  /* Read time data */
  jump_to_offset(0x8); // 0x08
  g_game_state.hour = read_uint8();
  g_game_state.minute = read_uint8();

  g_game_state.x_pos = read_uint16();
  g_game_state.y_pos = read_uint16();

  /* Read money */
  g_game_state.money = read_uint32();

  jump_to_offset(0x30); // 0x30
  g_game_state.map = read_uint8();
  g_game_state.party_size = read_uint8();

  /* Read party order */
  for (int i = 0; i < 5; i++) {
    g_game_state.party_order[i] = read_uint8();
  }
  g_game_state.day_of_week = read_uint8();

  /* Read player records */
  for (int i = 0; i < 5; i++) {
    jump_to_offset(0x3A + (i * 332));
    read_player_rec(&g_game_state.players[i]);
  }

  // TODO:
  // This does a number of other manipulations of disk1_bytes based on whether
  // we're dealing with a saved game or not. For a new game, we have to
  // initialize a number of data components to 0.

  if (g_game_state.do_init != 0) {
    fprintf(stderr, "Second word of disk1 is not 0, unhandled (CS: 0x014D)!\n");
    exit(1);
  }

  if (g_game_state.saved_game == 0) {

    for (int i = 0; i < 5; i++) {
      g_game_state.party_order[i] = 0;
      g_game_state.players[i].name[0] = '\0';
      g_game_state.players[i].inventory_count = 6;

      // Initialize items array
      for (int n = 0; n < 32; n++) {
        g_game_state.players[i].items[n].item_id = 0xFF;
        for (int j = 0; j < 5; j++) {
          g_game_state.players[i].items[n].props[j] = 0;
        }
      }

      // 0x01DF (TODO)
      g_game_state.players[i].items[0].item_id = 2; // Handgun?
    }

    // 0x298
    g_game_state.money = 100;

    g_game_state.party_size = 0; // Number of players in the party
  }

  return g_game_state.saved_game != 0;
}

/* Save game state to disk1 */
bool save_game_state()
{
  // TODO: This is not finished, because the struct is not the same size as
  // the game, and this would produce a corrupted save game file.
  FILE *fp = fopen("disk1", "wb");
  if (fp == NULL) {
      fprintf(stderr, "Error: Could not open file 'disk1' for writing\n");
      return false;
  }

  // The original game wrote directly from a struct to a file, but this is not
  // endian safe, and it also seems that there is padding bytes that are saved
  // and read as well.

  /* Write basic game state values */
  write_uint16(fp, g_game_state.saved_game);  // 0x00-0x01
  write_uint16(fp, g_game_state.do_init);     // 0x02-0x03
  write_uint16(fp, 0); // Unknown (0x04-0x05) // Padding?
  write_uint8(fp, g_game_state.video_mode);   // 0x06
#if 0
  /* Write time data */
  fwrite(&g_game_state.hour, sizeof(uint8_t), 1, fp);
  fwrite(&g_game_state.minute, sizeof(uint8_t), 1, fp);

  /* Write money */
  fwrite(&g_game_state.money, sizeof(uint32_t), 1, fp);

  /* Write party information */
  fwrite(&g_game_state.party_size, sizeof(uint8_t), 1, fp);

  /* Write party order */
  fwrite(g_game_state.party_order, sizeof(uint8_t), 5, fp);

  /* Write player records */
  for (int i = 0; i < 5; i++) {
    struct player_rec *player = &g_game_state.players[i];

    /* Write player name */
    fwrite(player->name, 1, 12, fp);

    /* Write player stats */
    fwrite(player->attributes, 1, sizeof(player->attributes), fp);
    fwrite(&player->attribute_points, sizeof(uint8_t), 1, fp);

    /* Write active skills */
    fwrite(player->active_skills, sizeof(uint8_t), 16, fp);

    /* Write passive skills */
    fwrite(player->passive_skills, sizeof(uint8_t), 16, fp);

    /* Write condition values */
    fwrite(&player->condition, sizeof(uint16_t), 1, fp);
    fwrite(&player->max_condition, sizeof(uint16_t), 1, fp);

    /* Write equipped items */
    fwrite(player->eq_items, sizeof(uint8_t), 4, fp);

    /* Write rank and affliction */
    fwrite(&player->rank, sizeof(uint8_t), 1, fp);
    fwrite(&player->affliction, sizeof(uint8_t), 1, fp);

    /* Write inventory items */
    for (int j = 0; j < 32; j++) {
      fwrite(&player->items[j].item_id, sizeof(uint8_t), 1, fp);
      fwrite(player->items[j].props, sizeof(uint8_t), 5, fp);
    }
  }
#endif
  fclose(fp);

  return true;
}

// KEH: seg000:0x073E
// Returns true if (xpos, ypos) is within 'radius' of the player on
// both axes (an axis-aligned square centered on the player, edges inclusive).
// See also Chebyshev distance.
bool is_near_player(int radius, int xpos, int ypos)
{
  int dx = xpos - g_game_state.x_pos;
  int dy = ypos - g_game_state.y_pos;

  return abs(dx) <= radius && abs(dy) <= radius;
}

// KEH: seg000:0xDFC6
// Saves the player's current direction, as well as updating the sprite
// character to reflect the direction accurately.
void set_direction(int direction)
{
  if (direction != 5) {
    current_direction = direction;

    ui_set_sprite_overlay(1, party_sprite_positions[direction - 1]);
  }
}

// KEH: seg001:045A
void party_remove_member(int member)
{
  struct player_rec *player = &g_game_state.players[member];

  g_game_state.unknown_flags_table[player->unknown_49] &= ~0x01;
  player->name[0] = '\0';

  g_game_state.party_size--;

  for (int i = member; i < g_game_state.party_size; i++) {
    g_game_state.party_order[i] = g_game_state.party_order[i + 1];
    g_game_state.players[i] = g_game_state.players[i + 1];
  }
}

// KEH: seg000:0xBB93 - check party condition
int check_party_condition(int arg0)
{
  int i;

  for (i = 0; i < g_game_state.party_size; i++) {
    if (get_player_condition_status(&g_game_state.players[i]) < arg0) {
      return i;
    }
  }
  return g_game_state.party_size;
}

// KEH: seg000:0x98E3
void set_current_player_name(int player_idx)
{
  current_player_name = g_game_state.players[player_idx].name;
  // XXX:Should this function also return this for callers?
};

// KEH: seg000:CB19
// Print incapacitated message
void print_incapacitated(int player_num, uint16_t arg2)
{
  char buffer[42];

  struct player_rec *current_player = &g_game_state.players[player_num];

  snprintf(buffer, sizeof(buffer), "%s is incapacitated.", current_player->name);

  if (arg2 == 0) {
    ui_display_msg(buffer);
    return;
  }

  sub_B2F0(buffer, 0, 2);
}

static const char *poss_pronoun_tbl[] = {
  "its", "his", "her"
};

static const char *obj_pronoun_tbl[] = {
  "it", "him", "her"
};

const char *get_possessive_pronoun(uint8_t gender)
{
  return poss_pronoun_tbl[gender & 3];
}

const char *get_object_pronoun(uint8_t gender)
{
  return obj_pronoun_tbl[gender & 3];
}

