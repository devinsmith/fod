/* $Id: stream.c 2002 2008-12-24 18:54:01Z devin $ */

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

#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"

struct stream *
stream_init(unsigned char *data, size_t len)
{
	struct stream *s = NULL;

	s = malloc((size_t) sizeof(struct stream));
	if (s == NULL) return NULL;
	memset(s, 0, sizeof(struct stream));

	s->data = data;
	s->len = len;
	s->offset = 0;

	return s;
}

void
stream_advance(struct stream *s, int adv)
{
	s->offset += adv;
}

int
stream_read_string_until_crlf(struct stream *s, char *str, size_t siz)
{
	int res = 0;
	char *d = str;

	if (siz == 0) return -1;

	while (s->offset < s->len && isprint(*(s->data + s->offset)) &&
	    --siz != 0) {
		*d++ = *(s->data + s->offset);
		res++;
		s->offset++;
	}

	if (siz != 0)
		*d = '\0';

	/* If we got a string at all */
	if (res > 0)
		s->offset += 2; /* Skip the CR/LF */

	return res;
}

uint8_t
stream_get8(struct stream *s)
{
	return s->data[s->offset++];
}

uint16_t
stream_peek16_le(struct stream *s)
{
	uint16_t ret;

	ret = s->data[s->offset];
	ret += s->data[s->offset + 1] << 8;
	return ret;
}

uint16_t
stream_get16_le(struct stream *s)
{
	uint16_t ret;

	ret = s->data[s->offset++];
	ret += s->data[s->offset++] << 8;

	return ret;
}

uint16_t
stream_get16(struct stream *s)
{
	uint16_t ret;

	ret = s->data[s->offset++] << 8;
	ret += s->data[s->offset++];

	return ret;
}

uint32_t
stream_get32_le(struct stream *s)
{
	uint32_t ret;

	ret = s->data[s->offset++];
	ret += s->data[s->offset++] << 8;
	ret += s->data[s->offset++] << 16;
	ret += s->data[s->offset++] << 24;

	return ret;
}

uint32_t
stream_get32(struct stream *s)
{
	uint32_t ret;

	ret = htonl(*((uint32_t *) &s->data[s->offset]));
	s->offset += 4;

	return ret;
}

int
stream_getstring(struct stream *s, int len, char *str)
{
	if (str == NULL)
		return -1;

	strncpy(str, (char *) s->data + s->offset, len - 1);
	str[len - 1] = '\0';

	s->offset += len;

	return len;
}

int
stream_getdata(struct stream *s, int len, unsigned char *data)
{
	if (data == NULL)
		return -1;

	memcpy(data, s->data + s->offset, len);

	s->offset += len;

	return len;
}

uint32_t
stream_getbits(struct stream *s, int num_bits)
{
	int temp;
	int shift;
	uint32_t ret;

	while (s->holding_count < num_bits) {
		if (s->offset > s->len) {
			/* This would indicate a corrupt SWF file
			 * and that's an error worth bailing on */
			return 0;
		}

		temp = stream_get8(s);
		/* copy the byte we just read over to holding, and bump
		 * holding_count up by 8 because 8 bits have been added to
		 * holding */
		s->holding = s->holding | (temp << (24 - s->holding_count));
		s->holding_count += 8;
	}

	shift = 32 - num_bits;
	ret = (s->holding >> shift);
	s->holding <<= num_bits;
	s->holding_count -= num_bits;
	return ret;
}

int32_t
stream_getsbits(struct stream *s, int num_bits)
{
	int32_t ret;

	/* Get the number as an unsigned value. */
	ret = stream_getbits(s, num_bits);

	/* Is the number negative? */
	if (ret & (1L << (num_bits - 1))) {
		/* Yes. Extend the sign. */
		ret |= -1L << num_bits;
	}

	return ret;
}

void
stream_add8(struct stream *s, uint8_t val)
{
	s->data[s->offset] = val;
	s->offset++;
	s->len++;
}

void
stream_free(struct stream *s)
{
	if (s == NULL) return;
	free(s);
}

