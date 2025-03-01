# Makefile
# Fountain of Dreams game

COMMON_SRCS = common/compress.c common/stream.c common/hexdump.c
GAME_SRCS = decode.c
INSTALL_SRCS = install.c

COMMON_OBJS = $(COMMON_SRCS:.c=.o)
GAME_OBJS = $(GAME_SRCS:.c=.o)
INSTALL_OBJS = $(INSTALL_SRCS:.c=.o)

ALL_OBJS = $(COMMON_OBJS) $(GAME_OBJS) $(INSTALL_OBJS)
DEPS = $(ALL_OBJS:.o=.d)

CC = gcc
INTERNAL_CFLAGS = -Wall -Wextra -I.
CFLAGS= -O2 -pipe
DEBUG_CFLAGS = -ggdb3
LFLAGS =
LIBS =

default: fod install

fod: $(COMMON_OBJS) $(GAME_OBJS)
	$(CC) $(INTERNAL_CFLAGS) $(CFLAGS) -o fod $(COMMON_OBJS) $(GAME_OBJS) $(LFLAGS) $(LIBS)

install: $(COMMON_OBJS) $(INSTALL_OBJS)
	$(CC) $(INTERNAL_CFLAGS) $(CFLAGS) -o install $(COMMON_OBJS) $(INSTALL_OBJS) $(LFLAGS) $(LIBS)

clean:
	rm -f $(ALL_OBJS) fod install $(DEPS)

.c.o:
	$(CC) $(INTERNAL_CFLAGS) $(CFLAGS) -MMD -MP -MT $@ -o $@ -c $<

# Include automatically generated dependency files
-include $(DEPS)
