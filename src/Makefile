#
#
#
CC = gcc
CFLAGS = -Wall -O2 -s

pbf2sqlite : pbf2sqlite.c
	$(CC) pbf2sqlite.c -lsqlite3 -lreadosm -lm -o pbf2sqlite $(CFLAGS)

clean:
	rm -f pbf2sqlite

