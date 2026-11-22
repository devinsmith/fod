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

#ifndef PLAYER_H
#define PLAYER_H

#include <assert.h> // For static_assert macro
#include <stddef.h>
#include <stdint.h>

// 6 bytes each?
struct item_rec {
  uint8_t item_id;
  uint8_t ammo_type;
  uint8_t ammo_count;
  uint8_t props[2];
  uint8_t flags; // extra flag properties.
} __attribute__((packed));

static_assert(sizeof(struct item_rec) == 6, "item_rec must be 6 bytes");


enum player_attribute {
  ATTR_STRENGTH     = 0,
  ATTR_INTELLIGENCE = 1,
  ATTR_DEXTERITY    = 2,
  ATTR_WILLPOWER    = 3,
  ATTR_APTITUDE     = 4,
  ATTR_CHARISMA     = 5,
  ATTR_LUCK         = 6
};

// Should be 332 bytes long.
struct player_rec {
  char name[14]; // 0x00-0x0D (12 chars + NUL)
  uint8_t profession; // 0x0E
  uint8_t unknown_0F; // 0x0F
  uint8_t unknown_10; // 0x10
  uint8_t unknown_11; // 0x11
  uint8_t unknown_12[6];  // 0x12-0x17

  // 0x18 - 0x22
  // 0 - Strength - 0x18
  // 1 - Intelligence - 0x19
  // 2 - Dexterity - 0x1A
  // 3 - Willpower - 0x1B
  // 4 - Aptitude - 0x1C
  // 5 - Charisma - 0x1D
  // 6 - Luck - 0x1E
  // Unknown - 0x1F
  // Unknown - 0x20
  // Unknown - 0x21
  // Unknown - 0x22
  uint8_t attributes[11]; // 0x18

  uint8_t attribute_points; // 0x23

  // Skills 0x24 - 0x43
  // Active skills come first, then passive skills.

  // ACTIVE SKILLS
  // =============
  // 0x24 = Medic
  // 0x25 = Lockpick
  // 0x26 = Forgery!!
  // 0x27 = Climb
  // 0x28 = Pharmacy
  // 0x29 = Bomb/Alarm Disarm
  // 0x2A = Mechanics
  // 0x2B = Electronics!!
  // 0x2C = Cryptography!!
  // 0x2D = Pickpocket!!
  // 0x2E = FODDY FOD
  // 0x2F = Doctor
  // 0x30 = *Chameleon
  // 0x31 = *Shriek
  // 0x32 = *Stun
  // 0x33 = *Corruption!!

  // PASSIVE SKILLS
  // ==============
  // 0x34 = Perception
  // 0x35 = Stealth
  // 0x36 = Languages
  // 0x37 = Demolitions
  // 0x38 = Brawling
  // 0x39 = Blades
  // 0x3A = Handgun
  // 0x3B = Rifle
  // 0x3C = Auto Weapon
  // 0x3D = Boating!!
  // 0x3E = Gunsmith
  // 0x3F = Swim
  // 0x40 = Evasion
  // 0x41 = Gambling!!
  // 0x42 = *Mutant Recog!!
  // 0x43 = *Camouflage

  uint8_t skills[32]; // 0x24 (Indexed by enum skill_type (see skill.h))

  uint16_t condition;        // 0x44
  uint16_t max_condition;    // 0x46

  // Padding bytes? (unknown)
  uint8_t unknown_48;      // 0x48
  uint8_t unknown_49;      // 0x49

  // Value of 0xFF = no item equipped
  // other values are indexes into items array.
  uint8_t equipped_weapon; // Item index of of equipped weapon (0x4A)
  uint8_t equipped_armor[3]; // Equipped armor items (0x4B-0x4D)

  uint8_t rank; // 0x4E
  uint8_t unknown_4F;  // 0x4F
  uint8_t gender; // 0x50
  uint8_t unknown_51; // 0x51

  uint8_t inventory_count; // 0x52
  uint8_t unknown_53[4]; // 0x53

  uint8_t unknown_57; // 0x57 (player drunk?)
  uint8_t affliction; // 0x58 - bit encoded
  /*  00000001 - Poisoned
      00000010 - Irradiated
      00000100 - Rabid
      00001000 - Envenomed
      00010000 - Condition 5
      00100000 - Condition 6
      01000000 - Condition 7
      10000000 - Mutant
   */

  uint8_t unknown_59[9];  // 0x59

  struct item_rec items[32]; // 0x62
  uint8_t unknown_122;     // 0x122
  uint8_t unknown_123;     // 0x123
  uint8_t skill_uses[32];  // 0x124-0x143
  uint8_t cmd_type;        // 0x144
  uint8_t cmd_arg;         // 0x145
  uint8_t unknown_146;     // 0x146
  uint8_t selected_item;   // 0x147

  // Contains segment:offset of level map cell
  uint32_t unknown_148;    // 0x148-0x14B
} __attribute__((packed));

static_assert(offsetof(struct player_rec, profession) == 0x0E, "profession");
static_assert(offsetof(struct player_rec, unknown_12) == 0x12, "unknown_12");
static_assert(offsetof(struct player_rec, attributes) == 0x18, "attributes");
static_assert(offsetof(struct player_rec, attribute_points) == 0x23, "attribute_points");
static_assert(offsetof(struct player_rec, skills) == 0x24, "skills");
static_assert(offsetof(struct player_rec, condition) == 0x44, "condition");
static_assert(offsetof(struct player_rec, equipped_weapon) == 0x4A, "equipped_weapon");
static_assert(offsetof(struct player_rec, gender) == 0x50, "gender");
static_assert(offsetof(struct player_rec, affliction) == 0x58, "affliction");
static_assert(offsetof(struct player_rec, items) == 0x62, "items");
static_assert(offsetof(struct player_rec, unknown_122) == 0x122, "unknown_122");
static_assert(offsetof(struct player_rec, skill_uses) == 0x124, "skill_uses");
static_assert(offsetof(struct player_rec, cmd_type) == 0x144, "cmd_type");
static_assert(offsetof(struct player_rec, cmd_arg) == 0x145, "cmd_arg");
static_assert(offsetof(struct player_rec, unknown_146) == 0x146, "unknown_146");
static_assert(sizeof(struct player_rec) == 332, "player_rec size");

bool player_has_willpower(const struct player_rec *p);
uint16_t player_armor_class(const struct player_rec *p);
int get_player_condition_status(struct player_rec *player);

bool skill_use_check(uint16_t player_idx, int skill);
void skill_up_message(struct player_rec *p, uint16_t skill, bool flag);
void inv_remove_or_set_item(uint16_t player_idx, uint16_t slot, uint16_t value);


#endif /* PLAYER_H */
