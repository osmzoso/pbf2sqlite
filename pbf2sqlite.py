#!/usr/bin/env python
"""
Reads OpenStreetMap PBF data into a SQLite database

Fedora:
sudo dnf install python3-osmium.x86_64

https://docs.osmcode.org/pyosmium/latest/index.html
https://wiki.openstreetmap.org/wiki/PBF_Format
"""
import sys
import sqlite3
import osmium

class OSMHandler(osmium.SimpleHandler):
    """
    Define a class OSMHandler that subclasses osmium.SimpleHandler
    Define methods 'node', 'way' and 'relation'
    These methods will be called by Osmium's parser whenever it encounters
    a node, way or relation in the .osm.pbf file
    """
    def __init__(self, cur):
        osmium.SimpleHandler.__init__(self)
        self.num_nodes = 0
        self.num_ways = 0
        self.num_relations = 0
        #
        self.cur = cur
        #
        self.cur.execute('PRAGMA journal_mode = OFF') # tuning database
        self.cur.execute('PRAGMA page_size = 65536')
        self.cur.execute('BEGIN TRANSACTION')
        self.cur.executescript('''
        CREATE TABLE nodes (
         node_id      INTEGER PRIMARY KEY,  -- node ID
         lon          REAL,                 -- longitude
         lat          REAL                  -- latitude
        );
        CREATE TABLE node_tags (
         node_id      INTEGER,              -- node ID
         key          TEXT,                 -- tag key
         value        TEXT                  -- tag value
        );
        CREATE TABLE way_nodes (
         way_id       INTEGER,              -- way ID
         node_id      INTEGER,              -- node ID
         node_order   INTEGER               -- node order
        );
        CREATE TABLE way_tags (
         way_id       INTEGER,              -- way ID
         key          TEXT,                 -- tag key
         value        TEXT                  -- tag value
        );
        CREATE TABLE relation_members (
         relation_id  INTEGER,              -- relation ID
         ref          TEXT,                 -- reference ('node','way','relation')
         ref_id       INTEGER,              -- node, way or relation ID
         role         TEXT,                 -- describes a particular feature
         member_order INTEGER               -- member order
        );
        CREATE TABLE relation_tags (
         relation_id  INTEGER,              -- relation ID
         key          TEXT,                 -- tag key
         value        TEXT                  -- tag value
        );
        ''')

    def __del__(self):
        self.cur.execute('COMMIT TRANSACTION')

    def node(self, n):
        """Insert node in database"""
        #print("Node:", n)
        #print("  Node ID:", n.id)
        #print("  Node Version:", n.version)
        #print("  Node Timestamp:", n.timestamp)
        #print("  Node User ID:", n.uid)
        #print("  Node Tags:", {tag.k: tag.v for tag in n.tags})
        #print("  Node Location (Lat, Lon):", (n.location.lat, n.location.lon))
        self.num_nodes += 1
        self.cur.execute('INSERT INTO nodes (node_id,lon,lat) VALUES (?,?,?)', (n.id, n.location.lon, n.location.lat))
        for tag in n.tags:
            self.cur.execute('INSERT INTO node_tags (node_id,key,value) VALUES (?,?,?)', (n.id, tag.k, tag.v))

    def way(self, w):
        """Insert way in database"""
        #rint("Way:", w)
        #print("  Way ID:", w.id)
        #print("  Way Version:", w.version)
        #print("  Way Timestamp:", w.timestamp)
        #print("  Way User ID:", w.uid)
        #print("  Way Tags:", {tag.k: tag.v for tag in w.tags})
        #print("  Way Nodes:", [node.ref for node in w.nodes])
        self.num_ways += 1
        node_order = 1
        for node in w.nodes:
            self.cur.execute('INSERT INTO way_nodes (way_id,node_id,node_order) VALUES (?,?,?)', (w.id, node.ref, node_order))
            node_order += 1
        for tag in w.tags:
            self.cur.execute('INSERT INTO way_tags (way_id,key,value) VALUES (?,?,?)', (w.id, tag.k, tag.v))

    def relation(self, r):
        """Insert relation in database"""
        #print("Relation:", r)
        #print("  Relation ID:", r.id)
        #print("  Relation Version:", r.version)
        #print("  Relation Timestamp:", r.timestamp)
        #print("  Relation User ID:", r.uid)
        #print("  Relation Tags:", {tag.k: tag.v for tag in r.tags})
        #print("  Relation Members:", [(member.type, member.ref, member.role) for member in r.members])
        self.num_relations += 1
        member_order = 1
        for member in r.members:
            self.cur.execute('INSERT INTO relation_members (relation_id,ref,ref_id,role,member_order) VALUES (?,?,?,?,?)', (r.id, member.type, member.ref, member.role, member_order))
            member_order += 1
        for tag in r.tags:
            self.cur.execute('INSERT INTO relation_tags (relation_id,key,value) VALUES (?,?,?)', (r.id, tag.k, tag.v))


def add_index():
    """Create the indexes in the database"""
    db.executescript('''
    CREATE INDEX node_tags__node_id            ON node_tags (node_id);
    CREATE INDEX node_tags__key                ON node_tags (key);
    CREATE INDEX way_tags__way_id              ON way_tags (way_id);
    CREATE INDEX way_tags__key                 ON way_tags (key);
    CREATE INDEX way_nodes__way_id             ON way_nodes (way_id, node_order);
    CREATE INDEX way_nodes__node_id            ON way_nodes (node_id);
    CREATE INDEX relation_members__relation_id ON relation_members (relation_id, member_order);
    CREATE INDEX relation_members__ref_id      ON relation_members (ref_id);
    CREATE INDEX relation_tags__relation_id    ON relation_tags (relation_id);
    CREATE INDEX relation_tags__key            ON relation_tags (key);
    ''')


def main():
    """entry point"""
    if len(sys.argv) != 3:
        print('Read .osm.pbf file\n'
              'Usage:\n'
              f'{sys.argv[0]} DATABASE OSM_PBF_FILE\n')
        sys.exit(1)
    # connect to the database
    con = sqlite3.connect(sys.argv[1])
    cur = con.cursor()   # new database cursor
    #
    osm_handler = OSMHandler(cur)
    osm_handler.apply_file(sys.argv[2])
    print(f'Number of nodes: {osm_handler.num_nodes}')
    print(f'Number of ways: {osm_handler.num_ways}')
    print(f'Number of relations: {osm_handler.num_relations}')
    del osm_handler
    #
    add_index(cur)
    con.commit()
    con.close()


if __name__ == '__main__':
    main()
