#ifndef HEADER_H
#define HEADER_H

#define BIN_NAME            "pbf2sqlite"
#define VERSION_NUMBER      "0.5.6 ALPHA"

#define QUICK_HELP \
"\n" \
BIN_NAME" "VERSION_NUMBER"\n" \
"\n" \
"OpenStreetMap PBF/XML importer for SQLite\n" \
"\n" \
"`"BIN_NAME"` imports OpenStreetMap data from `.osm.pbf` or `.osm` files into an SQLite database.\n" \
"It also provides commands for creating spatial indexes, address and routing tables, inspecting OSM elements,\n" \
"executing SQL statements, visualizing geographic data, and calculating shortest paths.\n" \
"\n" \
"Usage:\n" \
BIN_NAME" <database> [COMMAND ...]\n" \
"\n" \
"# Importing and preparing data\n" \
"\n" \
"| Command       | Description                                    |\n" \
"| ------------- | ---------------------------------------------- |\n" \
"| `read <file>` | Import data from an `.osm.pbf` or `.osm` file. |\n" \
"| `index`       | Create basic database indexes.                 |\n" \
"| `rtree`       | Create R*Tree spatial indexes.                 |\n" \
"| `addr`        | Create address tables.                         |\n" \
"| `graph`       | Create graph tables for routing.               |\n" \
"\n" \
"# Inspecting and querying data\n" \
"\n" \
"| Command         | Description                           |\n" \
"| --------------- | ------------------------------------- |\n" \
"| `node <id>`     | Display information about a node.     |\n" \
"| `way <id>`      | Display information about a way.      |\n" \
"| `relation <id>` | Display information about a relation. |\n" \
"| `sql [<stmt>]`  | Execute an SQL statement.             |\n" \
"\n" \
"# Map visualization\n" \
"\n" \
"| Command                                         | Description                                        |\n" \
"| ----------------------------------------------- | -------------------------------------------------- |\n" \
"| `vaddr <lon1> <lat1> <lon2> <lat2> <htmlfile>`  | Generate an HTML map displaying addresses.         |\n" \
"| `vgraph <lon1> <lat1> <lon2> <lat2> <htmlfile>` | Generate an HTML map displaying the routing graph. |\n" \
"\n" \
"Coordinates are specified as longitude/latitude pairs.\n" \
"\n" \
"# Shortest-path routing\n" \
"\n" \
"`route <mode> <lon1> <lat1> <lon2> <lat2> [<lon3> <lat3> ...] <file>`\n" \
"\n" \
"The route is calculated between the specified coordinates and written to `<file>`.\n" \
"\n" \
"Supported transportation modes (`<mode>`):\n" \
"- `foot` – Walking\n" \
"- `bike` – Cycling\n" \
"- `roadbike` – Road cycling\n" \
"- `car` – Driving\n" \
"\n"

#endif  /* HEADER_H */
