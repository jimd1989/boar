.POSIX:
PREFIX = /usr/local

.SUFFIXES:
all:
	chicken-csc -O3 -strict-types -C -O3 -C -ffast-math -C -march=native -L -lsndio ./audio.c ./boar.scm -o boar-scm
install:
	mkdir -p $(PREFIX)/bin
	cp boar-scm $(PREFIX)/bin
uninstall:
	rm $(PREFIX)/bin/boar-scm
