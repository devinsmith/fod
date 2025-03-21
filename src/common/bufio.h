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

/* Provides "readers" and "writers" over a buffer of bytes. */

#ifndef BUFIO_H_INCLUDED
#define BUFIO_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct buf_wri {
  unsigned char *base;
  size_t len;
};

struct buf_rdr {
  unsigned char *data;
  size_t len;
  int offset;
};

/* Buffer writing functions */
struct buf_wri *buf_wri_init(size_t len);
void buf_wri_free(struct buf_wri *w);
void buf_add8(struct buf_wri *w, uint8_t val);

/* Buffer reader functions */

/**
 * @brief Creates a new buffered reader
 *
 * The reader does not own the data but operates on it. The caller should
 * handle the lifetime of the data parameter.
 *
 * @param data  The data to create the reader over
 * @param len   The size of the input data.
 */
struct buf_rdr *buf_rdr_init(unsigned char *data, size_t len);
void buf_rdr_free(struct buf_rdr *r);

/* read data */
void buf_reset(struct buf_rdr *r);

/**
 * @brief Reads 1 byte (8 bits) from the buffer.
 *
 * The index is also advanced by 1 byte.
 *
 * @param r   The reader to read a byte from.
 * @return    The byte that was read.
 */
uint8_t buf_get8(struct buf_rdr *r);
uint16_t buf_get16le(struct buf_rdr *r);
uint16_t buf_get16be(struct buf_rdr *r);
uint32_t buf_get32le(struct buf_rdr *r);
uint32_t buf_get32be(struct buf_rdr *r);

#ifdef __cplusplus
}
#endif

#endif /* BUFIO_H_INCLUDED */

