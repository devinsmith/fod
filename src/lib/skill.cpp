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

#include "skill.h"

// Active and Passive skills (not all used in the game)
static const struct skill_def skill_defs[] = {
  // ACTIVE SKILLS
  { "Medic", 0x04, 0x3F },
  { "Lockpick", 0x04, 0x00 },
  { "Forgery!!", 0x00, 0x00 },
  { "Climb", 0x02, 0x00 },
  { "Pharmacy", 0x04, 0x00 },
  { "Bomb/Alarm Disarm", 0x02, 0x00 },
  { "Mechanics", 0x02, 0x00 },
  { "Electronics!!", 0x00, 0x00 },
  { "Cryptography!!", 0x00, 0x00 },
  { "Pickpocket!!", 0x00, 0x00 },
  { "FODDY FOD", 0x00, 0x00 },
  { "Doctor", 0x04, 0x00 },
  { "*Chameleon", 0x64, 0x00 },
  { "*Shriek", 0x64, 0x00 },
  { "*Stun", 0x64, 0x00 },
  { "*Corruption", 0x64, 0x00 },
  // PASSIVE SKILLS
  { "Perception", 0x01, 0x00 },
  { "Stealth", 0x02, 0x00 },
  { "Languages", 0x01, 0x00 },
  { "Demolitions", 0x14, 0x00 },
  { "Brawling", 0x05, 0x00 },
  { "Blades", 0x0F, 0x00 },
  { "Handgun", 0x0F, 0x00 },
  { "Rifle", 0x0F, 0x00 },
  { "Auto Weapon", 0x19, 0x00 },
  { "Boating!!", 0x00, 0x00 },
  { "Gunsmith", 0x01, 0x00 },
  { "Swim", 0x02, 0x00 },
  { "Evasion", 0x0A, 0x00 },
  { "Gambling!!", 0x00, 0x00 },
  { "*Mutant Recog!!", 0x64, 0x00 },
  { "*Camouflage", 0x64, 0x00 }
};

const size_t skill_defs_count = sizeof(skill_defs) / sizeof(skill_defs[0]);

const struct skill_def *get_skill_def(int skill_id)
{
  if (skill_id > (int)skill_defs_count) {
    return &skill_defs[0];
  }

  return &skill_defs[skill_id];
}

