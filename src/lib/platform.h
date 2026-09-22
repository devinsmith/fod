/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2018-2020,2025-2026 Devin Smith <devin@devinsmith.net>
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

#ifndef PLAT_INTERFACE_H
#define PLAT_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct plat_driver {
  const char *driver_name;
  int (*initialize)(int game_width, int game_height);
  void (*end)();
  void (*update)();
  uint8_t (*waitkey)();
  uint8_t* (*memory)();
  bool (*pollkey)(unsigned int ms);
  bool (*poll)();
  void (*delay)(unsigned int ms);
  unsigned int (*ticks)();

  // Turns the speaker on or off.
  void (*speaker_set)(bool enable);

  // Divisor = PC-speaker PIT divisor from original
  // sound data.
  void (*tone)(uint16_t divisor);
};

void register_platform_driver(struct plat_driver *driver);

int vga_initialize(int game_width, int game_height);
uint8_t* vga_memory();
void vga_update();
bool vga_pollkey(unsigned int ms);
uint8_t vga_waitkey();
void vga_end();
void vga_addkey(int key);
int vga_getkey();
bool vga_peek_key();
bool vga_poll_events();
void sys_delay(unsigned int ms);
unsigned int sys_ticks();

void hw_speaker_set(bool enable);
void hw_speaker_tone(uint16_t divisor);

// Intended for use by various platforms drivers.
void platform_setup();

#ifdef __cplusplus
}
#endif

#endif // PLAT_INTERFACE_H
