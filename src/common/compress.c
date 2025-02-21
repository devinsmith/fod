
#include <string.h>

#include "common/compress.h"

unsigned char g_hist[4096];

void decompress(struct stream *s, struct stream *out_s, uint32_t u_bytes)
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
			byte = stream_get8(s);
			holding_bits = 8;
		}
		if (byte & 0x01) {
			byte = byte >> 1;
			t_byte = stream_get8(s);
			stream_add8(out_s, t_byte);
			u_bytes--;
			if (u_bytes == 0) break;
			g_hist[g_hist_woff] = t_byte;
			g_hist_woff++;
			g_hist_woff = g_hist_woff & 0xfff;
		} else {
			byte = byte >> 1;
			t_byte = stream_get8(s);
			cl_byte = t_byte;
			t_byte = stream_get8(s);
			ch_byte = t_byte;
			t_byte = (t_byte & 0x0f) + 3;
			g_hist_roff =(((ch_byte >> 4) << 8) + cl_byte);

			for (j = 0; j < t_byte; j++) {
				al_byte = g_hist[g_hist_roff];
				stream_add8(out_s, al_byte);
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
