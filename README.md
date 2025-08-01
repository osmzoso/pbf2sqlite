# pbf2sqlite

A simple command line tool for reading OpenStreetMap .osm.pbf files into a SQLite database.

```
Usage:
pbf2sqlite DATABASE [OPTION ...]

Main options:
  read FILE     Reads FILE into the database
                (.osm.pbf or .osm)
  rtree         Add R*Tree indexes
  addr          Add address tables
  graph         Add graph table
```

The command
```
pbf2sqlite test.db read country.osm.pbf
```
reads the [OSM PBF](https://wiki.openstreetmap.org/wiki/PBF_Format)
file **country.osm.pbf** and creates in the database **test.db** the tables.

The tables created are described in the [documentation](doc/pbf2sqlite.md).

OSM data can be obtained from a provider such as [Geofabrik](https://download.geofabrik.de).

[Notes on compiling](doc/compiling.md)

|[**Download the latest version**](https://github.com/osmzoso/pbf2sqlite/releases/latest)|
|----------------------------------------------------------------------------------------|

