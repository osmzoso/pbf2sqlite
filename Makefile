#
#
#
CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS =

# build directory, binary name, source files
BUILD_DIR = ./build/
TARGET = pbf2sqlite
SRC = ./src/main.c

DOC_SRC = ./doc/pbf2sqlite.md
DOC_CSS = ./doc/custom.css


all: bldir compile
test: bldir debug runtest
doc: bldir renderdoc
release: bldir compile_static renderdoc
amalgamation: bldir single_src


bldir:
	mkdir -p $(BUILD_DIR)

compile:
	$(CC) $(CFLAGS) -O2 -s $(SRC) -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(TARGET)

debug:
	$(CC) $(CFLAGS) -O0 -g $(SRC) -lsqlite3 -lreadosm -lm -o $(BUILD_DIR)$(TARGET) -DDEBUG

compile_static:
	@echo "TODO: compile static..."

runtest:
	@echo "TODO: run tests..."

renderdoc:
	pandoc \
     --standalone \
     --embed-resources \
     --metadata title="$(TARGET)" \
     --toc \
     --css=$(DOC_CSS) \
     $(DOC_SRC) \
     -o $(BUILD_DIR)$(TARGET).html

single_src:
	mkdir -p $(BDIR)
	$(CC) -E main.c | grep -v '^#' > $(BDIR)$(BNAME).c

install:
	install -m755 $(BUILD_DIR)$(TARGET) /usr/bin

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test doc release amalgamation bldir compile debug compile_static runtest renderdoc single_src install clean
