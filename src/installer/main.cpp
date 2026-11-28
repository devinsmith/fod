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

#include "compress.h"
#include "bufio.h"
#include "hexdump.h"

static void extract_disk1(buf_rdr& rdr);
static void do_install(void);
static void extract_raw_file(buf_rdr& rdr, const char *filename);
static void extract_file(buf_rdr& rdr, const char *filename);

int main(void)
{
  do_install();
  printf("Install done\n");

  return 0;
}

static void do_install(void)
{
  mkdir("output", 0777);

  {
    /* Open F1.PAK */
    auto rdr = buf_rdr::from_file("F1.PAK");
    extract_disk1(rdr);
    rdr.reset();

    chdir("output");
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
    chdir("..");
  }

  {
    /* Open F1.RAW */
    auto rdr = buf_rdr::from_file("F1.RAW");

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

    chdir("..");
  }

  {
    /* Open F2.PAK */
    auto rdr = buf_rdr::from_file("F2.PAK");

    chdir("output");
    extract_file(rdr, "fmap");
    extract_file(rdr, "dmap");

    chdir("..");
  }

  {
    /* Open F2.RAW */
    auto rdr = buf_rdr::from_file("F2.RAW");

    chdir("output");
    extract_raw_file(rdr, "fscr");
    extract_raw_file(rdr, "fani");
    extract_raw_file(rdr, "dani");
    extract_raw_file(rdr, "dscr");
    chdir("..");
  }
}

static void extract_raw_file(buf_rdr& rdr, const char *filename)
{
  uint32_t u_bytes;
  FILE *fp;

  /* RAW file format
   * Uncompressed length (4 bytes): Length of uncompressed data.
   * Data */

  u_bytes = rdr.get32le();
  printf("Extracting %s (%d bytes)\n", filename, u_bytes);

  fp = fopen(filename, "wb");
  fwrite(rdr.cursor(), 1, u_bytes, fp);
  fclose(fp);

  rdr.skip(u_bytes);
}

static void extract_file(buf_rdr& rdr, const char *filename)
{
  uint32_t c_bytes;
  uint32_t u_bytes;
  FILE *fp;

  /* F1.PAK file format
   * Header length (4 bytes): Length of compressed data minus the 4 byte header
   * Uncompressed length (4 bytes): Length of uncompressed data.
   * Compression codes (variable) */

  c_bytes = rdr.get32le();
  u_bytes = rdr.get32le();
  printf("Extracting %s (inflating %d to %d bytes)\n", filename, c_bytes, u_bytes);

  unsigned char *dest = (unsigned char *)malloc(u_bytes);

  decompress(rdr.cursor(), dest, u_bytes);

  fp = fopen(filename, "wb");
  fwrite(dest, 1, u_bytes, fp);
  fclose(fp);

  free(dest);
}

static void extract_disk1(buf_rdr &rdr)
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
    rdr.seek(next_offset);

    /* Read in the next offset (stored in the first 4 bytes) */
    c_bytes = rdr.get32le();
    bytes_read += 4; /* 4 bytes read */
    next_offset += c_bytes + 4;

    c_bytes += bytes_read;
    bytes_read = c_bytes;
    bytes_read = bytes_read & 0x000f;
  }

  rdr.seek(next_offset);
  c_bytes = rdr.get32le();
  u_bytes = rdr.get32le();

  unsigned char *dest = (unsigned char *)malloc(u_bytes);

  decompress(rdr.cursor(), dest, u_bytes);

  /* No idea why the installer does this */
  junk = (unsigned char *)malloc(0xec0 - (0x729 * 2));
  memset(junk, 0, 0xec0 - (0x729 * 2));
  dest[6] = 3;

  printf("Extracting disk1 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
  fp = fopen("disk1", "wb");
  fwrite(dest, 1, 0x729 * 2, fp);
  fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
  fclose(fp);

  printf("Extracting disk3 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
  fp = fopen("disk3", "wb");
  fwrite(dest, 1, 0x729 * 2, fp);
  fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
  fclose(fp);

  printf("Extracting disk4 (inflating %d to %d bytes)\n", c_bytes, u_bytes);
  fp = fopen("disk4", "wb");
  fwrite(dest, 1, 0x729 * 2, fp);
  fwrite(junk, 1, 0xec0 - (0x729 * 2), fp);
  fclose(fp);


  free(junk);
  free(dest);
}

