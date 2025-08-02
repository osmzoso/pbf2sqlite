# Compiling pbf2sqlite on Linux

The [readosm library](https://www.gaia-gis.it/fossil/readosm/index)
is used for this program.


## Dynamic binary for Linux

Required packages (Fedora):
```
sqlite-libs
sqlite-devel
readosm
readosm-devel
```

The dynamic libs are in the following directory:
```
/usr/lib64/libsqlite3.so.0
/usr/lib64/libreadosm.so.1
/usr/lib64/libexpat.so.1
```

Compile with
```
cd ./src
make
```
The binary is in the **/build** directory.  
(install in **/usr/bin** with `sudo make install`)  

Create the documentation files (pandoc is required):
```
cd ./doc
make
```
The doc files are also in the **/build** directory.  


## Build a static binary for Windows (64bit)

Compilation for Windows systems with Linux and crosscompiler MinGW:

Required packages (Fedora):
```
mingw64-gcc
mingw64-expat
mingw64-expat-static
mingw64-zlib
mingw64-zlib-static
mingw64-win-iconv
mingw64-win-iconv-static
mingw64-winpthreads
mingw64-winpthreads-static
```

The static libs are in the following directories:
```
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libexpat.a
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libwinpthread.a
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libz.a
/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libws2_32.a
/usr/lib/gcc/x86_64-w64-mingw32/12.2.1/libgcc.a
```

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

Compile with
```
cd ./src
make win64
```
The binary is in the **/build** directory.  
