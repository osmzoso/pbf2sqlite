#
#
#
CC = gcc
CFLAGS = -Wall -std=c99

# build directory and binary name
BDIR = ../build/
BNAME = pbf2sqlite

all:
	mkdir -p $(BDIR)
	$(CC) $(CFLAGS) -O2 -s main.c -lsqlite3 -lreadosm -lm -o $(BDIR)$(BNAME)

debug:
	mkdir -p $(BDIR)
	$(CC) $(CFLAGS) -O0 -g main.c -fsanitize=address -lasan -lsqlite3 -lreadosm -lm -o $(BDIR)$(BNAME)

#
# Static binary for Linux
#
static:
	mkdir -p $(BDIR)
	$(CC) \
     -static \
     $(CFLAGS) \
     -O2 \
     -s \
     -DSQLITE_THREADSAFE=0 \
     -DSQLITE_OMIT_LOAD_EXTENSION \
     -DSQLITE_ENABLE_RTREE \
     -DSQLITE_ENABLE_MATH_FUNCTIONS \
     main.c \
     ./sqlite3/sqlite3.c \
     ./readosm/osm_objects.c \
     ./readosm/osmxml.c \
     ./readosm/protobuf.c \
     ./readosm/readosm.c \
     -o $(BDIR)$(BNAME) \
     -I. \
     -I./sqlite3 \
     -I./readosm \
     -lexpat -lz -lm -lgcc

#
# Static binary for Windows (x86 64bit)
#
win64:
	mkdir -p $(BDIR)
	x86_64-w64-mingw32-gcc \
     -static \
     $(CFLAGS) \
     -O2 \
     -s \
     -DSQLITE_THREADSAFE=0 \
     -DSQLITE_OMIT_LOAD_EXTENSION \
     -DSQLITE_ENABLE_RTREE \
     -DSQLITE_ENABLE_MATH_FUNCTIONS \
     -D_FORTIFY_SOURCE=2 \
     main.c \
     ./sqlite3/sqlite3.c \
     ./readosm/osm_objects.c \
     ./readosm/osmxml.c \
     ./readosm/protobuf.c \
     ./readosm/readosm.c \
     -o $(BDIR)$(BNAME).exe \
     -I. \
     -I./sqlite3 \
     -I./readosm \
     -I/usr/x86_64-w64-mingw32/sys-root/mingw/include \
     -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib \
     -lexpat -lz -lpthread -lwinpthread -lws2_32 -lssp -lgcc

install:
	install -m755 $(BDIR)$(BNAME) /usr/bin

clean:
	rm -f $(BDIR)$(BNAME) $(BDIR)$(BNAME).exe

amalgamation:
	mkdir -p $(BDIR)
	$(CC) -E main.c | grep -v '^#' > $(BDIR)$(BNAME).c


.PHONY: debug static win64 install clean amalgamation
