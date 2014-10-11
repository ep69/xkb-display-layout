all: xkb-display-layout

xkb-display-layout.o: xkb-display-layout.c
	gcc -std=gnu99 -c -pedantic -Wall -O2 -o $@ $^

xkb-display-layout: xkb-display-layout.o
	gcc -lX11 -o $@ $^

clean: 
	rm -f xkb-display-layout *.o

dist:
	tar cfa xkb-display-layout.tar.gz Makefile COPYING README.md xkb-display-layout.c


