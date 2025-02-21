# Makefile Fountain of Dreams installer.
#

SRCS = fod_install.c hexdump.c stream.c
OBJS = $(SRCS:.c=.o)

ALL_OBJS = $(OBJS)

EXE = fod_install

CC = gcc
INTERNAL_CFLAGS = -Wall
CFLAGS= -O2 -pipe
DEBUG_CFLAGS = -ggdb3
LFLAGS =
LIBS =

default:
	@$(MAKE) $(EXE)

debug:
	@$(MAKE) CFLAGS=$(DEBUG_CFLAGS) $(EXE)

$(EXE): $(OBJS)
	$(CC) $(INTERNAL_CFLAGS) $(CFLAGS) -o $(EXE) $(OBJS) $(LFLAGS) $(LIBS)

clean:
	rm -f $(ALL_OBJS) $(EXE)

.c.o:
	$(CC) $(INTERNAL_CFLAGS) $(CFLAGS) -o $@ -c $<
