#
# Makefile pbf2sqlite
#
# C compiler, compiler flags, linker flags
CC = gcc
CFLAGS += -Wall -std=c99 -O2
LDFLAGS += -static -s

# build directory, binary name
BUILD_DIR = ./build/
BIN = pbf2sqlite

# source files
SRC = ./src/main.c
SRC_STATIC = \
 ./src/sqlite3/sqlite3.c \
 ./src/readosm/osm_objects.c \
 ./src/readosm/osmxml.c \
 ./src/readosm/protobuf.c \
 ./src/readosm/readosm.c

# documentation files
DOC_SRC = ./doc/pbf2sqlite.md
DOC_CSS = ./doc/custom.css

#
# use the recommended SQLite compile-time options, see
# https://www.sqlite.org/compile.html#recommended_compile_time_options
#
COMPILE_OPTIONS_SQLITE = \
 -DSQLITE_DQS=0 \
 -DSQLITE_THREADSAFE=0 \
 -DSQLITE_DEFAULT_MEMSTATUS=0 \
 -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=1 \
 -DSQLITE_LIKE_DOESNT_MATCH_BLOBS \
 -DSQLITE_MAX_EXPR_DEPTH=0 \
 -DSQLITE_OMIT_DECLTYPE \
 -DSQLITE_OMIT_DEPRECATED \
 -DSQLITE_OMIT_PROGRESS_CALLBACK \
 -DSQLITE_OMIT_SHARED_CACHE \
 -DSQLITE_OMIT_AUTOINIT \
 -DSQLITE_STRICT_SUBTYPE=1 \
 -DSQLITE_ENABLE_RTREE \
 -DSQLITE_ENABLE_MATH_FUNCTIONS

#
# main targets
#
.PHONY: all static install doc clean test debug doc2 amalgamation
all: bldir compile
static: clean bldir compile_static compile_static_win64 check_static_binaries render_doc
install:
	install -m755 $(BUILD_DIR)$(BIN) /usr/bin
doc: bldir render_doc
clean:
	rm -rf $(BUILD_DIR)
test: clean bldir compile_asan quicktest
debug: clean bldir compile_debug quicktest
doc2: bldir render_doc_src
amalgamation: bldir single_src

#
#
#
.PHONY: bldir compile compile_debug compile_asan compile_static compile_static_win64
.PHONY: quicktest check_static_binaries render_doc render_doc_src single_src
bldir:
	mkdir -p $(BUILD_DIR)

compile:
	$(CC) -Wall -std=c99 -O2 -s $(SRC) -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(BIN)
	ldd $(BUILD_DIR)$(BIN)

compile_debug:
	$(CC) -Wall -std=c99 -O0 -g -DDEBUG $(SRC) -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(BIN)

compile_asan:
	$(CC) -Wall -std=c99 -O0 -g -DDEBUG $(SRC) -fsanitize=address -lasan -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(BIN)

compile_static:
	$(CC) $(CFLAGS) $(LDFLAGS) $(COMPILE_OPTIONS_SQLITE) $(SRC) $(SRC_STATIC) \
 -o $(BUILD_DIR)$(BIN) \
 -I. -I./src/sqlite3 -I./src/readosm \
 -lexpat -lz -lm -lgcc
	upx --best $(BUILD_DIR)$(BIN)

compile_static_win64:
	x86_64-w64-mingw32-gcc $(CFLAGS) $(LDFLAGS) $(COMPILE_OPTIONS_SQLITE) $(SRC) $(SRC_STATIC) \
 -o $(BUILD_DIR)$(BIN).exe \
 -I. -I./src/sqlite3 -I./src/readosm \
 -I/usr/x86_64-w64-mingw32/sys-root/mingw/include \
 -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib \
 -lexpat -lz -lpthread -lwinpthread -lws2_32 -lssp -lgcc

quicktest:
	bash $(PWD)/test/run_test.sh $(PWD)/build $(PWD)/test/weimar.osm

check_static_binaries:
	bash $(PWD)/test/check_static_binaries.sh $(PWD)/build $(PWD)/test/weimar.osm

render_doc:
	pandoc \
     -V geometry:margin=0.6in \
     $(DOC_SRC) \
     --pdf-engine=xelatex \
     --toc \
     -o $(BUILD_DIR)$(BIN).pdf
	pandoc \
     --standalone \
     --embed-resources \
     --metadata title="$(BIN)" \
     --toc \
     --css=$(DOC_CSS) \
     $(DOC_SRC) \
     -o $(BUILD_DIR)$(BIN).html
	rm -f $(BUILD_DIR)$(BIN).1.gz
	pandoc \
     -s -f markdown -t man \
     $(DOC_SRC) \
     -o $(BUILD_DIR)$(BIN).1
	gzip $(BUILD_DIR)$(BIN).1

render_doc_src:
	doxygen
	xdg-open ./build/html/index.html

single_src:
	$(CC) -E $(SRC) | grep -v '^#' > $(BUILD_DIR)$(BIN).c

