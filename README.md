# pbf2sqlite

A simple command line tool for reading OpenStreetMap .osm.pbf files into a SQLite database.

```
Usage:
pbf2sqlite.py DATABASE OSM_PBF_FILE
```

The Python module **osmium** is required.  
Fedora: `sudo dnf install python3-osmium`

<https://docs.osmcode.org/pyosmium/latest/index.html>  
<https://wiki.openstreetmap.org/wiki/PBF_Format>  


## Tables

The following tables and indexes are created in the database:

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

