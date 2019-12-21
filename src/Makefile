SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
CFLAGS = -pipe -Wall -fomit-frame-pointer -std=c18 -DNIX -DMULTIPLE_SCORE_ENTRY -DEXTRA
LDFLAGS = -lm -lncurses

larn: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

-include $(DEP)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(OBJ) larn

.PHONY: cleandep
cleandep:
	rm -f $(DEP)
