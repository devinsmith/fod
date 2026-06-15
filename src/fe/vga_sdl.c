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

static uint8_t translate_key(const SDL_Event *e)
{
  const SDL_KeyboardEvent *ke = &e->key;
  const SDL_Keysym *ksym = &ke->keysym;
  uint8_t keyCode = 0;

  // Special cases specific to FOD.
  if (ksym->sym == SDLK_RIGHT) {
    return 0xFA;
  }

  if (ksym->sym == SDLK_LEFT) {
    return 0xFB;
  }

  if (ksym->sym == SDLK_DOWN) {
    return 0xFC;
  }

  if (ksym->sym == SDLK_UP) {
    return 0xFD;
  }

  if (ksym->sym == SDLK_RETURN) {
    return 0xFE;
  }

  if (ksym->sym == SDLK_ESCAPE) {
    return 0xFF;
  }

  if (ksym->sym == SDLK_F1) {
    return 0x3B;
  }

  if (ksym->sym == SDLK_F2) {
    return 0x3C;
  }

  if (ksym->sym == SDLK_F3) {
    return 0x3D;
  }

  // Here we assume that ksym->sym will contain the
  // standard ascii key codes
  // 'A' = 0x41, 'a' = 0x61, and so on.
  if ((ksym->sym & SDLK_SCANCODE_MASK) == 0) {
    keyCode = (uint8_t)ksym->sym;
    if (ksym->mod & KMOD_SHIFT) {
      keyCode -= 0x20;
    }
  }

  return keyCode;
}

static uint8_t *
get_fb_mem()
{
  return surface->pixels;
}

static bool pollkey(unsigned int ms)
{
  return vga_getkey() != 0;
}

static int handle_key(SDL_Event *e)
{
  uint8_t key = translate_key(e);
  if (key > 0) {
    vga_addkey(key);
  }
#if 0
  const SDL_KeyboardEvent *ke = &e->key;
  const SDL_Keysym *ksym = &ke->keysym;
  const uint16_t *scancode_table;
  const int table_size = sizeof(normal_scancodes) / sizeof(normal_scancodes[0]);
  SDL_Scancode scancode = ke->keysym.scancode;

  if (ksym->mod & KMOD_SHIFT) {
    scancode_table = shifted_scancodes;
  } else if (ksym->mod & KMOD_ALT) {
    scancode_table = alt_scancodes;
  } else if (ksym->mod & KMOD_CTRL) {
    scancode_table = ctrl_scancodes;
  } else {
    scancode_table = normal_scancodes;
  }

  if (scancode < 0 || scancode >= table_size) {
    printf("Scancode %d out of table range (max %d)\n", scancode, table_size - 1);
    return 1;
  }

  uint16_t key = scancode_table[scancode];
  printf("Handling key: 0x%04X\n", key);
  if (key > 0) {
    vga_addkey(key);
  }
#endif
  return 1;
}

// Quickly poll for keystrokes and the SDL_QUIT message,
// pushing key presses into a key buffer.
// Return value indicates whether SQL_QUIT was processed.
static bool poll_events()
{
  SDL_Event e;
  bool should_exit = false;

  while (SDL_PollEvent(&e) != 0) {
    switch (e.type) {
    case SDL_QUIT:
      should_exit = true;
      break;
    case SDL_KEYDOWN:
      handle_key(&e);
      break;
    }
  }
  return should_exit;
}

uint8_t waitkey()
{
  uint8_t key;

  while (1) {
    key = vga_getkey();
    if (key != 0) {
      return key;
    }
    if (poll_events()) {
      return 0;
    }
    SDL_Delay(10);
  }
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
