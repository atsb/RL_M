# Makefile for Larn

CC      = cc
CFLAGS  = -Wall -Wextra -Werror -pedantic -DMULTIPLE_SCORE_ENTRY
LDFLAGS = -lm -lncurses

include mk/objects.mk

all: larn

larn: $(OBJS)
    $(CC) $(CFLAGS) $(OBJS) -o larn $(LDFLAGS)

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f *.o larn
