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

#include "common/hexdump.h"
#include "resource.h"
#include "vga.h"

static const int GAME_WIDTH = 320;
static const int GAME_HEIGHT = 200;

static void title_draw(const struct resource *title)
{
  const uint16_t *src = (const uint16_t *)title->bytes;
  uint16_t *dest = (uint16_t *)vga_memory();

  // Processes 1600 compressed pixel groups
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

  do_title();

  vga_end();

  return 0;
}
