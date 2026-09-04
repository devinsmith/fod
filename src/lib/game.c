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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

struct game_state g_game_state;

static unsigned char disk1[3776];
static size_t disk1_size = 0;
static size_t disk1_offset = 0;

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

/* Write a uint32_t value in little-endian format */
static void write_uint32(FILE *fp, uint32_t value)
{
  uint8_t bytes[4];

  /* Convert to little-endian format */
  bytes[0] = (uint8_t)(value & 0xFF);
  bytes[1] = (uint8_t)((value >> 8) & 0xFF);
  bytes[2] = (uint8_t)((value >> 16) & 0xFF);
  bytes[3] = (uint8_t)((value >> 24) & 0xFF);

  write_bytes(fp, bytes, 4);
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
  item->unknown = read_uint8();
  item->str_index = read_uint8();

  for (int i = 0; i < 2; i++) {
    item->props[i] = read_uint8();
  }

  item->flags = read_uint8();
}

/* Function to read a player_rec from a file */
static void read_player_rec(struct player_rec *player)
{
  /* Read player name (12 chars) */
  read_bytes(player->name, sizeof(player->name) - 1);
  player->name[12] = '\0';

  advance_reader(2); // Skip 2 bytes

  /* Read player stats */
  player->profession = read_uint8();
  advance_reader(7);

  // Gender starts at 0x50
  player->gender = read_uint8();
  player->unknown_51 = read_uint8();

  read_bytes(player->attributes, sizeof(player->attributes));
  /* Assuming attributes are stored in the order:
   * Strength, Intelligence, Dexterity, Willpower, Aptitude,
   * Charisma, Luck, Unknown (0x59), Unknown (0x5A),
   * Unknown (0x5B), Unknown (0x5C)
   */
  player->attribute_points = read_uint8();

  /* Read active skills */
  for (int i = 0; i < 16; i++) {
    player->active_skills[i] = read_uint8();
  }

  /* Read passive skills */
  for (int i = 0; i < 16; i++) {
    player->passive_skills[i] = read_uint8();
  }

  /* Read condition values */
  player->condition = read_uint16();
  player->max_condition = read_uint16();

  player->unknown_82 = read_uint8();
  player->unknown_83 = (int8_t)read_uint8();

  player->weapon_id = read_uint8();

  /* Read equipped items */
  for (int i = 0; i < 3; i++) {
    player->eq_items[i] = read_uint8();
  }

  /* Read rank and affliction */
  player->rank = read_uint8();
  advance_reader(3); // 0x89, 0x8A, 0x8B
  player->unknown_8C = read_uint8();

  advance_reader(4); // 0x8D, 0x8E, 0x8F, 0x90
  player->unknown_91 = read_uint8();
  player->affliction = read_uint8();

  advance_reader(9); // 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B

  /* Read inventory items */
  for (int i = 0; i < 32; i++) {
    read_item_rec(&player->items[i]);
  }
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
      g_game_state.players[i].unknown_8C = 6; // ???

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

// This comes from "globals", 0x2A bytes in, Each item string can be 24 bytes
static const char *item_names[] = {
    "Null Item",
    "Dollars",
    ".22 Handgun",
    ".45 Colt Pistol",
    "9mm Browning Pistol",
    "Ochoa's Rifle",
    "Elephant Gun",
    "Shotgun",
    "Uzi",
    "Clown MegaUzi",
    "Clown Mega Uzi",
    "AK-47 Assault Rifle",
    ".22 Clip",
    ".38 Clip",
    ".45 Clip (7)",
    ".45 Clip (30)",
    ".50 Clip",
    "5.56mm Clip (20)",
    "5.56mm Clip (30)",
    "7.62mm Clip (5)",
    "7.62mm Clip (18)",
    "7.62mm Clip (30)",
    "9mm Clip (13)",
    "9mm Clip (30)",
    "9mm Clip (40)",
    "Shotgun Shells",
    "Grenade",
    "Plastic Explosives",
    "TNT",
    "Gas Balloons",
    "Flamethrower",
    "Napalm Cannister",
    "Machete",
    "Meat Cleaver",
    "Carving Knife",
    "Cane Knife",
    "Scalpel",
    "Pool Cue",
    "Garcia Sunday Vest",
    "Shagreen Vest",
    "Shagreen suit",
    "Leather Vest",
    "Leather Suit",
    "Flak Vest",
    "Flak Suit",
    "Rad Suit",
    "Obeah Necklace",
    "Kevlar Vest",
    "Kevlar Suit",
    "Leather Cap",
    "Riot Helmet",
    "Kevlar Derby",
    "Flo's Chapeau",
    "Strongbox Key",
    "Jewelry",
    "Canteen",
    "Canteen, Sea Water",
    "Empty Canteen",
    "Vial of Sea Water",
    "Empty Vial",
    "DeSoto Rum",
    "DeSoto's XXX Finest",
    "Chateau Pierrot",
    "Empty Bottle",
    "Bottle of Sea Water",
    "Broken Bottle",
    "Brewhoe Nostrum",
    "Voodoo Tonic",
    "Voodoo Elixir",
    "Voodoo Cologne",
    "Guard's Journal",
    "Imelda's Letters",
    "Mario's Diary",
    "DeSoto Vault Key",
    "Clown Master Key",
    "Red Key",
    "Black Key",
    "Garcia's Key",
    "Van Gogh painting",
    "Pollack painting",
    "Ojnab painting",
    "Velvet Elvis",
    "Bust of Beethoven",
    "Mutant Badge",
    "Clown Make up",
    "Silverware",
    "Spark Plug",
    "Bandage",
    "50' Rope",
    ".38 Police Special",
    "Remington 700 Rifle",
    "M16A1 Assault Rifle",
    "MAC10",
    "Shovel",
    "Mechanic Toolkit",
    "Electronics Tools",
    "Obeah Talisman",
    "DeSoto Amulet",
    "Miami PD Badge",
    "Vial, Healing Water",
    "Vial, Sea Water",
    "Vial, alcohol",
    "Canteen, Heal Water",
    "Canteen, alcohol",
    "Bottle, Heal Water",
    "Bottle of alcohol",
    "Bottle, blue tap",
    "Bottle, green tap",
    "Bottle, violet tap",
    "Canteen, blue tap",
    "Canteen, green tap",
    "Canteen, violet tap",
    "Vial, blue tap",
    "Vial, green tap",
    "Vial, violet tap",
    "Imelda's brooch",
    "Personal Radio",
    "Rubber Boots",
};
const size_t item_names_count = sizeof(item_names) / sizeof(item_names[0]);

const char *get_item_name(int item_id)
{
  if (item_id > item_names_count) {
    return item_names[0];
  }

  return item_names[item_id];
}
