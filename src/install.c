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

#include "common/compress.h"
#include "common/bufio.h"

void hexdump(void *vp, int length);
static void extract_disk1(struct buf_rdr *rdr);
static void do_install(void);
static void extract_raw_file(struct buf_rdr *rdr, char *filename);
static void extract_file(struct buf_rdr *rdr, char *filename);
static struct buf_rdr *open_install_file(char *filename);

int
main(void)
{
	do_install();
	printf("Install done\n");

	return 0;
}

static struct buf_rdr *
open_install_file(char *filename)
{
	FILE *fp;
	int size;
	int tb;
	unsigned char *data;
	struct buf_rdr *rdr;

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
	rdr = buf_rdr_init(data, size);

	return rdr;
}

static void
do_install(void)
{
	struct buf_rdr *rdr;

	/* Open F1.PAK */
	rdr = open_install_file("F1.PAK");
	if (rdr == NULL) {
		errx(1, "no memory for stream");
	}

	mkdir("output", 0777);
	chdir("output");

	extract_disk1(rdr);
	rdr->offset = 0;
	extract_file(rdr, "fod.exe");
	extract_file(rdr, "keh.exe");
	extract_file(rdr, "globals");
	extract_file(rdr, "hdspct");
	extract_file(rdr, "weapons");
	extract_file(rdr, "packets");
	extract_file(rdr, "archtype");
	extract_file(rdr, "services");
	extract_file(rdr, "font");
	extract_file(rdr, "borders");
	extract_file(rdr, "disk2");
	extract_file(rdr, "kmap");

	/* Close data allocated for F1.PAK */
	free(rdr->data);
  buf_rdr_free(rdr);

	chdir("..");

	/* Open F1.RAW */
	rdr = open_install_file("F1.RAW");
	if (rdr == NULL) {
		errx(1, "no memory for stream");
	}

	chdir("output");
	extract_raw_file(rdr, "tiles");
	extract_raw_file(rdr, "gani");
	extract_raw_file(rdr, "tpict");
	extract_raw_file(rdr, "kscr");
	extract_raw_file(rdr, "kani");
	extract_raw_file(rdr, "kpict");
	extract_raw_file(rdr, "fpict");
	extract_raw_file(rdr, "epict");
	extract_raw_file(rdr, "dpict");

	/* Close data allocated for F1.RAW */
	free(rdr->data);
  buf_rdr_free(rdr);

	chdir("..");

	/* Open F2.PAK */
	rdr = open_install_file("F2.PAK");
	if (rdr == NULL) {
		errx(1, "no memory for stream");
	}

	chdir("output");
	extract_file(rdr, "fmap");
	extract_file(rdr, "dmap");

	/* Close data allocated for F2.PAK */
	free(rdr->data);
  buf_rdr_free(rdr);

	chdir("..");

	/* Open F2.RAW */
	rdr = open_install_file("F2.RAW");
	if (rdr == NULL) {
		errx(1, "no memory for stream");
	}

	chdir("output");
	extract_raw_file(rdr, "fscr");
	extract_raw_file(rdr, "fani");
	extract_raw_file(rdr, "dani");
	extract_raw_file(rdr, "dscr");

	/* Close data allocated for F2.RAW */
	free(rdr->data);
  buf_rdr_free(rdr);
}

static void
extract_raw_file(struct buf_rdr *rdr, char *filename)
{
	uint32_t u_bytes;
	FILE *fp;

	/* RAW file format
	 * Uncompressed length (4 bytes): Length of uncompressed data.
	 * Data */

	u_bytes = buf_get32le(rdr);
	printf("Extracting %s (%d bytes)\n", filename, u_bytes);

	fp = fopen(filename, "wb");
	fwrite(rdr->data + rdr->offset, 1, u_bytes, fp);
	fclose(fp);

	rdr->offset += u_bytes;
}

static void
extract_file(struct buf_rdr *rdr, char *filename)
{
	uint32_t c_bytes;
	uint32_t u_bytes;
	FILE *fp;

	/* F1.PAK file format
	 * Header length (4 bytes): Length of compressed data minus the 4 byte header
	 * Uncompressed length (4 bytes): Length of uncompressed data.
	 * Compression codes (variable) */

	c_bytes = buf_get32le(rdr);
	u_bytes = buf_get32le(rdr);
	printf("Extracting %s (inflating %d to %d bytes)\n", filename, c_bytes, u_bytes);

  struct buf_wri *writer = buf_wri_init(u_bytes);

	decompress(rdr, writer, u_bytes);

	fp = fopen(filename, "wb");
	fwrite(writer->base, 1, u_bytes, fp);
	fclose(fp);

  buf_wri_free(writer);
}

static void
extract_disk1(struct buf_rdr *rdr)
{
	uint32_t c_bytes;
	uint32_t u_bytes;
	int bytes_read;
	int next_offset;
	int i;
	FILE *fp;
	unsigned char *junk;

	bytes_read = 0;
	next_offset = 0;

	/* The disk1 data is based off of disk2 */
	for (i = 0; i < 10; i++) {
		rdr->offset = next_offset;

		/* Read in the next offset (stored in the first 4 bytes) */
		c_bytes = buf_get32le(rdr);
		bytes_read += 4; /* 4 bytes read */
		next_offset += c_bytes + 4;

		c_bytes += bytes_read;
		bytes_read = c_bytes;
		bytes_read = bytes_read & 0x000f;
	}

	rdr->offset = next_offset;
	c_bytes = buf_get32le(rdr);
	u_bytes = buf_get32le(rdr);

  struct buf_wri *writer = buf_wri_init(u_bytes);

	decompress(rdr, writer, u_bytes);

	/* No idea why the installer does this */
	junk = malloc(0xec0 - (0x729 * 2));
	memset(junk, 0, 0xec0 - (0x729 * 2));
	writer->base[6] = 3;

	printf("Extracting disk1 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
	fp = fopen("disk1", "wb");
	fwrite(writer->base, 1, 0x729 * 2, fp);
	fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
	fclose(fp);

	printf("Extracting disk3 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
	fp = fopen("disk3", "wb");
	fwrite(writer->base, 1, 0x729 * 2, fp);
	fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
	fclose(fp);

	printf("Extracting disk4 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
	fp = fopen("disk4", "wb");
	fwrite(writer->base, 1, 0x729 * 2, fp);
	fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
	fclose(fp);


	free(junk);
  buf_wri_free(writer);
}

