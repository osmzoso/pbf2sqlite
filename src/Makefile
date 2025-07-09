#
#
#
CC = gcc
CFLAGS = -Wall -O2 -s -std=c99

# Source files
SRC = pbf2sqlite.c show.c
OBJ = $(SRC:.c=.o)
BIN = pbf2sqlite

# Compiling
$(BIN): $(SRC)
	$(CC) $(SRC) -lsqlite3 -lreadosm -lm -o $(BIN) $(CFLAGS)

install:
	install -m755 $(BIN) /usr/bin

win64:
	x86_64-w64-mingw32-gcc \
     -static \
     -O2 -s \
     -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_ENABLE_RTREE \
     -D_FORTIFY_SOURCE=2 \
     $(SRC) \
     sqlite3.c \
     osm_objects.c \
     osmxml.c \
     protobuf.c \
     readosm.c \
     -o $(BIN).exe \
     -I. \
     -I/usr/x86_64-w64-mingw32/sys-root/mingw/include \
     -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib \
     -lexpat -lz -liconv -lpthread -lwinpthread -lws2_32 -lssp -lgcc

clean:
	rm -f \
     $(BIN) \
     $(BIN).exe \
     sqlite3.c \
     sqlite3.h \
     config.h \
     osm_objects.c \
     osmxml.c \
     protobuf.c \
     readosm.c \
     readosm.h \
     readosm_internals.h \
     readosm_protobuf.h

.PHONY: clean install win64
