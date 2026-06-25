CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lncurses

game: game.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

update:
	curl -L -o game.c https://raw.githubusercontent.com/SkywaterBrown/Progressbar88/main/game.c
	curl -L -o Makefile https://raw.githubusercontent.com/SkywaterBrown/Progressbar88/main/Makefile

clean:
	rm -f game

.PHONY: update clean
