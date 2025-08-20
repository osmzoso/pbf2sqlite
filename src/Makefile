#
#
#
CC = gcc
CFLAGS = -Wall -O2 -s -std=c99

# Source files
SRC = main.c \
      pbf2sqlite.h \
      read_data.c \
      opt_data.c \
      show_data.c \
      add_addr.sql \
      add_graph_permit.sql \
      add_rtree.sql \
      leaflet.c \
      visualize_data.c
CFILES = $(filter %.c, $(SRC))
BIN = ../build/pbf2sqlite

# Compiling
$(BIN): $(SRC)
	mkdir -p $(dir $(BIN))
	$(CC) $(CFILES) -lsqlite3 -lreadosm -lm -o $(BIN) $(CFLAGS)

win64:
	mkdir -p $(dir $(BIN))
	x86_64-w64-mingw32-gcc \
     -static \
     $(CFLAGS) \
     -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_ENABLE_RTREE \
     -D_FORTIFY_SOURCE=2 \
     $(CFILES) \
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
     -lexpat -lz -lpthread -lwinpthread -lws2_32 -lssp -lgcc

#
# Experimental: Static binary for linux
#
# Additionally, the following Fedora packages are required:
#  sudo dnf install expat-static
#  sudo dnf install glibc-static
#  sudo dnf install zlib-ng-compat-static
#
# In ./src/readosm/readosm.c in line 50 add the following lines:
#   #ifdef __linux__
#   #include <strings.h>
#   #endif
#
static:
	$(CC) \
     -static \
     $(CFLAGS) \
     -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_ENABLE_RTREE \
     $(CFILES) \
     ./sqlite3/sqlite3.c \
     ./readosm/osm_objects.c \
     ./readosm/osmxml.c \
     ./readosm/protobuf.c \
     ./readosm/readosm.c \
     -o $(BIN)_x86_64 \
     -I. \
     -I./sqlite3 \
     -I./readosm \
     -lexpat -lz -lm -lgcc

# install -m755 $(BIN) /usr/bin
install:
	cp $(BIN) /usr/bin
	cp $(BIN)_x86_64 /usr/bin

clean:
	rm -f $(BIN) $(BIN).exe $(BIN)_x86_64


.PHONY: win64 static install clean
