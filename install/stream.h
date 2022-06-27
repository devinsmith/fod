/* $Id: stream.h 2002 2008-12-24 18:54:01Z devin $ */

/*
 * Copyright (c) 2008 Devin Smith <devin@devinsmith.net>
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

#ifndef __STREAM_H__
#define __STREAM_H__

#include <sys/types.h>
#include <stdint.h>

struct stream
{
	unsigned char *data;
	size_t len;
	int offset;

	uint32_t holding;
	int holding_count;
};

/* stream functions */
struct stream * stream_init(unsigned char *data, size_t len);
void stream_advance(struct stream *s, int adv);
int stream_read_string_until_crlf(struct stream *s, char *str, size_t siz);
uint8_t stream_get8(struct stream *s);
uint16_t stream_get16_le(struct stream *s);
uint16_t stream_get16(struct stream *s);
uint32_t stream_get32_le(struct stream *s);
uint32_t stream_get32(struct stream *s);
uint16_t stream_peek16_le(struct stream *);
int stream_getstring(struct stream *s, int len, char *str);
int stream_getdata(struct stream *s, int len, unsigned char *data);
uint32_t stream_getbits(struct stream *s, int num_bits);
int32_t stream_getsbits(struct stream *s, int num_bits);
void stream_free(struct stream *s);

void stream_add8(struct stream *s, uint8_t val);

#endif /* __STREAM_H__ */

