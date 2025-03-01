/* $Id: fod_install.c 2021 2008-12-30 06:44:36Z devin $ */

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
#include <sys/stat.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/stream.h"

void hexdump(void *vp, int length);
static void extract_disk1(struct stream *s);
static void do_install(void);
static void extract_raw_file(struct stream *s, char *filename);
static void extract_file(struct stream *s, char *filename);
static struct stream *open_install_file(char *filename);

int
main(void)
{
	do_install();
	printf("Install done\n");

	return 0;
}

static struct stream *
open_install_file(char *filename)
{
	FILE *fp;
	int size;
	int tb;
	unsigned char *data;
	struct stream *s;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		errx(1, "couldn't open %s", filename);
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("Opened %s %d bytes\n", filename, size);

	data = malloc(size);
	if (data == NULL) {
		errx(1, "no memory");
	}

	tb = 0;
	while (feof(fp) == 0) {
		tb += fread(data + tb, 1, 1024, fp);
	}

	fclose(fp);
	s = stream_init(data, size);

	return s;
}

static void
do_install(void)
{
	struct stream *s;

	/* Open F1.PAK */
	s = open_install_file("F1.PAK");
	if (s == NULL) {
		errx(1, "no memory for stream");
	}

	mkdir("output", 0777);
	chdir("output");

	extract_disk1(s);
	s->offset = 0;
	extract_file(s, "fod.exe");
	extract_file(s, "keh.exe");
	extract_file(s, "globals");
	extract_file(s, "hdspct");
	extract_file(s, "weapons");
	extract_file(s, "packets");
	extract_file(s, "archtype");
	extract_file(s, "services");
	extract_file(s, "font");
	extract_file(s, "borders");
	extract_file(s, "disk2");
	extract_file(s, "kmap");

	/* Close data allocated for F1.PAK */
	free(s->data);
	stream_free(s);

	chdir("..");

	/* Open F1.RAW */
	s = open_install_file("F1.RAW");
	if (s == NULL) {
		errx(1, "no memory for stream");
	}

	chdir("output");
	extract_raw_file(s, "tiles");
	extract_raw_file(s, "gani");
	extract_raw_file(s, "tpict");
	extract_raw_file(s, "kscr");
	extract_raw_file(s, "kani");
	extract_raw_file(s, "kpict");
	extract_raw_file(s, "fpict");
	extract_raw_file(s, "epict");
	extract_raw_file(s, "dpict");

	/* Close data allocated for F1.RAW */
	free(s->data);
	stream_free(s);

	chdir("..");

	/* Open F2.PAK */
	s = open_install_file("F2.PAK");
	if (s == NULL) {
		errx(1, "no memory for stream");
	}

	chdir("output");
	extract_file(s, "fmap");
	extract_file(s, "dmap");

	/* Close data allocated for F2.PAK */
	free(s->data);
	stream_free(s);

	chdir("..");

	/* Open F2.RAW */
	s = open_install_file("F2.RAW");
	if (s == NULL) {
		errx(1, "no memory for stream");
	}

	chdir("output");
	extract_raw_file(s, "fscr");
	extract_raw_file(s, "fani");
	extract_raw_file(s, "dani");
	extract_raw_file(s, "dscr");

	/* Close data allocated for F2.RAW */
	free(s->data);
	stream_free(s);


}

static void
extract_raw_file(struct stream *s, char *filename)
{
	uint32_t u_bytes;
	FILE *fp;

	/* RAW file format
	 * Uncompressed length (4 bytes): Length of uncompressed data.
	 * Data */

	u_bytes = stream_get32_le(s);
	printf("Extracting %s (%d bytes)\n", filename, u_bytes);

	fp = fopen(filename, "wb");
	fwrite(s->data + s->offset, 1, u_bytes, fp);
	fclose(fp);

	s->offset += u_bytes;
}

static void
extract_file(struct stream *s, char *filename)
{
	uint32_t c_bytes;
	uint32_t u_bytes;
	unsigned char *data;
	struct stream *out_s;
	FILE *fp;

	/* F1.PAK file format
	 * Header length (4 bytes): Length of compressed data minus the 4 byte header
	 * Uncompressed length (4 bytes): Length of uncompressed data.
	 * Compression codes (variable) */

	c_bytes = stream_get32_le(s);
	u_bytes = stream_get32_le(s);
	printf("Extracting %s (inflating %d to %d bytes)\n", filename, c_bytes, u_bytes);

	data = malloc(u_bytes);
	if (data == NULL) {
		errx(1, "no memory");
	}
	out_s = stream_init(data, 0);

	decompress(s, out_s, u_bytes);

	fp = fopen(filename, "wb");
	fwrite(out_s->data, 1, u_bytes, fp);
	fclose(fp);

	stream_free(out_s);
	free(data);

}

static void
extract_disk1(struct stream *s)
{
	uint32_t c_bytes;
	uint32_t u_bytes;
	int bytes_read;
	int next_offset;
	int i;
	unsigned char *data;
	struct stream *out_s;
	FILE *fp;
	unsigned char *junk;

	bytes_read = 0;
	next_offset = 0;

	/* The disk1 data is based off of disk2 */
	for (i = 0; i < 10; i++) {
		s->offset = next_offset;

		/* Read in the next offset (stored in the first 4 bytes) */
		c_bytes = stream_get32_le(s);
		bytes_read += 4; /* 4 bytes read */
		next_offset += c_bytes + 4;

		c_bytes += bytes_read;
		bytes_read = c_bytes;
		bytes_read = bytes_read & 0x000f;
	}

	s->offset = next_offset;
	c_bytes = stream_get32_le(s);
	u_bytes = stream_get32_le(s);

	data = malloc(u_bytes);
	if (data == NULL) {
		errx(1, "no memory");
	}
	out_s = stream_init(data, 0);

	decompress(s, out_s, u_bytes);

	/* No idea why the installer does this */
	junk = malloc(0xec0 - (0x729 * 2));
	memset(junk, 0, 0xec0 - (0x729 * 2));
	out_s->data[6] = 3;

	printf("Extracting disk1 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
	fp = fopen("disk1", "wb");
	fwrite(out_s->data, 1, 0x729 * 2, fp);
	fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
	fclose(fp);

	printf("Extracting disk3 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
	fp = fopen("disk3", "wb");
	fwrite(out_s->data, 1, 0x729 * 2, fp);
	fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
	fclose(fp);

	printf("Extracting disk4 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
	fp = fopen("disk4", "wb");
	fwrite(out_s->data, 1, 0x729 * 2, fp);
	fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
	fclose(fp);


	free(junk);
	stream_free(out_s);
	free(data);
}

