# Compiling pbf2sqlite on Linux


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

Compilation and installation in /usr/bin:
```
cd ./src
make
sudo make install
```

Show shared object dependencies for pbf2sqlite: `ldd pbf2sqlite`


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

First, the following files must also be present in the /src directory:

The [SQLite amalgamation files](https://www.sqlite.org/amalgamation.html):  
sqlite3.c  
sqlite3.h  

Second, the source code files of the [readosm library](https://www.gaia-gis.it/fossil/readosm/index):

config.h  
osm_objects.c  
osmxml.c  
protobuf.c  
readosm.c  
readosm.h  
readosm_internals.h  
readosm_protobuf.h  

`make win64` should do the job.
