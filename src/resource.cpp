//
// Fountain of Dreams - Reverse Engineering Project
//
// Copyright (c) 2018-2020,2025 Devin Smith <devin@devinsmith.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdio>
#include <cstring>

#include <err.h>

#include "common/compress.h"
#include "common/stream.h"
#include "resource.h"
#include "utils/sha1.h"

static bool check_file(const char *filename, const char *hash)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == nullptr) {
    fprintf(stderr, "Failed to open %s for reading. Can't proceed.\n", filename);
    return false;
  }

  fseek(fp, 0, SEEK_END);
  long filesize = ftell(fp);
  if (filesize == -1) {
    fprintf(stderr, "Failed to find file size for file %s. Can't proceed.\n", filename);
    fclose(fp);
    return false;
  }
  rewind(fp);

  if (filesize > 0xFFFF) {
    fprintf(stderr, "File %s is likely too large (%zu bytes). Can't proceed.\n", filename, filesize);
    fclose(fp);
    return false;
  }

  auto bytes = new unsigned char[filesize];

  size_t tb = 0;
  while (!feof(fp)) {
    tb += fread(bytes + tb, 1, BUFSIZ, fp);
  }

  if (tb != (size_t)filesize) {
    fprintf(stderr, "Failed to read all of file %s. Read %zu out of %zu bytes. Can't proceed.\n", filename, tb, filesize);
    fclose(fp);
    delete[] bytes;
    return false;
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
    fclose(fp);
    delete[] bytes;
    return false;

  }

  delete[] bytes;
  fclose(fp);

  return true;
}

static bool check_files()
{
  return check_file("tpict", "b9dfccb6e084458e321aa866b1ce52e9aba0a040");
}

static struct stream *open_file(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == nullptr) {
    errx(1, "couldn't open %s", filename);
  }
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  printf("Opened %s %zu bytes\n", filename, size);

  auto data = new unsigned char[size];
  if (data == nullptr) {
    errx(1, "no memory");
  }

  size_t tb = 0;
  while (feof(fp) == 0) {
    tb += fread(data + tb, 1, 1024, fp);
  }

  fclose(fp);
  return stream_init(data, size);
}

int decompress_file(struct stream *s, resource* output)
{
  uint16_t u_bytes = stream_get16_le(s);
  uint16_t dx = stream_get16_le(s);
  printf("Total bytes: %d\n", u_bytes);
  printf("DX: 0x%04x (should be 0)\n", dx);
  if (dx != 0) {
    printf("DX values other than 0 are unhandled\n");
    return -1;
  }

  output->bytes = new unsigned char[u_bytes];
  if (output->bytes == nullptr) {
    errx(1, "no memory");
  }
  auto out_s = stream_init(output->bytes, 0);

  decompress(s, out_s, u_bytes);

  return 0;
}

resource* resource_load(resource_type rt)
{
  const char *fname = nullptr;
  bool compressed = false;

  if (rt == resource_type::title) {
    fname = "tpict";
    compressed = true;
  }

  if (fname == nullptr) {
    return nullptr;
  }

  resource* res = new resource;
  stream *res_stream = open_file(fname);
  if (compressed) {
    decompress_file(res_stream, res);
  }

  return res;
}

bool rm_init()
{
  return check_files();
}

