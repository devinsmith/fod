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

#if 0
static void decode(uint16_t src)
{
  uint8_t ch = src >> 8;

  uint16_t bx = src;
  bx = (bx << 4) | (bx >> 12); // Rotate left by 4 bits

  // Rotated low byte goes to high.
  uint16_t trans1 = ((bx & 0x00FF) << 8) | (src & 0x00FF);
  uint16_t trans2 = (bx & 0xFF00) | ch;

  printf("SRC: 0x%04X (BX: 0x%04X) T1: 0x%04X, T2: 0x%04X\n",
         src, bx, trans1, trans2);
}
#endif


static void title_draw(const struct resource *title)
{
//  unsigned char *src = title->bytes;
//  uint8_t *framebuffer = vga_memory();
  hexdump(title->bytes, 32);


#if 0
  for (int i = 0; i < title->len; i++) {

  }
#endif
}

static void do_title()
{
  struct resource *title_res = resource_load(RESOURCE_TITLE);

  title_draw(title_res);

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
