#
#
#
CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS =

# build directory, binary name
BUILD_DIR = ./build/
TARGET = pbf2sqlite

# source files, doc
SRC = ./src/main.c
DOC_SRC = ./doc/pbf2sqlite.md
DOC_CSS = ./doc/custom.css

#
# main targets
#
.PHONY: all static install doc clean test debug doc2 amalgamation
all: bldir compile
static: clean bldir compile_static compile_static_win64 check_static_binaries renderdoc
install:
	install -m755 $(BUILD_DIR)$(TARGET) /usr/bin
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
.PHONY: bldir
bldir:
	mkdir -p $(BUILD_DIR)

.PHONY: compile
compile:
	$(CC) $(CFLAGS) -O2 -s $(SRC) -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(TARGET)

.PHONY: compile_debug
compile_debug:
	$(CC) $(CFLAGS) -O0 -g $(SRC) -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(TARGET) -DDEBUG

.PHONY: compile_asan
compile_asan:
	$(CC) $(CFLAGS) -O0 -g $(SRC) -fsanitize=address -lasan -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(TARGET) -DDEBUG

.PHONY: compile_static
compile_static:
	$(CC) -static $(CFLAGS) -O2 -s \
     -DSQLITE_THREADSAFE=0 \
     -DSQLITE_OMIT_LOAD_EXTENSION \
     -DSQLITE_ENABLE_RTREE \
     -DSQLITE_ENABLE_MATH_FUNCTIONS \
     $(SRC) \
     ./src/sqlite3/sqlite3.c \
     ./src/readosm/osm_objects.c \
     ./src/readosm/osmxml.c \
     ./src/readosm/protobuf.c \
     ./src/readosm/readosm.c \
     -o $(BUILD_DIR)$(TARGET) \
     -I. -I./src/sqlite3 -I./src/readosm \
     -lexpat -lz -lm -lgcc

.PHONY: compile_static_win64
compile_static_win64:
	x86_64-w64-mingw32-gcc -static $(CFLAGS) -O2 -s \
     -DSQLITE_THREADSAFE=0 \
     -DSQLITE_OMIT_LOAD_EXTENSION \
     -DSQLITE_ENABLE_RTREE \
     -DSQLITE_ENABLE_MATH_FUNCTIONS \
     -D_FORTIFY_SOURCE=2 \
     $(SRC) \
     ./src/sqlite3/sqlite3.c \
     ./src/readosm/osm_objects.c \
     ./src/readosm/osmxml.c \
     ./src/readosm/protobuf.c \
     ./src/readosm/readosm.c \
     -o $(BUILD_DIR)$(TARGET).exe \
     -I. -I./src/sqlite3 -I./src/readosm \
     -I/usr/x86_64-w64-mingw32/sys-root/mingw/include \
     -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib \
     -lexpat -lz -lpthread -lwinpthread -lws2_32 -lssp -lgcc

.PHONY: quicktest
quicktest:
	bash $(PWD)/test/run_test.sh $(PWD)/build $(PWD)/test/weimar.osm

.PHONY: check_static_binaries
check_static_binaries:
	bash $(PWD)/test/check_static_binaries.sh $(PWD)/build $(PWD)/test/weimar.osm

.PHONY: render_doc
render_doc:
	pandoc \
     -V geometry:margin=0.6in \
     $(DOC_SRC) \
     --pdf-engine=xelatex \
     --toc \
     -o $(BUILD_DIR)$(TARGET).pdf
	pandoc \
     --standalone \
     --embed-resources \
     --metadata title="$(TARGET)" \
     --toc \
     --css=$(DOC_CSS) \
     $(DOC_SRC) \
     -o $(BUILD_DIR)$(TARGET).html
	rm -f $(BUILD_DIR)$(TARGET).1.gz
	pandoc \
     -s -f markdown -t man \
     $(DOC_SRC) \
     -o $(BUILD_DIR)$(TARGET).1
	gzip $(BUILD_DIR)$(TARGET).1

.PHONY: render_doc_src
render_doc_src:
	doxygen
	xdg-open ./build/html/index.html

.PHONY: single_src
single_src:
	$(CC) -E $(SRC) | grep -v '^#' > $(BUILD_DIR)$(TARGET).c

