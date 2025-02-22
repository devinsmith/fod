#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>

#include "common/compress.h"
#include "common/stream.h"

extern void hexdump(void *vp, int length);

unsigned char g_win[0x1010]; // 4112 bytes

static void build_lookup_buf()
{
  for (int i = 0; i < 0x1010; i++) {
    g_win[i] = 0x20;
  }
}


static void decode(uint16_t src)
{
  uint8_t ch = src >> 8;

  uint16_t bx = src;
  bx = (bx << 4) | (bx >> 12); // Rotate left by 4 bits

  // Rotated low byte goes to high.
  uint16_t trans1 = ((bx & 0x00FF) << 8) | (src & 0x00FF);
  uint16_t trans2 = (bx & 0xFF00) | ch;

  printf("SRC: 0x%04X (BX: 0x%04X) T1: 0x%04X, T2: 0x%04X\n",
      src, bx, trans1, trans2);
}

static struct stream *open_file(char *filename)
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

int decompress_file(struct stream *s, const char *name)
{
  struct stream *out_s;

  printf("Decompressing %s\n", name);

  uint16_t u_bytes = stream_get16_le(s);
  uint16_t dx = stream_get16_le(s);
  printf("Total bytes: %d\n", u_bytes);
  printf("DX: 0x%04x (should be 0)\n", dx);
  if (dx != 0) {
    printf("DX values other than 0 are unhandled\n");
    return -1;
  }

  unsigned char *data = malloc(u_bytes);
  if (data == NULL) {
    errx(1, "no memory");
  }
  out_s = stream_init(data, 0);

  decompress(s, out_s, u_bytes);

  FILE *fp = fopen("tpict.raw", "wb");
  fwrite(out_s->data, 1, u_bytes, fp);
  fclose(fp);

  stream_free(out_s);
  free(data);

  return 0;
}

int main()
{
  struct stream *s = open_file("tpict");

  decompress_file(s, "tpict");

  /*
  build_lookup_buf();

  decode(0xB2BA);
  decode(0xAA2A);
  decode(0x22AA);
  */

  return 0;
}
