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

#ifndef SKILL_H
#define SKILL_H

#include <stdint.h>

// Enumeration of all player skills (Active and passive)
// These are offsets into the player_rec skills[] array.
enum skill_type {
  // Active
  SKILL_MEDIC             = 0x00,
  SKILL_LOCKPICK          = 0x01,
  SKILL_FORGERY           = 0x02,
  SKILL_CLIMB             = 0x03,
  SKILL_PHARMACY          = 0x04,
  SKILL_BOMB_ALARM_DISARM = 0x05,
  SKILL_MECHANICS         = 0x06,
  SKILL_ELECTRONICS       = 0x07,
  SKILL_CRYPTOGRAPHY      = 0x08,
  SKILL_PICKPOCKET        = 0x09,
  SKILL_FODDY_FOD         = 0x0A,
  SKILL_DOCTOR            = 0x0B,
  SKILL_CHAMELEON         = 0x0C,
  SKILL_SHRIEK            = 0x0D,
  SKILL_STUN              = 0x0E,
  SKILL_CORRUPTION        = 0x0F,
  // Passive
  SKILL_PERCEPTION        = 0x10,
  SKILL_STEALTH           = 0x11,
  SKILL_LANGUAGES         = 0x12,
  SKILL_DEMOLITIONS       = 0x13,
  SKILL_BRAWLING          = 0x14,
  SKILL_BLADES            = 0x15,
  SKILL_HANDGUN           = 0x16,
  SKILL_RIFLE             = 0x17,
  SKILL_AUTO_WEAPON       = 0x18,
  SKILL_BOATING           = 0x19,
  SKILL_GUNSMITH          = 0x1A,
  SKILL_SWIM              = 0x1B,
  SKILL_EVASION           = 0x1C,
  SKILL_GAMBLING          = 0x1D,
  SKILL_MUTANT_RECOG      = 0x1E,
  SKILL_CAMOUFLAGE        = 0x1F
};

#define PASSIVE_SKILL_BASE 16

// 22 (0x16) byte entry in the HDSPCT file.
struct skill_def {
  char name[20];
  uint8_t threshold; // 0x14
  uint8_t unknown;   // 0x15
} __attribute__((packed));

const struct skill_def *get_skill_def(int skill_id);

#endif /* SKILL_H */
