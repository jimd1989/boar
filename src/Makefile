.POSIX:
PREFIX = /usr/local

.SUFFIXES:
all:
	chicken-csc -debug-level 3 -L -lsndio -O0 ./audio.c ./boar.scm -o boar-scm
install:
	mkdir -p $(PREFIX)/bin
	cp boar-scm $(PREFIX)/bin
uninstall:
	rm $(PREFIX)/bin/boar-scm
