/*
 * sfx2wav: render PC-speaker effect streams to 48 kHz mono 16-bit WAV files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "platform.h"
#include "sfx.h"

#define RATE           48000
#define PIT_HZ         1193182.0
#define VGA_REFRESH_HZ 70.086       /* one sub_1119E tick per frame */
#define MAX_HZ         20000.0      /* above this the speaker is silent (e.g. divisor 2) */
#define AMPLITUDE      0.5

/* ---- fake hardware: record instead of play ---- */
static float  *pcm;
static size_t  pcm_n, pcm_cap;
static bool    gate;
static double  phase, frac;

static double tri(double x) { x -= floor(x); return x < 0.5 ? x : 1.0 - x; }

static void speaker_set(bool enable)       { gate = enable; }

void hw_pit_ch2_set_divisor(uint16_t divisor)
{
  if (!gate) return;
  frac += RATE / VGA_REFRESH_HZ;            /* ~684.86 samples/frame, no drift */
  size_t n = (size_t)frac;
  frac -= (double)n;

  if (pcm_n + n > pcm_cap) {
      pcm_cap = (pcm_n + n) * 2 + 4096;
      pcm = realloc(pcm, pcm_cap * sizeof *pcm);
      if (!pcm) { perror("realloc"); exit(1); }
  }
  double hz = PIT_HZ / (divisor ? divisor : 65536);
  if (hz > MAX_HZ) {
      memset(pcm + pcm_n, 0, n * sizeof *pcm);
  } else {
      double dp = hz / RATE;
      for (size_t i = 0; i < n; i++) {
          double p1 = phase + dp;
          pcm[pcm_n + i] = (float)(AMPLITUDE * (tri(p1) - tri(phase)) / dp);
          phase = p1 - floor(p1);
      }
  }
  pcm_n += n;
}

/* ---- rendering / output ---- */
static void render(int snd_num)
{
  pcm_n = 0; phase = 0; frac = 0; gate = false;
  play_sound(snd_num);
}

static void put16(FILE *f, unsigned v) { fputc(v & 255, f); fputc((v >> 8) & 255, f); }
static void put32(FILE *f, unsigned v) { put16(f, v & 0xFFFF); put16(f, v >> 16); }

static int write_wav(const char *path)
{
  size_t start = 0;
  while (start < pcm_n && pcm[start] == 0.0f) start++;   /* trim the silent lead-in */
  size_t n = pcm_n - start;

  FILE *f = fopen(path, "wb");
  if (!f) { perror(path); return -1; }
  fwrite("RIFF", 1, 4, f); put32(f, 36 + (unsigned)(n * 2));
  fwrite("WAVEfmt ", 1, 8, f); put32(f, 16);
  put16(f, 1); put16(f, 1);                 /* PCM, mono */
  put32(f, RATE); put32(f, RATE * 2); put16(f, 2); put16(f, 16);
  fwrite("data", 1, 4, f); put32(f, (unsigned)(n * 2));
  for (size_t i = 0; i < n; i++) {
      float v = pcm[start + i];
      if (v > 1.0f) v = 1.0f; else if (v < -1.0f) v = -1.0f;
      put16(f, (unsigned)(int)(v * 32767.0f) & 0xFFFF);
  }
  fclose(f);
  printf("%s: %zu samples, %.0f ms\n", path, n, 1000.0 * (double)n / RATE);
  return 0;
}

struct plat_driver write_wav_driver = {
  "WAV_OUTPUT",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  speaker_set,
  hw_pit_ch2_set_divisor,
  NULL
};

int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "usage: %s OUT.wav [sample#]\n", argv[0]);
    return 2;
  }

  int idx = (argc >= 3) ? (int)strtol(argv[2], NULL, 0) : 0;

  register_platform_driver(&write_wav_driver);

  render(idx);
  return write_wav(argv[1]) ? 1 : 0;
}
