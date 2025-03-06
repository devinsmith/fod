//
// Fountain of Dreams - Reverse Engineering Project
//
// Copyright (c) 2018-2020,2025 Devin Smith <devin@devinsmith.net>
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

#ifndef VGA_INTERFACE_H
#define VGA_INTERFACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vga_driver {
  const char *driver_name;
  int (*initialize)(int game_width, int game_height);
  void (*end)();
  void (*update)();
  void (*waitkey)();
  uint8_t* (*memory)();
  uint16_t (*getkey)();
  int (*poll)();
  void (*delay)(unsigned int ms);
  unsigned short (*ticks)();
};

void register_vga_driver(struct vga_driver *driver);

int vga_initialize(int game_width, int game_height);
uint8_t* vga_memory();
void vga_update();
uint16_t vga_getkey();
void vga_waitkey();
void vga_end();
void vga_addkey(int key);
int vga_getkey2();
int vga_poll_events();
void sys_delay(unsigned int ms);
unsigned short sys_ticks();

// Intended for use by various vga drivers.
void video_setup();

#ifdef __cplusplus
}
#endif

#endif // VGA_INTERFACE_H
