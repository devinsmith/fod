//
// Fountain of Dreams - Reverse Engineering Project
//
// Copyright (c) 2025 Devin Smith <devin@devinsmith.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdio>

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
