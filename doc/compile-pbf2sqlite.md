# Compiling pbf2sqlite on Linux


## Dynamic binary for Linux

Required packages (Fedora):
```
sqlite-libs
sqlite-devel
```

The dynamic libs are in the following directory:
```
/usr/lib64/libsqlite3.so.0
/usr/lib64/libxml2.so.2
```

Compilation and installation in /usr/bin:
```
cd ./src
make
sudo make install
```

Show shared object dependencies for pbf2sqlite: `ldd pbf2sqlite`


## Static binary for Windows (64bit)

First the SQLite amalgamation files **sqlite3.c** and **sqlite3.h** must exist
in the directory `./src`, see <https://www.sqlite.org/amalgamation.html>.

Second the following files from **readosm** must exist in the directory `./src`:

config.h  
osm_objects.c  
osmxml.c  
protobuf.c  
readosm.c  
readosm.h  
readosm_internals.h  
readosm_protobuf.h  

Compilation for Windows systems with Linux and crosscompiler MinGW.

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

`make win64` should do the job.
