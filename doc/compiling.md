# Compiling on Linux

make options:
```
make               ;# Build dynamic binary for Linux
make static        ;# Build static binaries for Linux and Windows
make install       ;# Copy binary in /usr/bin (sudo is required)
make doc           ;# Create the documentation files (pandoc is required)
make clean         ;# Remove ./build directory
# Options for testing purposes only
make test          ;# Build dynamic binary with debug info and libasan, run some tests
make debug         ;# Build dynamic binary with debug info
make doc2          ;# Create source code documentation (doxygen is required)
make amalgamation  ;# Combine all source code into a single file 
```

All generated files can be found in the automatically created **./build** directory.


## Build dynamic binary for Linux

Additional required packages (Fedora):
```
sqlite-libs
sqlite-devel
readosm
readosm-devel
```

Compile with `make`


## Build static binaries for Linux and Windows

Additional source files in the /src tree are required to create a static binary.

The following files must also be present in the /src path:

In subdirectory ./src/sqlite3/ the [SQLite amalgamation files](https://www.sqlite.org/amalgamation.html):  
```
sqlite3.c
sqlite3.h
```

In subdirectory ./src/readosm/ the source files of [readosm library](https://www.gaia-gis.it/fossil/readosm/index):  
```
config.h
osm_objects.c
osmxml.c
protobuf.c
readosm.c
readosm.h
readosm_internals.h
readosm_protobuf.h
```

Unfortunately, there are compiler warnings.  
Therefore, in ./src/readosm/readosm.c in line 50 add the following lines:  
```
#ifdef __linux__
#include <strings.h>
#endif
```

Additional required packages for the static Linux version (Fedora):
```
expat-static
glibc-static
zlib-ng-compat-static
```

Additional required packages for the static Windows version (Fedora):
```
mingw64-gcc
mingw64-expat
mingw64-expat-static
mingw64-zlib
mingw64-zlib-static
mingw64-winpthreads
mingw64-winpthreads-static
```

Compile with `make static`


## Library directories in Fedora

The dynamic libraries are located in the following directory:  
```
/usr/lib64/libsqlite3.so.0
/usr/lib64/libreadosm.so.1
/usr/lib64/libexpat.so.1
```

The static libraries are located in the following directories:  
```
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libexpat.a
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libwinpthread.a
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libz.a
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libws2_32.a
/usr/lib/gcc/x86_64-w64-mingw32/12.2.1/libgcc.a
```

