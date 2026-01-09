.POSIX:
PREFIX = /usr/local

.SUFFIXES:
all:
	chicken-csc -L -lsndio -lfa2 -strict-types -O3 -specialize snd.scm -o boar-scm
install:
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man1
	cp boar $(PREFIX)/bin
	cp boar.1 $(PREFIX)/share/man/man1
uninstall:
	rm $(PREFIX)/bin/boar
	rm $(PREFIX)/share/man/man1/boar.1
