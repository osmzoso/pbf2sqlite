# pbf2sqlite

A simple command line tool for reading OpenStreetMap .osm.pbf files into a SQLite database.

```
Usage:
pbf2sqlite DATABASE [OPTION ...]

Options:
  read FILE    Reads .osm or .osm.pbf FILE into the database
  rtree        Add R*Tree indexes
  addr         Add address tables
  graph        Add graph table
```

The command
```
pbf2sqlite test.db read country.osm.pbf
```
reads the [OSM PBF](https://wiki.openstreetmap.org/wiki/PBF_Format)
file **country.osm.pbf** and creates in the database **test.db** the tables.

OSM data can be obtained from a provider such as [Geofabrik](https://download.geofabrik.de).

Linux Fedora:  
The readosm packages are required for compilation:  
`sudo dnf install readosm readosm-devel`  
<https://www.gaia-gis.it/fossil/readosm/index>  
The Python module osmium is required for testing:  
`sudo dnf install python3-osmium`  
<https://docs.osmcode.org/pyosmium/latest/index.html>  

