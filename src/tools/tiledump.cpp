//
// Copyright (c) 2025 Devin Smith <devin@devinsmith.net>
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

#include "resource.h"
#include "hexdump.h"

// This program explores TILES

static const size_t fb_width = 640; // 40 * 16
static const size_t fb_height = 400; // 25 * 16
static const int BYTES_PER_PIXEL = 3; // RGB
static const size_t fb_size = fb_width * fb_height * BYTES_PER_PIXEL; // RGB
static unsigned char *fb_mem;

struct palette {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

static const struct palette dos_palette[] = {
  { 0x00, 0x00, 0x00 },  // BLACK
  { 0x00, 0x00, 0x9D },
  { 0x0C, 0x95, 0x0C },
  { 0x00, 0x6D, 0x6D },
  { 0x81, 0x00, 0x00 },
  { 0x6D, 0x00, 0x6D },
  { 0x81, 0x6D, 0x14 },
  { 0x99, 0x99, 0x99 },
  { 0x40, 0x40, 0x40 },
  { 0x00, 0x00, 0xE2 },
  { 0x00, 0xFF, 0x00 },
  { 0x00, 0xFF, 0xFF },
  { 0xEA, 0x81, 0x40 },
  { 0xFF, 0x00, 0xFF },
  { 0xFF, 0xFF, 0x00 },
  { 0xFF, 0xFF, 0xFF }
};

static void write_data(const resource *r, int sprite_offset, int x, int y)
{
  int line_num = y * 16;
  int rows = 16;
  int cols = 4;

  int offset = sprite_offset * 128;

  const unsigned short *p = (unsigned short *)(r->bytes + offset);

  for (int r = 0; r < rows; r++) {
    size_t fb_off = (line_num * (fb_width * BYTES_PER_PIXEL)) +
      ((x * 16) * BYTES_PER_PIXEL); // indentation
    for (int c = 0; c < cols; c++) {
      uint16_t src_pixel = *p++;
      offset += 2;

      // Extract components
      uint8_t low_byte = src_pixel & 0xFF;
      uint8_t high_byte = src_pixel >> 8;

      // Rotate left by 4, right by 12
      uint16_t rotated_pixel = (src_pixel << 4) | (src_pixel >> 12);
      uint8_t rotated_low = rotated_pixel & 0xFF;

      // Rotated low byte goes to high.
      uint16_t trans1 = (rotated_low << 8) | low_byte;
      uint16_t trans2 = (rotated_pixel & 0xFF00) | high_byte;


      uint8_t pixels[4];

      pixels[0] = (trans1 & 0x00FF) >> 4;
      pixels[1] = trans1 >> 12;
      pixels[2] = (trans2 & 0x00FF) >> 4;
      pixels[3] = trans2 >> 12;

      // printf("0x%04X 0x%04X -> 0x%02X 0x%02X 0x%02X 0x%02X\n", trans1, trans2,
      //    pixels[0], pixels[1], pixels[2], pixels[3]);

      for (int k = 0; k < 4; k++) {
        fb_mem[fb_off++] = dos_palette[pixels[k]].r;
        fb_mem[fb_off++] = dos_palette[pixels[k]].g;
        fb_mem[fb_off++] = dos_palette[pixels[k]].b;
      }
    }
    line_num++;
  }

//  printf("Offset: %d\n", offset);
//  printf("Total size: %zu\n", r->len);
//  hexdump(r->bytes + offset, r->len - offset);
}

static void init_buffers()
{
  fb_mem = new unsigned char[fb_size];
  memset(fb_mem, 0, fb_size);
}

int main(int argc, char *argv[])
{
  resource *r = resource_load(RESOURCE_TILES, 0, 0);
  hexdump(r->bytes + 0xe780, 16);
  init_buffers();

  // Dump to PPM
  FILE *imageFile = fopen("tiles.ppm","wb");

  if (imageFile == nullptr){
    perror("ERROR: Cannot open output file");
    exit(EXIT_FAILURE);
  }

  fprintf(imageFile,"P6\n");               // P6 filetype
  fprintf(imageFile,"%zu %zu\n", fb_width, fb_height);   // dimensions
  fprintf(imageFile,"255\n");              // Max pixel

  // There seem to be some duplicate tiles...
  for (int j = 0; j < 25; j++) {
    for (int i = 0; i < 40; i++) {
      write_data(r, i + (j * 25), i, j);
    }
  }

  fwrite(fb_mem, 1, fb_size, imageFile);
  fclose(imageFile);
  resource_release(r);

  return 0;
}
