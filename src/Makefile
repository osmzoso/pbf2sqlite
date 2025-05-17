#
#
#
CC = gcc
CFLAGS = -Wall -O2 -s

pbf2sqlite : pbf2sqlite.c
	$(CC) pbf2sqlite.c -lsqlite3 -lreadosm -lm -o pbf2sqlite $(CFLAGS)

install: pbf2sqlite
	install -m755 pbf2sqlite /usr/bin

clean:
	rm -f pbf2sqlite

