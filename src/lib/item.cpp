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

#include <stddef.h>

#include "item.h"

static const struct item_def item_defs[] = {
    { "Null Item", 0x20, 0x00, 0x00, 0x00 },
    { "Dollars", 0xC0, 0x00, 0x00, 0x00 },
    { ".22 Handgun", 0xC1, 0x00, 0x00, 0x00 },
    { ".45 Colt Pistol", 0xC1, 0x01, 0x00, 0x00 },
    { "9mm Browning Pistol", 0xC1, 0x02, 0x00, 0x00 },
    { "Ochoa's Rifle", 0xC1, 0x03, 0x00, 0x00 },
    { "Elephant Gun", 0xC1, 0x04, 0x00, 0x00 },
    { "Shotgun", 0xC1, 0x05, 0x00, 0x00 },
    { "Uzi", 0xC1, 0x06, 0x00, 0x00 },
    { "Clown MegaUzi", 0xC1, 0x07, 0x00, 0x00 },
    { "Clown Mega Uzi", 0xC1, 0x08, 0x00, 0x00 },
    { "AK-47 Assault Rifle", 0xC1, 0x09, 0x00, 0x00 },
    { ".22 Clip", 0xC0, 0x00, 0x00, 0x00 },
    { ".38 Clip", 0xC0, 0x00, 0x00, 0x00 },
    { ".45 Clip (7)", 0xC0, 0x00, 0x00, 0x00 },
    { ".45 Clip (30)", 0xC0, 0x00, 0x00, 0x00 },
    { ".50 Clip", 0xC0, 0x00, 0x00, 0x00 },
    { "5.56mm Clip (20)", 0xC0, 0x00, 0x00, 0x00 },
    { "5.56mm Clip (30)", 0xC0, 0x00, 0x00, 0x00 },
    { "7.62mm Clip (5)", 0xC0, 0x00, 0x00, 0x00 },
    { "7.62mm Clip (18)", 0xC0, 0x00, 0x00, 0x00 },
    { "7.62mm Clip (30)", 0xC0, 0x00, 0x00, 0x00 },
    { "9mm Clip (13)", 0xC0, 0x00, 0x00, 0x00 },
    { "9mm Clip (30)", 0xC0, 0x00, 0x00, 0x00 },
    { "9mm Clip (40)", 0xC0, 0x00, 0x00, 0x00 },
    { "Shotgun Shells", 0xC0, 0x00, 0x00, 0x00 },
    { "Grenade", 0xC1, 0x0A, 0x00, 0x00 },
    { "Plastic Explosives", 0xC1, 0x0B, 0x00, 0x00 },
    { "TNT", 0xC1, 0x0C, 0x00, 0x00 },
    { "Gas Balloons", 0xC0, 0x00, 35, 0x07 },
    { "Flamethrower", 0xC1, 0x0D, 0x00, 0x00 },
    { "Napalm Cannister", 0xC0, 0x00, 0x00, 0x00 },
    { "Machete", 0xC1, 0x0E, 0x00, 0x00 },
    { "Meat Cleaver", 0xC1, 0x0F, 0x00, 0x00 },
    { "Carving Knife", 0xC1, 0x10, 0x00, 0x00 },
    { "Cane Knife", 0xC1, 0x11, 0x00, 0x00 },
    { "Scalpel", 0xC1, 0x12, 0x00, 0x00 },
    { "Pool Cue", 0xC1, 0x13, 0x00, 0x00 },
    { "Garcia Sunday Vest", 0xC6, 0x02, 0x00, 0x00 },
    { "Shagreen Vest", 0xC6, 0x02, 0x00, 0x00 },
    { "Shagreen suit", 0xC2, 0x03, 0x00, 0x00 },
    { "Leather Vest", 0xC6, 0x01, 0x00, 0x00 },
    { "Leather Suit", 0xC2, 0x02, 0x00, 0x00 },
    { "Flak Vest", 0xC6, 0x03, 0x00, 0x00 },
    { "Flak Suit", 0xC2, 0x04, 0x00, 0x00 },
    { "Rad Suit", 0xC2, 0x05, 0x00, 0x00 },
    { "Obeah Necklace", 0xCA, 0x01, 0x00, 0x00 },
    { "Kevlar Vest", 0xC6, 0x04, 0x00, 0x00 },
    { "Kevlar Suit", 0xC2, 0x05, 0x00, 0x00 },
    { "Leather Cap", 0xCA, 0x01, 0x00, 0x00 },
    { "Riot Helmet", 0xCA, 0x02, 0x00, 0x00 },
    { "Kevlar Derby", 0xCA, 0x02, 0x00, 0x00 },
    { "Flo's Chapeau", 0xCA, 0x03, 0x00, 0x00 },
    { "Strongbox Key", 0xC0, 0x00, 35, 0x07 },
    { "Jewelry", 0xC0, 0x00, 20, 0x07 },
    { "Canteen", 0xC0, 0x00, 0x00, 0x00 },
    { "Canteen, Sea Water", 0xC0, 0x00, 0x00, 0x00 },
    { "Empty Canteen", 0xC0, 0x00, 0x00, 0x00 },
    { "Vial of Sea Water", 0xC0, 0x00, 0x00, 0x00 },
    { "Empty Vial", 0xC0, 0x00, 0x00, 0x00 },
    { "DeSoto Rum", 0xC4, 0x00, 0x00, 0x00 },
    { "DeSoto's XXX Finest", 0xC4, 0x00, 0x00, 0x00 },
    { "Chateau Pierrot", 0xC4, 0x00, 0x00, 0x00 },
    { "Empty Bottle", 0xC0, 0x00, 0x00, 0x00 },
    { "Bottle of Sea Water", 0xC0, 0x00, 0x00, 0x00 },
    { "Broken Bottle", 0xC0, 0x00, 0x00, 0x00 },
    { "Brewhoe Nostrum", 0xC0, 0x00, 10, 0x00 },
    { "Voodoo Tonic", 0xC0, 0x00, 5, 0x00 },
    { "Voodoo Elixir", 0xC0, 0x00, 5, 0x00 },
    { "Voodoo Cologne", 0xC0, 0x00, 0x00, 0x00 },
    { "Guard's Journal", 0xC0, 0x00, 0x23, 0x07 },
    { "Imelda's Letters", 0xC0, 0x00, 0x00, 0x00 },
    { "Mario's Diary", 0xC0, 0x00, 0x19, 0x07 },
    { "DeSoto Vault Key", 0xC0, 0x00, 0x23, 0x07 },
    { "Clown Master Key", 0xC0, 0x00, 0x2D, 0x07 },
    { "Red Key", 0xC0, 0x00, 0x2D, 0x07 },
    { "Black Key", 0xC0, 0x00, 0x2D, 0x07 },
    { "Garcia's Key", 0xC0, 0x00, 0x32, 0x07 },
    { "Van Gogh painting", 0xC0, 0x00, 0x23, 0x07 },
    { "Pollack painting", 0xC0, 0x00, 0x23, 0x07 },
    { "Ojnab painting", 0xC0, 0x00, 0x23, 0x07 },
    { "Velvet Elvis", 0xC0, 0x00, 0x55, 0x07 },
    { "Bust of Beethoven", 0xC0, 0x00, 0x00, 0x00 },
    { "Mutant Badge", 0xDC, 0x00, 0x00, 0x03 },
    { "Clown Make up", 0xC0, 0x00, 0x4B, 0x07 },
    { "Silverware", 0xC0, 0x00, 0x00, 0x00 },
    { "Spark Plug", 0xC0, 0x00, 0x00, 0x00 },
    { "Bandage", 0xC0, 0x00, 0x00, 0x00 },
    { "50' Rope", 0xC0, 0x00, 0x00, 0x00 },
    { ".38 Police Special", 0xC1, 0x14, 0x00, 0x00 },
    { "Remington 700 Rifle", 0xC1, 0x15, 0x00, 0x00 },
    { "M16A1 Assault Rifle", 0xC1, 0x16, 0x00, 0x00 },
    { "MAC10", 0xC1, 0x17, 0x00, 0x00 },
    { "Shovel", 0xC0, 0x00, 0x00, 0x00 },
    { "Mechanic Toolkit", 0xC0, 0x00, 0x00, 0x00 },
    { "Electronics Tools", 0xC0, 0x00, 0x00, 0x00 },
    { "Obeah Talisman", 0xDC, 0x00, 0x00, 0x01 },
    { "DeSoto Amulet", 0xDC, 0x00, 0x00, 0x00 },
    { "Miami PD Badge", 0xDC, 0x00, 0x00, 0x02 },
    { "Vial, Healing Water", 0xC8, 0x00, 0x00, 0x00 },
    { "Vial, Sea Water", 0xC0, 0x00, 0x00, 0x00 },
    { "Vial, alcohol", 0xC0, 0x00, 0x00, 0x00 },
    { "Canteen, Heal Water", 0xC8, 0x00, 0x00, 0x00 },
    { "Canteen, alcohol", 0xC0, 0x00, 0x00, 0x00 },
    { "Bottle, Heal Water", 0xC8, 0x00, 0x00, 0x00 },
    { "Bottle of alcohol", 0xC0, 0x00, 0x00, 0x00 },
    { "Bottle, blue tap", 0xC0, 0x00, 0x00, 0x00 },
    { "Bottle, green tap", 0xC0, 0x00, 0x00, 0x00 },
    { "Bottle, violet tap", 0xC8, 0x00, 0x00, 0x00 },
    { "Canteen, blue tap", 0xC0, 0x00, 0x00, 0x00 },
    { "Canteen, green tap", 0xC0, 0x00, 0x00, 0x00 },
    { "Canteen, violet tap", 0xC8, 0x00, 0x00, 0x00 },
    { "Vial, blue tap", 0xC0, 0x00, 0x00, 0x00 },
    { "Vial, green tap", 0xC0, 0x00, 0x00, 0x00 },
    { "Vial, violet tap", 0xC8, 0x00, 0x00, 0x00 },
    { "Imelda's brooch", 0xC0, 0x00, 0x23, 0x07 },
    { "Personal Radio", 0xC0, 0x00, 0x23, 0x07 },
    { "Rubber Boots", 0xC0, 0x00, 0x23, 0x07 },
};

const size_t item_defs_count = sizeof(item_defs) / sizeof(item_defs[0]);

const struct item_def *get_item_def(int item_id)
{
  if (item_id > (int)item_defs_count) {
    return &item_defs[0];
  }

  return &item_defs[item_id];
}

// KEH: DSEG 0xCFC8
// This comes from the weapons file.
static const struct weapon_type weapon_types[] = {
    { .fire_msg = "pops a shot at", .unknown_14 = 1, .unknown_15 = 0, .unknown_16 = 4,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 99, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 5,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x05A2
  },
  { .fire_msg = "fires a round at", .unknown_14 = 2, .unknown_15 = 0, .unknown_16 = 6,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 99, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 1, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 5,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x05B4
  },
  { .fire_msg = "snaps off a shot at", .unknown_14 = 2, .unknown_15 = 0, .unknown_16 = 5,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 99, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 5,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x05C6
  },
  { .fire_msg = "booms a shot at", .unknown_14 = 4, .unknown_15 = 0, .unknown_16 = 8,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 98, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 5, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 2, .ammo_list_offset = 0x05D8
  },
  { .fire_msg = "thunders", .unknown_14 = 4, .unknown_15 = 0, .unknown_16 = 8,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 97, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 5, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x05FC
  },
  { .fire_msg = "blasts at", .unknown_14 = 4, .unknown_15 = 0, .unknown_16 = 8,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 99, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 5, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x060E
  },
  { .fire_msg = "snaps a shot at", .unknown_14 = 0, .unknown_15 = 0, .unknown_16 = 100,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 98, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 5, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 3, .ammo_list_offset = 0x0620
  },
  { .fire_msg = "snaps a shot at", .unknown_14 = 0, .unknown_15 = 0, .unknown_16 = 100,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 0, .unknown_1A = 95, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 5, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 4, .ammo_list_offset = 0x0656
  },
  { .fire_msg = "launches death at", .unknown_14 = 0, .unknown_15 = 0, .unknown_16 = 100,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 0, .unknown_1A = 95, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 5, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 4, .ammo_list_offset = 0x069E
  },
  { .fire_msg = "snaps a shot at", .unknown_14 = 4, .unknown_15 = 0, .unknown_16 = 8,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 98, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 5, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 2, .ammo_list_offset = 0x06E6
  },
  { .fire_msg = "chucks death at", .unknown_14 = 0, .unknown_15 = 116, .unknown_16 = 40,
    .unknown_17 = 0, .flags = 2, .unknown_19 = 50, .unknown_1A = 94, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x070A
  },
  { .fire_msg = "tosses death at", .unknown_14 = 30, .unknown_15 = 0, .unknown_16 = 50,
    .unknown_17 = 0, .flags = 2, .unknown_19 = 50, .unknown_1A = 94, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x071C
  },
  { .fire_msg = "hurls death at", .unknown_14 = 10, .unknown_15 = 0, .unknown_16 = 30,
    .unknown_17 = 0, .flags = 2, .unknown_19 = 50, .unknown_1A = 97, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x072E
  },
  { .fire_msg = "blasts flame at", .unknown_14 = 0, .unknown_15 = 0, .unknown_16 = 100,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 0, .unknown_1A = 96, .unknown_1B = { 0, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x0740
  },
  { .fire_msg = "slashes at", .unknown_14 = 10, .unknown_15 = 0, .unknown_16 = 18,
    .unknown_17 = 0, .flags = 0, .unknown_19 = 65, .unknown_1A = 99, .unknown_1B = { 0, 3, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 1,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x0752
  },
  { .fire_msg = "chops at", .unknown_14 = 7, .unknown_15 = 0, .unknown_16 = 15,
    .unknown_17 = 0, .flags = 0, .unknown_19 = 60, .unknown_1A = 99, .unknown_1B = { 0, 3, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 0,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x0764
  },
  { .fire_msg = "slices at", .unknown_14 = 6, .unknown_15 = 0, .unknown_16 = 13,
    .unknown_17 = 0, .flags = 0, .unknown_19 = 55, .unknown_1A = 96, .unknown_1B = { 0, 3, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 0,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x0776
  },
  { .fire_msg = "chops at", .unknown_14 = 7, .unknown_15 = 0, .unknown_16 = 15,
    .unknown_17 = 0, .flags = 0, .unknown_19 = 55, .unknown_1A = 99, .unknown_1B = { 32, 3, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 0,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x0788
  },
  { .fire_msg = "gingerly swipes at", .unknown_14 = 9, .unknown_15 = 0, .unknown_16 = 20,
    .unknown_17 = 0, .flags = 0, .unknown_19 = 35, .unknown_1A = 90, .unknown_1B = { 30, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x079A
  },
  { .fire_msg = "bashes", .unknown_14 = 3, .unknown_15 = 0, .unknown_16 = 10,
    .unknown_17 = 0, .flags = 0, .unknown_19 = 65, .unknown_1A = 85, .unknown_1B = { 4, 3, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 0,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x07AC
  },
  { .fire_msg = "snaps a shot off at", .unknown_14 = 2, .unknown_15 = 0, .unknown_16 = 4,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 99, .unknown_1B = { 98, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 5,
    .unknown_2F = { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x07BE
  },
  { .fire_msg = "fires at", .unknown_14 = 4, .unknown_15 = 0, .unknown_16 = 8,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 99, .unknown_1B = { 30, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 5, 0, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 1, .ammo_list_offset = 0x07D0
  },
  { .fire_msg = "snaps off a shot at", .unknown_14 = 4, .unknown_15 = 0, .unknown_16 = 8,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 98, .unknown_1B = { 4, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 5, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 2, .ammo_list_offset = 0x07E2
  },
  { .fire_msg = "snaps off a shot at", .unknown_14 = 3, .unknown_15 = 0, .unknown_16 = 6,
    .unknown_17 = 0, .flags = 3, .unknown_19 = 50, .unknown_1A = 98, .unknown_1B = { 112, 0, 0 },
    .unknown_1E = 5, .unknown_1F = { 0, 0, 0 }, .unknown_22 = 3,
    .unknown_23 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .unknown_2E = 0,
    .unknown_2F = { 0, 5, 0, 0, 0, 0, 0, 0, 0 },
    .ammo_count = 5, .ammo_list_offset = 0x0806
  }
};

const size_t weapon_types_count = sizeof(weapon_types) / sizeof(weapon_types[0]);

const struct weapon_type *get_weapon_type(int item_id)
{
  if (item_id > (int)weapon_types_count) {
    return &weapon_types[0];
  }

  return &weapon_types[item_id];
}

