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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "cursor.h"
#include "platform.h"
#include "sfx.h"

#define PIT_INITIAL_DIVISOR 2

struct sfx_state {
  bool disabled; // DSEG:0xB9DE, pressing F10 is supposed to toggle this.
  bool playing; // DSEG:0xB9DF
  uint8_t frames_left; // DSEG:B9E0, ticks remaining in current segment.
  uint8_t frames_len;  // DSEG:B9E1, reload value for frames_left
  uint8_t repeats_left; // DSEG:B9E2 extra replays of the current segment
  uint16_t div_base; // DSEG:B9E3  segment start divisor (reload on repeat)
  uint16_t div_cur; // DSEG:B9E5  current PIT ch2 divisor
  uint16_t  div_delta; // DSEG:B9E7  added to div_cur each tick
  uint16_t  div_delta2; // DSEG:B9E9 added to div_delta each tick
  struct data_cursor stream;
};

static struct sfx_state sfx = { 0 };

/* Sound stream opcodes (sub_1119E) */
enum {
  SFX_OP_REPEAT  = 0x81,   /* u8 n                            replay the NEXT segment n more times */
  SFX_OP_SEGMENT = 0x83,   /* u8 len, u16 div, u16 d, u16 d2  divisor sweep, len+1 ticks           */
  SFX_OP_END     = 0x84    /*                                 speaker off, playing = 0             */
};

static unsigned char sample_B9EF[] = {
  0x81, 0x00, 0x83, 0x0C, 0xBC, 0x1B, 0x70, 0x0C, 0x30, 0x06, 0x84
};

static unsigned char sample_B9FA[] = {
  0x81, 0x00, 0x83, 0x02, 0xBC, 0x1F, 0x70, 0x01, 0x30, 0x06, 0x84
};

static unsigned char sample_BA05[] = {
  0x81, 0x00, 0x83, 0x08, 0xBC, 0x00, 0xC0, 0x00, 0x60, 0x00, 0x84
};

static unsigned char sample_BA10[] = {
  0x81, 0x00, 0x83, 0x1E, 0xFF, 0x01, 0x01, 0x00, 0x00, 0x00,
  0x83, 0x08, 0xFF, 0x0F, 0x37, 0x00, 0x30, 0x06,
  0x83, 0x14, 0xBC, 0x1B, 0x70, 0x0C, 0x30, 0x06,
  0x84
};

// 0xBA2B
// Sound effect for bumping into an object (wall/tree)
static unsigned char bump_sfx[] = {
  0x81, 0x00, 0x83, 0x08, 0x00, 0x12, 0x80, 0x10, 0x00, 0x00, 0x84
};

static unsigned char sample_BA36[] = {
  0x81, 0x00,
  0x83, 0x05, 0xC0, 0x01, 0xFF, 0xFF, 0x00, 0x00,
  0x83, 0x0A, 0xC0, 0x05, 0x00, 0x00, 0x00, 0x00,
  0x83, 0x05, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x83, 0x0A, 0xC0, 0x05, 0x00, 0x00, 0x00, 0x00,
  0x83, 0x05, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x83, 0x0A, 0xC0, 0x05, 0x00, 0x00, 0x00, 0x00,
  0x84
};

static unsigned char sample_BA69[] = {
  0x81, 0x00,
  0x83, 0x03, 0xCD, 0x10, 0x0A, 0x00, 0x00, 0x00,
  0x84
};

struct sample {
  unsigned char *bytes;
  size_t len;
};

// The order of these match KEH.EXE
// KEH: DSEG:0xBA74
static struct sample samples[] = {
  { bump_sfx, sizeof(bump_sfx) },
  { sample_BA69, sizeof(sample_BA69) },
  { sample_BA36, sizeof(sample_BA36) },
  { sample_B9EF, sizeof(sample_B9EF) },
  { sample_BA05, sizeof(sample_BA05) },
  { sample_BA10, sizeof(sample_BA10) },
  { sample_B9FA, sizeof(sample_B9FA) }
};

#define SFX_MAX_INDEX (sizeof(samples) / sizeof(samples[0]))

// Plays a sound segment
// KEH: seg001:119E
static void play_sound_segment(void)
{
  if (!sfx.playing) {
    return;
  }

  if (sfx.frames_left != 0) {       /* loc_11215: mid-segment, keep sweeping */
    sfx.div_cur   += sfx.div_delta;
    sfx.div_delta += sfx.div_delta2;
    sfx.frames_left--;
  } else if (sfx.repeats_left != 0) {   /* loc_11202: replay the segment */
    sfx.repeats_left--;
    sfx.div_cur     = sfx.div_base;
    sfx.frames_left = sfx.frames_len;
    /* NOTE: div_delta / div_delta2 are NOT reset here, so a repeat starts
     * from the base divisor but continues the already-advanced delta.
     * Faithful to the original; only matters if delta/delta2 != 0. */
  } else {                                         /* loc_111BC: fetch next command */
    struct data_cursor c = sfx.stream;             /* mov si, word_1CC2D (committed only on 0x83) */
    for (;;) {
      /* TODO: original has no end check; a stream missing 0x84 runs off the
       * end. Treat "cursor out of bounds" as SFX_OP_END if cursor_t can tell you. */
      uint8_t op = cursor_read_u8(&c);

      if (op == SFX_OP_END) {                /* 84h */
        hw_speaker_set(false);                  /* in 61h ; and 0FCh ; out 61h */
        sfx.playing = false;
        return;                            /* no divisor write on this tick */
      }

      if (op == SFX_OP_SEGMENT) {            /* 83h */
        sfx.frames_left = sfx.frames_len = cursor_read_u8(&c);
        sfx.div_base    = sfx.div_cur    = cursor_read_u16(&c);
        sfx.div_delta   = cursor_read_u16(&c);
        sfx.div_delta2  = cursor_read_u16(&c);
        sfx.stream = c;
        break;                             /* -> loc_11228 */
      }

      if (op == SFX_OP_REPEAT) {             /* 81h */
        sfx.repeats_left = cursor_read_u8(&c);
        continue;
      }
    }
  }

  hw_speaker_tone(sfx.div_cur);           /* loc_11228: out 42h lo, out 42h hi */
}

// Play an indexed sound.
// KEH: seg001:1130
void play_sound(int snd_num)
{
  if (sfx.disabled) {
    return;
  }

  if (snd_num > SFX_MAX_INDEX) {
    return;
  }

  sfx.stream.base = samples[snd_num].bytes;
  sfx.stream.offset = 0;
  sfx.frames_left  = 0;
  sfx.frames_len = 0;
  sfx.repeats_left = 0;
  sfx.div_base = 0;
  sfx.div_cur = 0;
  sfx.div_delta = 0;
  sfx.div_delta2 = 0;

  hw_speaker_set(true);                            /* in/or 3/out 61h */
  hw_speaker_tone(PIT_INITIAL_DIVISOR);
  sfx.playing = true;

  do {
    hw_pace();
    vga_update();

    play_sound_segment();
  } while (sfx.playing);
}
