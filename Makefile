# Makefile for Larn

include mk/objects.mk

CC      = cc
CFLAGS  = -pedantic -pedantic-errors -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wold-style-definition -Wnested-externs -Wredundant-decls -Wstrict-overflow=5 -Wconversion -Wsign-conversion -Wfloat-equal -Wundef -Wbad-function-cast -Wmissing-field-initializers -Waggregate-return -Wstrict-aliasing=3 -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wstringop-overflow=4 -Wstringop-truncation -Walloc-zero -Walloca -Wvla -Wpacked -Winline -Wdisabled-optimization -Wswitch-enum -Wswitch-default -Wunreachable-code -Wunused -Wunused-parameter -Wunused-function -Wunused-variable -Wunused-macros -Wunused-const-variable -Wunused-value -unused-result -Wunused-label -Wunused-but-set-variable -Wunused-but-set-parameter -Wdouble-promotion -Wjump-misses-init -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -O3 -fstrict-aliasing -fno-strict-overflow -fno-delete-null-pointer-checks -fno-common -fwrapv -fstack-protector-all -DMULTIPLE_SCORE_ENTRY
LDFLAGS = -lm -lncurses

all: larn

larn: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o larn $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o larn
