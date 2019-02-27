.POSIX:
PREFIX = /usr/local

.SUFFIXES:
all:
	cc -O3 -ansi -Wall -Wextra -Wno-missing-field-initializers -pedantic -lsndio -lm -pthread *.c -o "boar"
install:
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man1
	cp boar $(PREFIX)/bin
	cp boar.1 $(PREFIX)/share/man/man1
uninstall:
	rm $(PREFIX)/bin/boar
	rm $(PREFIX)/share/man/man1/boar.1
