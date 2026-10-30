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

#ifndef ANIM_H
#define ANIM_H

#include <stdbool.h>
#include <stdint.h>

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* 12-byte record at DS:BEC2 + 12*i */
struct anim_entry {
  struct ui_region *region;   /* +0x00: copied from word_1E74[i + bias]   */
  uint16_t pos;          /* +0x02: u16 read from anim data at offset 1        */
  unsigned char *script; /* +0x04/+0x06: far ptr to decompressed anim buffer  */
  uint16_t field_8;      /* +0x08: never touched here                         */
  bool     active;       /* +0x0A: set to 1                                   */
  uint8_t  field_B;      /* +0x0B: never touched here                         */
};

extern uint16_t g_wanted_id[];
extern uint16_t g_cached_id[];
extern const char *level_ani_file;
extern unsigned char level_ani_bytes[];
extern struct ui_region region_1CEC;
extern struct anim_entry g_anim_entry[];

void sub_B360(int stat, char *entry);
void anim_update(bool force);

#ifdef __cplusplus
}
#endif

#endif /* ANIM_H */
