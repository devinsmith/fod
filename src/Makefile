# Makefile
# Fountain of Dreams game

SRCS = common/compress.c common/stream.c common/hexdump.c decode.c

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

ALL_OBJS = $(OBJS)

EXE = fod

CC = gcc
INTERNAL_CFLAGS = -Wall -Wextra -I.
CFLAGS= -O2 -pipe
DEBUG_CFLAGS = -ggdb3
LFLAGS =
LIBS =

default: $(EXE)

debug:
	@$(MAKE) CFLAGS=$(DEBUG_CFLAGS) $(EXE)

$(EXE): $(OBJS)
	$(CC) $(INTERNAL_CFLAGS) $(CFLAGS) -o $(EXE) $(OBJS) $(LFLAGS) $(LIBS)

clean:
	rm -f $(ALL_OBJS) $(EXE) $(DEPS)

.c.o:
	$(CC) $(INTERNAL_CFLAGS) $(CFLAGS) -MMD -MP -MT $@ -o $@ -c $<

# Include automatically generated dependency files
-include $(DEPS)
