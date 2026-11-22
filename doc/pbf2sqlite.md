# 1. pbf2sqlite

A simple command-line tool that imports
[PBF](https://wiki.openstreetmap.org/wiki/PBF_Format)
or
[XML](https://wiki.openstreetmap.org/wiki/OSM_XML)
OpenStreetMap data into an SQLite database.

OSM data can be obtained from a provider such as [Geofabrik](https://download.geofabrik.de).  

Simple example:  
`pbf2sqlite test.db read country.osm.pbf`  

Advanced example with more commands:  
`pbf2sqlite test.db read country.osm.pbf index rtree addr graph`  

The created database can be easily queried using the [SQLite CLI tool](https://www.sqlite.org/cli.html).  


# 2. Main commands

> The **order of the commands** is important. All commands are executed in the order given.

## 2.1. Command "read"

This option first creates six tables, provided they do not already exist.  
Then, the OSM data ([PBF](https://wiki.openstreetmap.org/wiki/PBF_Format) or
[XML](https://wiki.openstreetmap.org/wiki/OSM_XML)) is imported.

#### Table "nodes"
column       | type                | description
-------------|---------------------|-------------------------------------
node_id      | INTEGER PRIMARY KEY | node ID
lon          | REAL                | longitude
lat          | REAL                | latitude

#### Table "node_tags"
column       | type                | description
-------------|---------------------|-------------------------------------
node_id      | INTEGER             | node ID
key          | TEXT                | tag key
value        | TEXT                | tag value

#### Table "way_nodes"
column       | type                | description
-------------|---------------------|-------------------------------------
way_id       | INTEGER             | way ID
node_id      | INTEGER             | node ID
node_order   | INTEGER             | node order

#### Table "way_tags"
column       | type                | description
-------------|---------------------|-------------------------------------
way_id       | INTEGER             | way ID
key          | TEXT                | tag key
value        | TEXT                | tag value

#### Table "relation_members"
column       | type                | description
-------------|---------------------|-------------------------------------
relation_id  | INTEGER             | relation ID
ref          | TEXT                | reference ('node','way','relation')
ref_id       | INTEGER             | node, way or relation ID
role         | TEXT                | describes a particular feature
member_order | INTEGER             | member order

#### Table "relation_tags"
column       | type                | description
-------------|---------------------|-------------------------------------
relation_id  | INTEGER             | relation ID
key          | TEXT                | tag key
value        | TEXT                | tag value

## 2.2. Command "index"

This option creates the following basic indexes:  

table            | index_name                    | columns
-----------------|-------------------------------|---------------------------
node_tags        | node_tags__node_id            | node_id
node_tags        | node_tags__key                | key
way_tags         | way_tags__way_id              | way_id
way_tags         | way_tags__key                 | key
way_nodes        | way_nodes__way_id             | way_id, node_order
way_nodes        | way_nodes__node_id            | node_id
relation_members | relation_members__relation_id | relation_id, member_order
relation_members | relation_members__ref_id      | ref_id
relation_tags    | relation_tags__relation_id    | relation_id
relation_tags    | relation_tags__key            | key
 
Finally, the [ANALYZE](https://www.sqlite.org/lang_analyze.html) command is executed.  

## 2.3. Command "rtree"

This option creates two [R*Tree](https://www.sqlite.org/rtree.html)
indexes **rtree_way** and **rtree_node** for finding ways and nodes quickly.  

Internally, the index **rtree_way** is created as follows:  
``` sql
CREATE VIRTUAL TABLE rtree_way USING rtree(way_id, min_lat, max_lat, min_lon, max_lon);

INSERT INTO rtree_way (way_id, min_lat, max_lat, min_lon, max_lon)
SELECT way_nodes.way_id,min(nodes.lat),max(nodes.lat),min(nodes.lon),max(nodes.lon)
FROM way_nodes
LEFT JOIN nodes ON way_nodes.node_id=nodes.node_id
GROUP BY way_nodes.way_id;
```

> It is strongly recommended to create the basic indexes using the **index** command
> *before* executing the **rtree** command.

#### Example queries

``` sql
/*
** Find all elements of the index (ways) that are contained within the boundingbox:
**    min_lon (x1):  7.851, min_lat (y1): 47.995
**    max_lon (x2):  7.854, max_lat (y2): 47.996
*/
SELECT way_id
FROM rtree_way
WHERE min_lon>= 7.851 AND max_lon<= 7.854
 AND  min_lat>=47.995 AND max_lat<=47.996;
/*
** Find all elements of the index (ways) that overlap the boundingbox:
*/
SELECT way_id
FROM rtree_way
WHERE max_lon>= 7.851 AND min_lon<= 7.854
 AND  max_lat>=47.995 AND min_lat<=47.996;
/*
** Limits of an element of the index:
*/
SELECT min_lon,max_lon,min_lat,max_lat
FROM rtree_way
WHERE way_id=4872512;
```


## 2.4. Command "addr"

This option creates two tables with the address data.  

> Only addr tags from the **node_tags** and **way_tags** tables are taken into account.  
> There are also a few addr tags in the **relation_tags** table; these are not considered.  

#### Table "addr_street"
column       | type                | description
-------------|---------------------|-------------------------------------
street_id    | INTEGER PRIMARY KEY | street ID
country      | TEXT                | country
postcode     | TEXT                | postcode
city         | TEXT                | city
street       | TEXT                | street
min_lon      | REAL                | boundingbox min. longitude
min_lat      | REAL                | boundingbox min. latitude
max_lon      | REAL                | boundingbox max. longitude
max_lat      | REAL                | boundingbox max. latitude

#### Table "addr_housenumber"
column         | type                | description
---------------|---------------------|-------------------------------------
street_id      | INTEGER             | street ID
housenumber    | TEXT                | housenumber
lon            | REAL                | longitude
lat            | REAL                | latitude
way_id         | INTEGER             | way ID
node_id        | INTEGER             | node ID

Index **addr_housenumber\_\_street_id** on column (street_id)

The view **addr_view** joins these two tables.


## 2.5. Command "graph"

This option creates additional tables with the complete graph of all highways.  
A graph is a set of vertices (nodes/points) connected by edges (links/lines).  
This data is required for routing purposes, for example.  

#### Table "graph_edges"
column          | type                | description
----------------|---------------------|-------------------------------------
edge_id         | INTEGER PRIMARY KEY | edge ID
start_node_id   | INTEGER             | edge start node ID
end_node_id     | INTEGER             | edge end node ID
dist            | INTEGER             | distance in meters
way_id          | INTEGER             | way ID
nodes           | INTEGER             | number of nodes
permit          | INTEGER             | bit field access

Index **graph_edges\_\_way_id** on column (way_id)

#### Table "graph_vertices"
column     | type                | description
-----------|---------------------|-------------------------------------
vertex_id  | INTEGER PRIMARY KEY | vertex ID
node_id    | INTEGER             | node ID
num_edges  | INTEGER             | number of edges

Index **graph_vertices\_\_node_id** on column (node_id)

#### Access for foot, bike and car

The bit field **permit** determines who may use this edge:  

| bit   | meaning     | decimal |
|-------|-------------|--------:|
| Bit 0 | foot        |       1 |
| Bit 1 | bike        |       2 |
| Bit 2 | car         |       4 |
| Bit 3 | paved       |       8 |
| Bit 4 | oneway_bike |      16 |
| Bit 5 | oneway_car  |      32 |
| Bit 6 | (not used)  |      64 |
| Bit 7 | (not used)  |     128 |

To fill the column **permit** a table **graph_permit** is needed:  

#### Table "graph_permit"
column     | type     | description
-----------|----------|-------------------------------------
key        | TEXT     | tag key
value      | TEXT     | tag value
set_bit    | INTEGER  | bitmask set bits
clear_bit  | INTEGER  | bitmask clear bits

This table specifies which tags set or clear which bits in **permit**.  
Initially, no bits are set in **permit**.  
Then, the bits are set according to the tags found (set_bit).  
Finally, the bits are cleared according to the tags found (clear_bit).  

> If table **graph_permit** doesn't exist, a new table will be created with default values.  
> But if the table already exists, it will be used.  
> This makes it possible to use one's own custom definition.


# 3. View data

## 3.1. Command "node", "way" and "relation"

The **node**, **way** and **relation** commands display information about
the respective objects on the console.

Examples:  
```
pbf2sqlite germany.db node 3378340880
pbf2sqlite germany.db way 424415177
pbf2sqlite germany.db relation 2201742
```

## 3.2. Command "vaddr" and "vgraph"

The **vaddr** option visualizes the addresses for a given area.  
The **vgraph** option visualizes the graph table for a given area.  
These commands generate an HTML file with zoomable maps.  
The area is defined by a point and the size of the square bounding box in meters.  

Examples:  
```
pbf2sqlite germany.db vaddr 13.37 52.52 1000 addr_bln.html
pbf2sqlite germany.db vgraph 11.57 48.13 1000 foot graph_mchn.html
```

## 3.3. Command "sql" 

The **sql** option executes an SQL command.

If the SQL command is a [SELECT](https://www.sqlite.org/lang_select.html)
statement, then the result will be displayed on the console.

SQL commands can also be entered via stdin:  
```
pbf2sqlite test.db sql < stmt.sql
cat stmt.sql | pbf2sqlite test.db sql
```

### Additional SQL functions

SQLite is extended with the following functions:

function                         | description
---------------------------------|----------------------------------------
distance(lon1, lat1, lon2, lat2) | Calculates distance in meters
mercator_x(lon)                  | Web Mercator projection X (EPSG:3857)
mercator_y(lat)                  | Web Mercator projection Y (EPSG:3857)

Comparison of WGS84 (lon, lat) with Web Mercator (x, y):  
```
                                 ^
               lat:  85.05112878 | y:  20037508.343
                                 |
x: -20037508.343                 |                 x: 20037508.343
---------------------------------0--------------------------------->
lon: -180°                       |                 lon: +180°
                                 |
               lat: -85.05112878 | y: -20037508.343
```

Examples:  
```
pbf2sqlite test.db sql "SELECT * FROM nodes LIMIT 10"
pbf2sqlite test.db sql "ALTER TABLE nodes ADD COLUMN x"
pbf2sqlite test.db sql "ALTER TABLE nodes ADD COLUMN y"
pbf2sqlite test.db sql "UPDATE nodes SET x=mercator_x(lon),y=mercator_y(lat)"
```

The following extensions are included:  
<https://www.sqlite.org/rtree.html>  
<https://www.sqlite.org/geopoly.html>  
<https://www.sqlite.org/lang_mathfunc.html>  
<https://www.sqlite.org/fts3.html>  


# 4. Calculate the shortest path

## 4.1. Command "route"

The **route** option calculates a shortest way.

Table **graph_edges** and **rtree_way** are required (commands **graph** and **rtree**).

Any number of intermediate destinations can also be specified.

Usage:  
```
pbf2sqlite <database> route <mode> <lon1> <lat1> <lon2> <lat2> [<lon3> <lat3> ...] <file>
```

`<mode>` is an integer that is combined with the **permit** field in the **graph_edges** table
using a bitwise AND operation to determine the usable edges.  

For ease of input, the following strings are also recognized and converted into the corresponding decimal number:  

| `<mode>`   | decimal | binary |
|------------|--------:|-------:|
| all        |       0 |   0000 |
| foot       |       1 |   0001 |
| bike       |       2 |   0010 |
| roadbike   |      10 |   1010 |
| car        |       4 |   0100 |

The result is written to three files (HTML, CSV and GPX).  
Therefore, `<file>` is supplemented with the file extensions **.html**, **.csv** and **.gpx**.  

> Note: Turning regulations for cars are currently not being evaluated.

Examples:  
```
pbf2sqlite germany.db route foot 11.5777 48.1427 11.5922 48.1524 11.5870 48.1623 route_mchn_foot
pbf2sqlite germany.db route bike 11.5777 48.1427 11.6031 48.1619 route_mchn_bike
pbf2sqlite germany.db route car 11.5777 48.1427 11.6031 48.1619 route_mchn_car
```


# Appendix

## Time requirements

| Command   | germany.osm.pbf (4,6G) | andorra.osm.pbf (3,2M) |
|-----------|-----------------------:|-----------------------:|
| read      | 15 min                 |                 0,85 s |
| index     | 22 min                 |                 0,62 s |
| rtree     | 2 h 13 min             |                 0,49 s |
| addr      | 22 min                 |                 0,04 s |
| graph     | 19 min                 |                 0,81 s |

## Links

<https://www.sqlite.org/lang_with.html#queries_against_a_graph>  
<https://www.geofabrik.de/data/routeable-vector-data.html>  
<https://www.geofabrik.de/data/routable-shapefile.pdf>  

