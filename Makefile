.POSIX:
PREFIX = /usr/local

.SUFFIXES:
all:
	cc -O3 -Wall -Wextra -Wno-missing-field-initializers -pedantic -lm -lsndio src/parse/*.c src/noise/*.c src/audio/*.c src/repl/*.c src/curves/*.c src/*.c -o "boar"
	cc -O3 -Wall -Wextra -Wno-missing-field-initializers -pedantic -lsndio src/midi/*.c -o "midi-boar"
install:
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man1
	cp boar $(PREFIX)/bin
	cp boar.1 $(PREFIX)/share/man/man1
uninstall:
	rm $(PREFIX)/bin/boar
	rm $(PREFIX)/share/man/man1/boar.1
