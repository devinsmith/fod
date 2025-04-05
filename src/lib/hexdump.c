#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "hexdump.h"

void
hexdump(void *vp, int length)
{
	char linebuf[80];
	int i;
	int linebuf_dirty = 0;
	unsigned char *p = (unsigned char *)vp;

	memset(linebuf, ' ', sizeof(linebuf));
	linebuf[70] = '\0';

	for (i=0; i < length; ++i) {
		int x = i % 16;
		int ch = (unsigned)p[i];
		char hex[20];
		if (x >= 8)
			x = x * 3 + 1;
		else
			x = x * 3;
		snprintf(hex, sizeof(hex), "%02x", ch);
		linebuf[x] = hex[0];
		linebuf[x+1] = hex[1];

		if (isprint(ch))
			linebuf[52+(i%16)] = ch;
		else
			linebuf[52+(i%16)] = '.';

		linebuf_dirty = 1;
		if (!((i+1)%16)) {
			printf("%s\n", linebuf);
			memset(linebuf, ' ', sizeof(linebuf));
			linebuf[70] = '\0';
			linebuf_dirty = 0;
		}
	}
	if (linebuf_dirty == 1)
		printf("%s\n", linebuf);
}

