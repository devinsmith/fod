//
// Copyright (c) 2026 Devin Smith <devin@devinsmith.net>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "bufio.h"
#include "hexdump.h"
#include "item.h"

static void read_wt(buf_rdr& rdr)
{
  struct weapon_type wt;

  rdr.read(wt.fire_msg, sizeof(wt.fire_msg));

  wt.unknown_14 = rdr.get8();
  wt.unknown_15 = rdr.get8();
  wt.unknown_16 = rdr.get8();
  wt.unknown_17 = rdr.get8();
  wt.flags = rdr.get8();                 /* 0x18: 03 in all three; bit 0 = ranged */
  wt.unknown_19 = rdr.get8();            /* 0x19: 0x32 (50) in all three */
  wt.unknown_1A = rdr.get8();            /* 0x1A: 0x63 (99) in all three */
  rdr.read(wt.unknown_1B, sizeof(wt.unknown_1B));
  wt.unknown_1E = rdr.get8();            /* 0x1E: 05 in all three */
  rdr.read(wt.unknown_1F, sizeof(wt.unknown_1F));
  wt.unknown_22 = rdr.get8();            /* 0x22: 03 in all three */
  rdr.read(wt.unknown_23, sizeof(wt.unknown_23));
  wt.unknown_2E = rdr.get8();            /* 0x2E: 05 in all three */
  rdr.read(wt.unknown_2F, sizeof(wt.unknown_2F));
  wt.ammo_count = rdr.get16le();            /* 0x38 */
  wt.ammo_list_offset = rdr.get16le();      /* 0x3A: from start of file, header included */

  printf("  { .fire_msg = \"%s\", .unknown_14 = %d, "
      ".unknown_15 = %d, .unknown_16 = %d,\n    .unknown_17 = %d, .flags = %d, "
      ".unknown_19 = %d, .unknown_1A = %d, .unknown_1B = { %d, %d, %d },\n"
      "    .unknown_1E = %d, .unknown_1F = { %d, %d, %d }, .unknown_22 = %d,\n",
      wt.fire_msg, wt.unknown_14, wt.unknown_15, wt.unknown_16, wt.unknown_17,
      wt.flags, wt.unknown_19, wt.unknown_1A,
      wt.unknown_1B[0], wt.unknown_1B[1], wt.unknown_1B[2], wt.unknown_1E,
      wt.unknown_1F[0], wt.unknown_1F[1], wt.unknown_1F[2], wt.unknown_22);

  printf("    .unknown_23 = { ");
  for (size_t i = 0; i < sizeof(wt.unknown_23); i++) {
    if (i > 0) {
      printf(", %d", wt.unknown_23[i]);
    } else {
      printf("%d", wt.unknown_23[i]);
    }
  }

  printf(" },\n");

  printf("    .unknown_2E = %d,\n", wt.unknown_2E);
  printf("    .unknown_2F = { ");
  for (size_t i = 0; i < sizeof(wt.unknown_2F); i++) {
    if (i > 0) {
      printf(", %d", wt.unknown_2F[i]);
    } else {
      printf("%d", wt.unknown_2F[i]);
    }
  }
  printf(" },\n");
  printf("    .ammo_count = %d, .ammo_list_offset = 0x%04X\n", wt.ammo_count, wt.ammo_list_offset);
  printf("  },\n");
}

// This program explores the WEAPONS file
static void read_weapon_data()
{
  auto rdr = buf_rdr::from_file("weapons");
  uint16_t num_weapons = rdr.get16le();
  printf("Number of weapon types (?): %u\n", num_weapons);

  for (uint16_t i = 0; i < num_weapons; i++) {
    read_wt(rdr);
  }
  hexdump(rdr.cursor(), 9);
}

int main(int argc, char *argv[])
{
  read_weapon_data();
}
