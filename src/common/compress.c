
#include <string.h>

#include "common/compress.h"

unsigned char g_hist[4096];

void decompress(struct buf_rdr *rdr, struct buf_wri *writer, uint32_t u_bytes)
{
	int done;
	uint8_t byte;
	int holding_bits;
	int g_hist_woff;
	int g_hist_roff;
	uint8_t t_byte;
	uint8_t cl_byte;
	uint8_t ch_byte;
	uint8_t al_byte;
	int j;

	memset(g_hist, 0x20, sizeof(g_hist));

	g_hist_roff = 0;
	g_hist_woff = 0xfee;
	byte = 0;

	done = 0;
	holding_bits = 1;
	while (1) {
		holding_bits--;
		if (holding_bits == 0) {
      byte = buf_get8(rdr);
			holding_bits = 8;
		}
		if (byte & 0x01) {
			byte = byte >> 1;
			t_byte = buf_get8(rdr);
      buf_add8(writer, t_byte);
			u_bytes--;
			if (u_bytes == 0) break;
			g_hist[g_hist_woff] = t_byte;
			g_hist_woff++;
			g_hist_woff = g_hist_woff & 0xfff;
		} else {
			byte = byte >> 1;
			t_byte = buf_get8(rdr);
			cl_byte = t_byte;
			t_byte = buf_get8(rdr);
			ch_byte = t_byte;
			t_byte = (t_byte & 0x0f) + 3;
			g_hist_roff =(((ch_byte >> 4) << 8) + cl_byte);

			for (j = 0; j < t_byte; j++) {
				al_byte = g_hist[g_hist_roff];
        buf_add8(writer, al_byte);
				g_hist_roff++;
				g_hist_roff = g_hist_roff & 0xfff;
				u_bytes--;
				if (u_bytes == 0) {
					done = 1;
					break;
				}
				g_hist[g_hist_woff] = al_byte;
				g_hist_woff++;
				g_hist_woff = g_hist_woff & 0xfff;
			}
			if (done == 1)
				break;
		}
	}
}
