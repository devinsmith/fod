//
// Fountain of Dreams - Reverse Engineering Project
//
// Copyright (c) 2025 Devin Smith <devin@devinsmith.net>
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

#include "vga.h"
#include "utils/sha1.h"

static const int GAME_WIDTH = 320;
static const int GAME_HEIGHT = 200;

bool check_file(const char *filename, const char *hash)
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

  unsigned char *bytes = new unsigned char[filesize];

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

static void do_title()
{
  vga_waitkey();
}

int main(int argc, char *argv[])
{
  if (!check_files()) {
    fprintf(stderr, "Failed file check, exiting!\n");
    return 1;
  }

  // Register VGA driver.
  video_setup();

  if (vga_initialize(GAME_WIDTH, GAME_HEIGHT) != 0) {
    return 1;
  }

  do_title();

  vga_end();

  return 0;
}
