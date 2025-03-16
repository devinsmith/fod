/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2025 Devin Smith <devin@devinsmith.net>
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

#include "common/bufio.h"
#include "common/hexdump.h"
#include "resource.h"
#include "tables.h"
#include "vga.h"

static const int GAME_WIDTH = 320;
static const int GAME_HEIGHT = 200;

// FOD does most of it's work in a large allocated buffer
// DSEG:0x042D
static unsigned char *scratch;
static unsigned char *scratch_01; // DSEG:0x029A
static unsigned char *scratch_02; // DSEG:0x0292
static unsigned char *scratch_03; // DSEG:0x0296

static void title_draw(const struct resource *title)
{
  const uint16_t *src = (const uint16_t *)title->bytes;
  uint16_t *dest = (uint16_t *)vga_memory();

  // Processes 16000 compressed pixel groups
  // (16000 * 2 bytes for source -> 16000 * 4 bytes to destination)
  // Every short (2 bytes) defines 4 bytes of the output.
  for (int i = 0; i < 16000; i++) {
    uint16_t src_pixel = *src++;

    // Extract components
    uint8_t low_byte = src_pixel & 0xFF;
    uint8_t high_byte = src_pixel >> 8;

    // Rotate left by 4, right by 12
    uint16_t rotated_pixel = (src_pixel << 4) | (src_pixel >> 12);
    uint8_t rotated_low = rotated_pixel & 0xFF;

    // Rotated low byte goes to high.
    uint16_t trans1 = (rotated_low << 8) | low_byte;
    uint16_t trans2 = (rotated_pixel & 0xFF00) | high_byte;

    *dest++ = trans1;
    *dest++ = trans2;
  }
}

static void do_title()
{
  struct resource *title_res = resource_load(RESOURCE_TITLE);

  hexdump(title_res->bytes, 32);
  title_draw(title_res);
  vga_update();

  vga_waitkey();

  resource_release(title_res);
}

static uint16_t word_35E0 = 0;
static uint16_t word_35E2 = 0;

static void sub_1778(uint8_t al, int i, int j)
{
  printf("%s - 0x%02X %d %d\n", __func__, al, i, j);

  uint16_t di = i << 4;
//  uint16_t di = lookup[di];

  di += (j << 2);
//  unsigned char *es = scratch;

//  int ax = al << 5;

}

// seg000:0090
//
void sub_90()
{
}

// seg000:1439
// This is approximately what sub_1439 would do.
void game_mem_alloc()
{
  // Fountain of Dreams does most of its work within this scratch
  // buffer. I'm not sure it really needs to be this big but that's how much
  // is allocated in the disassembly.
  scratch = malloc(286544); // 286,544 bytes.
  if (scratch == NULL) {
    fprintf(stderr, "You do not have enough memory to run Fountain of Dreams.\n");
    exit(0);
  }

  scratch_01 = scratch + 0x07d0;
  scratch_02 = scratch_01 + 0x043A;
  scratch_03 = scratch_02 + 0x01CA;

  setup_tables();

  sub_90();
}

/* seg000:0x14FF */
void sub_14FF(struct resource *r)
{
  word_35E0 = 0;
  word_35E2 = 0;

  struct buf_rdr *rdr = buf_rdr_init(r->bytes, r->len);

  for (int j = 0; j < 25; j++) {
    for (int i = 0; i < 40; i++) {
      uint8_t al = buf_get8(rdr);
      if (al != 0) {
        sub_1778(al, i, j);
      }
    }
  }

  buf_rdr_free(rdr);
}

int main(int argc, char *argv[])
{
  if (!rm_init()) {
    fprintf(stderr, "Failed to initialize resource manager, exiting!\n");
    return 1;
  }

  // Register VGA driver.
  video_setup();

  if (vga_initialize(GAME_WIDTH, GAME_HEIGHT) != 0) {
    return 1;
  }

  game_mem_alloc();

  do_title();

  struct resource *borders = resource_load_sz(RESOURCE_BORDERS, 0x1388, 0x3E8);

  hexdump(borders->bytes, 64);

  sub_14FF(borders);

  free(scratch);

  vga_end();

  return 0;
}
