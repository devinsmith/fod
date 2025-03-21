/*
 * Copyright (c) 2008-2025 Devin Smith <devin@devinsmith.net>
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

#include <string.h>

#include "common/compress.h"

/**
 * Decompresses data using an LZSS-like algorithm
 *
 * @param src Pointer to compressed source data
 * @param dest Pointer to destination buffer
 * @param uncompressed_size Expected size of uncompressed data in bytes
 */
 // seg000:0x19DA
void decompress(const unsigned char *src, unsigned char *dest, uint32_t uncompressed_size)
{
  // Set up our history dictionary and window pointers
  unsigned char history[4096];
  memset(history, 0x20, sizeof(history));
  uint16_t dict_read_pos = 0;
  uint16_t dict_write_pos = 0xfee;

  uint32_t bytes_remaining = uncompressed_size;

  // Bit extraction state
  uint8_t control_byte = 0;
  int bits_remaining = 0;

  while (bytes_remaining > 0) {
    // Get a new control byte when needed
    if (bits_remaining == 0) {
      control_byte = *src++;
      bits_remaining = 8;
    }

    // Check the next control bit
    if (control_byte & 0x01) {
      // Literal byte: copy directly from input to output
      uint8_t literal = *src++;
      *dest++ = literal;
      bytes_remaining--;

      // Add the literal to history
      history[dict_write_pos] = literal;
      dict_write_pos = (dict_write_pos + 1) & 0xfff;
    } else {
      // Match reference: copy from history buffer
      uint8_t offset_low = *src++;
      uint8_t offset_high = *src++;

      // Compute match offset and length
      uint16_t match_offset = ((offset_high >> 4) << 8) | offset_low;
      uint8_t match_length = (offset_high & 0x0f) + 3;
      dict_read_pos = match_offset;

      // Copy matched bytes from history to output
      for (int i = 0; i < match_length; i++) {
        uint8_t matched_byte = history[dict_read_pos];
        *dest++ = matched_byte;
        dict_read_pos = (dict_read_pos + 1) & 0xfff;
        bytes_remaining--;

        if (bytes_remaining == 0) {
          break;
        }
        history[dict_write_pos] = matched_byte;
        dict_write_pos = (dict_write_pos + 1) & 0xfff;
      }
    }

    // Advance to next control bit
    control_byte >>= 1;
    bits_remaining--;
  }
}
