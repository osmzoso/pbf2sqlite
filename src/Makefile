#
#
#
CC = gcc
CFLAGS = -Wall -O2 -s -std=c99

# Source files
SRC = main.c read_data.c add_data.c show_data.c
OBJ = $(SRC:.c=.o)
BIN = ../build/pbf2sqlite

# Compiling
$(BIN): $(SRC)
	mkdir -p $(dir $(BIN))
	$(CC) $(SRC) -lsqlite3 -lreadosm -lm -o $(BIN) $(CFLAGS)

install:
	install -m755 $(BIN) /usr/bin

win64:
	mkdir -p $(dir $(BIN))
	x86_64-w64-mingw32-gcc \
     -static \
     $(CFLAGS) \
     -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_ENABLE_RTREE \
     -D_FORTIFY_SOURCE=2 \
     $(SRC) \
     ./sqlite3/sqlite3.c \
     ./readosm/osm_objects.c \
     ./readosm/osmxml.c \
     ./readosm/protobuf.c \
     ./readosm/readosm.c \
     -o $(BIN).exe \
     -I. \
     -I./sqlite3 \
     -I./readosm \
     -I/usr/x86_64-w64-mingw32/sys-root/mingw/include \
     -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib \
     -lexpat -lz -liconv -lpthread -lwinpthread -lws2_32 -lssp -lgcc

clean:
	rm -f $(BIN) $(BIN).exe

.PHONY: clean install win64
