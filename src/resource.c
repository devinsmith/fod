/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2018-2020,2025 Devin Smith <devin@devinsmith.net>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <err.h>

#include "common/bufio.h"
#include "common/compress.h"
#include "resource.h"
#include "utils/sha1.h"

static bool check_file(const char *filename, const char *hash)
{
  bool is_valid = false;
  unsigned char *bytes = NULL;
  FILE *fp;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open file '%s' for reading. Can't proceed.\n",
        filename);
    goto done;
  }

  fseek(fp, 0, SEEK_END);
  long filesize = ftell(fp);
  if (filesize == -1) {
    fprintf(stderr, "Failed to find file size for file %s. Can't proceed.\n",
        filename);
    goto done;
  }
  rewind(fp);

  if (filesize > 0xFFFF) {
    fprintf(stderr, "File %s is likely too large (%zu bytes). Can't proceed.\n",
        filename, filesize);
    goto done;
  }

  /* Skip hash check */
  if (hash == NULL) {
    is_valid = true;
    goto done;
  }

  bytes = malloc(filesize);

  size_t tb = 0;
  while (!feof(fp)) {
    tb += fread(bytes + tb, 1, BUFSIZ, fp);
  }

  if (tb != (size_t)filesize) {
    fprintf(stderr, "Failed to read all of file '%s'. "
        "Read %zu out of %zu bytes. Can't proceed.\n", filename, tb, filesize);
    goto done;
  }

  // Sha1
  char digest[SHA1_DIGEST_STRING_LENGTH];
  SHA1_CTX sha1_ctx;
  SHA1Init(&sha1_ctx);
  SHA1Update(&sha1_ctx, bytes, filesize);
  SHA1End(&sha1_ctx, digest);

  if (strcmp(digest, hash) != 0) {
    fprintf(stderr, "File %s has hash mismatch. Expected %s, Actual %s\n",
        filename, hash, digest);
    goto done;

  }

  is_valid = true;
done:
  if (fp != NULL) {
    fclose(fp);
  }
  if (bytes != NULL) {
    free(bytes);
  }
  return is_valid;
}

static bool check_files()
{
  return
    check_file("disk1", NULL) &&
    check_file("tpict", "b9dfccb6e084458e321aa866b1ce52e9aba0a040") &&
    check_file("borders", "ace004a244b9f55039e55092ec802869c544008f") &&
    check_file("font", "acc08c29b1df9d4049d8c2b28b69e3897e5779a1");
}

static struct buf_rdr *open_file(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    errx(1, "couldn't open %s", filename);
  }
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  printf("Opened %s %zu bytes\n", filename, size);

  unsigned char *data = malloc(size);
  if (data == NULL) {
    errx(1, "no memory");
  }

  size_t tb = 0;
  while (feof(fp) == 0) {
    tb += fread(data + tb, 1, 1024, fp);
  }

  fclose(fp);
  return buf_rdr_init(data, size);
}

struct resource* resource_load(enum resource_file rt)
{
  const char *fname = NULL;
  bool compressed = false;

  if (rt == RESOURCE_TITLE) {
    fname = "tpict";
    compressed = true;
  }

  if (fname == NULL) {
    return NULL;
  }

  struct resource* res = malloc(sizeof(struct resource));
  struct buf_rdr *rdr = open_file(fname);

  if (compressed) {
    uint16_t u_bytes = buf_get16le(rdr);
    uint16_t dx = buf_get16le(rdr);
    printf("Total bytes: %d\n", u_bytes);
    printf("DX: 0x%04x (should be 0)\n", dx);
    if (dx != 0) {
      errx(0, "DX values other than 0 are unhandled\n");
    }

    unsigned char *dest = malloc(u_bytes);

    decompress(rdr->data + 4, dest, u_bytes);

    // After decompressing, we can get rid of the compressed copy, and
    // store the data directly into the resource.
    res->len = u_bytes;
    // move pointer
    res->bytes = dest;
  } else {
    res->bytes = rdr->data;
    res->len = rdr->len;
  }

  buf_rdr_free(rdr);

  return res;
}

struct resource *resource_load_sz(enum resource_file rfile, long offset,
    size_t sz)
{
  const char *fname = NULL;

  if (rfile == RESOURCE_BORDERS) {
    fname = "borders";
  }

  if (fname == NULL) {
    return NULL;
  }

  printf("Reading %zu bytes of %s at offset %ld\n", sz, fname, offset);

  struct resource* res = malloc(sizeof(struct resource));

  FILE *fp = fopen(fname, "rb");
  if (fp == NULL) {
    errx(1, "couldn't open %s", fname);
  }
  fseek(fp, offset, SEEK_SET);

  res->bytes = malloc(sz);
  if (res->bytes == NULL) {
    errx(1, "no memory");
  }

  size_t tb = 0;
  while (tb < sz) {
    size_t bytes = sz > 1024 ? 1024 : sz;
    tb += fread(res->bytes + tb, 1, bytes, fp);
  }

  fclose(fp);

  res->len = sz;

  return res;
}

bool rm_init()
{
  return check_files();
}

void resource_release(struct resource *res)
{
  if (res != NULL) {
    free(res->bytes);
  }
  free(res);
}

