# Makefile for Larn

CC      = cc
CFLAGS  = -O3 -fstrict-aliasing -fno-strict-overflow -fno-delete-null-pointer-checks -fno-common -Wall -Wextra -Wshadow -Wstrict-prototypes -Wmissing-prototypes -DMULTIPLE_SCORE_ENTRY
LDFLAGS = -lm -lncurses

include mk/objects.mk

all: larn

larn: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o larn $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o larn
