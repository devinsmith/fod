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

#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>

#include "vga.h"

#define WIN_WIDTH 640
#define WIN_HEIGHT 400
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

static SDL_Window *main_window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;

// Fountain of Dreams uses 16 colors in total, but the palette is arranged
// so that the colors are duplicated every 16 entries. It's probably done this
// way for compression.

// 0x6B2:0x0CD2 - 0x6B2:0xCF2
static const unsigned char fod_vga_palette[16][3] = {
  { 0x00, 0x00, 0x00 },    // 0x00 - 0x00 (BLACK)
  { 0x00, 0x00, 0x27 },
  { 0x03, 0x25, 0x03 },
  { 0x00, 0x1B, 0x1B },
  { 0x20, 0x00, 0x00 },
  { 0x1B, 0x00, 0x1B },
  { 0x20, 0x1B, 0x05 },
  { 0x26, 0x26, 0x26 },
  { 0x10, 0x10, 0x10 },
  { 0x00, 0x00, 0x38 },
  { 0x00, 0x3F, 0x00 },
  { 0x00, 0x3F, 0x3F },
  { 0x3A, 0x20, 0x10 },
  { 0x3F, 0x00, 0x3F },
  { 0x3F, 0x3F, 0x00 },
  { 0x3F, 0x3F, 0x3F }     // 0xF0 - 0xFF  (WHITE)
};

// These are indexed by SDL_Keycode
static const uint16_t normal_scancodes[] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x1E61, 0x3062, // ....ab
  0x2E63, 0x2064, 0x1265, 0x2166, 0x2267, 0x2368, // cedfgh
  0x1769, 0x246A, 0x256B, 0x266C, 0x326D, 0x316E, // ijklmn
  0x186F, 0x1970, 0x1071, 0x1372, 0x1F73, 0x1474, // opqrst
  0x1675, 0x2F76, 0x1177, 0x2D78, 0x1579, 0x2C7A, // uvwxyz
  0x0231, 0x0332, 0x0433, 0x0534, 0x0635, 0x0736, // 123456
  0x0837, 0x0938, 0x0A39, 0x0B40, 0x1C0D, 0x011B, // 7890<Enter><Esc>
  0x0E08, 0x0F09, 0x3920, 0x0C2D, 0x0D3D, 0x1A5B, // <Backspace><Tab><Space>-=[
  0x1B5D, 0x2B5C, 0x0000, 0x273B, 0x2827, 0x2960, // ]\.;'`
  0x332C, 0x342E, 0x352F, 0x0000, 0x0000, 0x0000, // ,./...
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F3-F8
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F9-ScrollLock
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Pause-End
  0x0000, 0x4D00, 0x4B00, 0x5000, 0x4800          // Arrow keys
};

static const uint16_t shifted_scancodes[] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x1E41, 0x3042, // ....ab
  0x2E43, 0x2044, 0x1245, 0x2146, 0x2247, 0x2348, // cedfgh
  0x1749, 0x244A, 0x254B, 0x264C, 0x324D, 0x314E, // ijklmn
  0x184F, 0x1950, 0x1051, 0x1352, 0x1F53, 0x1454, // opqrst
  0x1655, 0x2F56, 0x1157, 0x2D58, 0x1559, 0x2C5A, // uvwxyz
  0x0221, 0x0340, 0x0423, 0x0524, 0x0625, 0x075E, // 123456
  0x0826, 0x092A, 0x0A28, 0x0B29, 0x1C0D, 0x011B, // 7890<Enter><Esc>
  0x0E08, 0x0F00, 0x3920, 0x0C5F, 0x0D2B, 0x1A7B, // <Backspace><Tab><Space>-=[
  0x1B7D, 0x2B7C, 0x0000, 0x273A, 0x2822, 0x297E, // ]\.;'`
  0x333C, 0x343E, 0x353F, 0x0000, 0x0000, 0x0000, // ,./...
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F3-F8
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F9-ScrollLock
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Pause-End
  0x0000, 0x4D36, 0x4B34, 0x5032, 0x4838          // Arrow keys
};

// Some keys are missing for Control-[key]
static const uint16_t ctrl_scancodes[] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x1E01, 0x3002, // ....ab
  0x2E03, 0x2004, 0x1205, 0x2106, 0x2207, 0x2308, // cedfgh
  0x1709, 0x240A, 0x250B, 0x260C, 0x320D, 0x310E, // ijklmn
  0x180F, 0x1910, 0x1011, 0x1312, 0x1F13, 0x1414, // opqrst
  0x1615, 0x2F16, 0x1117, 0x2D18, 0x1519, 0x2C1A, // uvwxyz
  0x0000, 0x0300, 0x0000, 0x0000, 0x0000, 0x071E, // 123456
  0x0000, 0x0000, 0x0000, 0x0000, 0x1C0A, 0x011B, // 7890<Enter><Esc>
  0x0E7F, 0x9400, 0x3920, 0x0C1F, 0x0000, 0x1A1B, // <Backspace><Tab><Space>-=[
  0x1B1D, 0x2B1C, 0x0000, 0x0000, 0x0000, 0x0000, // ]\.;'`
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // ,./...
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F3-F8
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F9-ScrollLock
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Pause-End
  0x0000, 0x7400, 0x7300, 0x9100, 0x8D00          // Arrow keys
};

static const uint16_t alt_scancodes[] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x1E00, 0x3000, // ....ab
  0x2E00, 0x2000, 0x1200, 0x2100, 0x2200, 0x2300, // cedfgh
  0x1700, 0x2400, 0x2500, 0x2600, 0x3200, 0x3100, // ijklmn
  0x1800, 0x1900, 0x1000, 0x1300, 0x1F00, 0x1400, // opqrst
  0x1600, 0x2F00, 0x1100, 0x2D00, 0x1500, 0x2C00, // uvwxyz
  0x7800, 0x7900, 0x7A00, 0x7B00, 0x7C00, 0x7D00, // 123456
  0x7E00, 0x7F00, 0x8000, 0x8100, 0xA600, 0x0100, // 7890<Enter><Esc>
  0x0E00, 0xA500, 0x3920, 0x8200, 0x8300, 0x1A00, // <Backspace><Tab><Space>-=[
  0x1B00, 0x2600, 0x0000, 0x2700, 0x0000, 0x0000, // ]\.;'`
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // ,./...
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F3-F8
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // F9-ScrollLock
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Pause-End
  0x0000, 0x9D00, 0x9B00, 0xA000, 0x9800          // Arrow keys
};

int
display_start(int game_width, int game_height)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "SDL could not initialize. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  if ((main_window = SDL_CreateWindow("Fountain of Dreams",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    WIN_WIDTH, WIN_HEIGHT,
    SDL_WINDOW_RESIZABLE)) == NULL) {
    fprintf(stderr, "Main window could not be created. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  if ((renderer = SDL_CreateRenderer(main_window, -1, 0)) == NULL) {
    fprintf(stderr, "Main renderer could not be created. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  if ((surface = SDL_CreateRGBSurface(SDL_SWSURFACE, game_width, game_height,
       8 /* bpp */, /* RGBA masks */ 0, 0, 0, 0)) == NULL) {
    fprintf(stderr, "8 bit surface could not be created. SDL Error: %s\n",
      SDL_GetError());
    return -1;
  }

  // Populate full 256-color palette
  SDL_Color colors[256];
  for (int i = 0; i < 256; i++) {
    int base_color = (i & 0xF0) >> 4;  // Extract upper nibble
    colors[i].r = (Uint8)(fod_vga_palette[base_color][0] * 255 / 63);
    colors[i].g = (Uint8)(fod_vga_palette[base_color][1] * 255 / 63);
    colors[i].b = (Uint8)(fod_vga_palette[base_color][2] * 255 / 63);
    colors[i].a = 255;
  }

  if (SDL_SetPaletteColors(surface->format->palette, colors, 0, 256) != 0) {
    fprintf(stderr, "Failed to set palette. SDL Error: %s\n", SDL_GetError());
    return -1;
  }

  return 0;
}

void
display_end(void)
{
  if (surface != NULL) {
    SDL_FreeSurface(surface);
  }

  if (renderer != NULL) {
    SDL_DestroyRenderer(renderer);
  }

  if (main_window != NULL) {
    SDL_DestroyWindow(main_window);
  }
  SDL_Quit();
}

void
display_update(void)
{
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(texture);
}

static uint8_t translate_key(SDL_Event *e)
{
  const SDL_KeyboardEvent *ke = &e->key;
  const SDL_Keysym *ksym = &ke->keysym;
  uint8_t keyCode = 0;

  if ((ksym->sym & SDLK_SCANCODE_MASK) == 0) {
    keyCode = (uint8_t)ksym->sym;
    if (ksym->mod & KMOD_SHIFT) {
      keyCode -= 0x20;
    }
  }

  return keyCode;
}

uint8_t waitkey()
{
  SDL_Event event;
  uint8_t key = 0;
  bool done = false;

  while (!done) {

    SDL_WaitEvent(&event);
    switch (event.type) {
    case SDL_QUIT:
      exit(0);
      break;

    case SDL_KEYDOWN:
      key = translate_key(&event);
      if (key != 0) {
        done = true;
      }

      break;
    }
  }

  return key;
}

static uint8_t *
get_fb_mem()
{
  return surface->pixels;
}

static uint16_t
shifted(const SDL_Keysym *key)
{
  if (key->sym >= 'a' && key->sym <= 'z') {
    return key->sym - 0x20;
  }

  // Unhandled shift key.
  return key->sym;
}

static uint8_t pollkey(unsigned int ms)
{
  SDL_Event event;
  uint8_t key = 0;

//  if (SDL_WaitEventTimeout(&event, ms)) {

  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
    case SDL_QUIT:
      exit(0);
      break;

    case SDL_KEYDOWN:
      key = translate_key(&event);
      break;
    }
  }

  return key;
}

static int handle_key(SDL_Event *e)
{
  const SDL_KeyboardEvent *ke = &e->key;
  const SDL_Keysym *ksym = &ke->keysym;
  const uint16_t *scancode_table;

  if (ksym->mod & KMOD_SHIFT) {
    scancode_table = shifted_scancodes;
  } else if (ksym->mod & KMOD_ALT) {
    scancode_table = alt_scancodes;
  } else if (ksym->mod & KMOD_CTRL) {
    scancode_table = ctrl_scancodes;
  } else {
    scancode_table = normal_scancodes;
  }
  uint16_t key = scancode_table[ksym->sym];
  if (key > 0) {
    vga_addkey(key);
  }
  return 1;
}

static int poll_events()
{
  SDL_Event e;
  int should_exit = 0;

  while (SDL_PollEvent(&e) != 0) {
    switch (e.type) {
    case SDL_QUIT:
      should_exit = 1;
      break;
    case SDL_KEYDOWN:
      handle_key(&e);
      break;
    }
  }
  return should_exit;
}

static void delay(unsigned int ms)
{
  SDL_Delay(ms);
}

static unsigned int ticks()
{
  return SDL_GetTicks();
}

struct vga_driver sdl_driver = {
  "SDL", // 2.0
  display_start,
  display_end,
  display_update,
  waitkey,
  get_fb_mem,
  pollkey,
  poll_events,
  delay,
  ticks
};

void video_setup()
{
  register_vga_driver(&sdl_driver);
}
