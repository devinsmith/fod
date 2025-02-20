#include <stdint.h>
#include <stdio.h>

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

int main()
{
  build_lookup_buf();

  decode(0xB2BA);
  decode(0xAA2A);
  decode(0x22AA);

  return 0;
}
