# 1. pbf2sqlite

A simple command line tool for reading a
[OpenStreetMap PBF file](https://wiki.openstreetmap.org/wiki/PBF_Format)
into a SQLite database.

Alternatively, [OpenStreetMap XML data](https://wiki.openstreetmap.org/wiki/OSM_XML)
can also be read.

OSM data can be obtained from a provider such as [Geofabrik](https://download.geofabrik.de).

A simple example:  

`pbf2sqlite test.db read country.osm.pbf`  

This command reads the file **country.osm.pbf** and creates the tables
described below in the **test.db** database.

An extended example:  

`pbf2sqlite test.db read country.osm.pbf addr rtree graph`  

This command first reads the file and then creates additional data.

> The **read** option is always executed first

The database can be easily queried with the [SQLite CLI tool](https://www.sqlite.org/cli.html).


# 2. Tables

The **read** option creates the following tables and indexes in the database:

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
- INDEX node_tags__node_id ON node_tags (node_id)
- INDEX node_tags__key     ON node_tags (key)

#### Table "way_nodes"
column       | type                | description
-------------|---------------------|-------------------------------------
way_id       | INTEGER             | way ID
node_id      | INTEGER             | node ID
node_order   | INTEGER             | node order
- INDEX way_nodes__way_id  ON way_nodes (way_id, node_order)
- INDEX way_nodes__node_id ON way_nodes (node_id)

#### Table "way_tags"
column       | type                | description
-------------|---------------------|-------------------------------------
way_id       | INTEGER             | way ID
key          | TEXT                | tag key
value        | TEXT                | tag value
- INDEX way_tags__way_id   ON way_tags (way_id)
- INDEX way_tags__key      ON way_tags (key)

#### Table "relation_members"
column       | type                | description
-------------|---------------------|-------------------------------------
relation_id  | INTEGER             | relation ID
ref          | TEXT                | reference ('node','way','relation')
ref_id       | INTEGER             | node, way or relation ID
role         | TEXT                | describes a particular feature
member_order | INTEGER             | member order
- INDEX relation_members__relation_id ON relation_members (relation_id, member_order)
- INDEX relation_members__ref_id      ON relation_members (ref_id)

#### Table "relation_tags"
column       | type                | description
-------------|---------------------|-------------------------------------
relation_id  | INTEGER             | relation ID
key          | TEXT                | tag key
value        | TEXT                | tag value
- INDEX relation_tags__relation_id    ON relation_tags (relation_id)
- INDEX relation_tags__key            ON relation_tags (key)

> The **noindex** option suppresses the creation of all indexes (not recommended)


# 3. Create additional data

Additional tables can be created from the tables created with the **read** option.

## 3.1. Option "rtree"

This option creates additional [R*Tree](https://www.sqlite.org/rtree.html)
indexes **rtree_node** and **rtree_way** for finding ways quickly.  

Internally, the index **rtree_way** is created as follows:  
``` sql
CREATE VIRTUAL TABLE rtree_way USING rtree(way_id, min_lat, max_lat, min_lon, max_lon);

INSERT INTO rtree_way (way_id, min_lat, max_lat, min_lon, max_lon)
SELECT way_nodes.way_id,min(nodes.lat),max(nodes.lat),min(nodes.lon),max(nodes.lon)
FROM way_nodes
LEFT JOIN nodes ON way_nodes.node_id=nodes.node_id
GROUP BY way_nodes.way_id;
```

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


## 3.2. Option "addr"

This option creates two tables with address data.  

#### Table "addr_street"
column       | type                | description
-------------|---------------------|-------------------------------------
street_id    | INTEGER PRIMARY KEY | street ID
postcode     | TEXT                | postcode
city         | TEXT                | city
street       | TEXT                | street
min_lon      | REAL                | boundingbox min. longitude
min_lat      | REAL                | boundingbox min. latitude
max_lon      | REAL                | boundingbox max. longitude
max_lat      | REAL                | boundingbox max. latitude
- INDEX addr_street_1 ON addr_street (postcode,city,street)

#### Table "addr_housenumber"
column       | type                | description
-------------|---------------------|-------------------------------------
housenumber_id | INTEGER PRIMARY KEY | housenumber ID
street_id      | INTEGER             | street ID
housenumber    | TEXT                | housenumber
lon            | REAL                | longitude
lat            | REAL                | latitude
way_id         | INTEGER             | way ID
node_id        | INTEGER             | node ID
- INDEX addr_housenumber_1 ON addr_housenumber (street_id)

The view **addr_view** join the two tables.


## 3.3. Option "graph"

This option creates an additional table **graph** with the complete graph
of all highways.  
This data is required for routing purposes, for example.  

#### Table "graph"
column          | type                | description
----------------|---------------------|-------------------------------------
edge_id         | INTEGER PRIMARY KEY | edge ID
start_node_id   | INTEGER             | edge start node ID
end_node_id     | INTEGER             | edge end node ID
dist            | INTEGER             | distance in meters
way_id          | INTEGER             | way ID
nodes           | INTEGER             | number of nodes
permit          | INTEGER             | bit field access

The bit field **permit** determines who may use this edge:  

bit   | meaning     | value decimal
------|-------------|----------------
Bit 0 | foot        | 2^0   1
Bit 1 | bike        | 2^1   2
Bit 2 | car         | 2^2   4
Bit 3 | paved       | 2^3   8
Bit 4 | oneway_bike | 2^4  16
Bit 5 | oneway_car  | 2^5  32
Bit 6 | (not used)  | 2^6  64
Bit 7 | (not used)  | 2^7 128

To fill the column **permit** a table **graph_permit** is needed.  
This table specifies which tags set or clear which bits in permit.  

> If table **graph_permit** doesn't exist, a new table will be created with default values.  
> But if the table already exists, it will be used.  
> This allows you to create your own definition for filling the permit field.  

#### Table "graph_permit"
column     | type     | description
-----------|----------|-------------------------------------
key        | TEXT     | tag key
value      | TEXT     | tag value
set_bit    | INTEGER  | bitmask set bits
clear_bit  | INTEGER  | bitmask clear bits

The **vgraph** option allows you to visualize the graph for a given area.  
This option creates an HTML file containing maps with the graph data.  
Usage:  
`pbf2sqlite DATABASE vgraph LON1 LAT1 LON2 LAT2 HTMLFILE`  

<https://www.sqlite.org/lang_with.html#queries_against_a_graph>

