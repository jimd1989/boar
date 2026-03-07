.POSIX:
PREFIX = /usr/local

.SUFFIXES:
all:
	chicken-csc -L -lsndio -O3 boar.scm -o boar-scm
install:
	mkdir -p $(PREFIX)/bin
	cp boar-scm $(PREFIX)/bin
uninstall:
	rm $(PREFIX)/bin/boar-scm
