SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
NCURSES = $(shell pkg-config --libs ncurses)
CFLAGS = -Wall -g -Werror -std=gnu99 -pedantic -DNIX -DMULTIPLE_SCORE_ENTRY -DEXTRA
LDFLAGS = -lm $(NCURSES)

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

.PHONY: prep
prep:
	mkdir ~/.larn
	cp ./larnmazefile ~/.larn
	cp ./larnforts ~/.larn
