#!/usr/bin/env python
"""
Reads OpenStreetMap PBF data into a SQLite database
"""
import sys
import sqlite3
import math
import osmium


help = f'''
Reads OpenStreetMap PBF data into a SQLite database.

Usage:
{sys.argv[0]} DATABASE [OPTION ...]

Main options:
  read FILE     Reads FILE (.osm.pbf or .osm) into the database
  graph         Add graph table
  noindex       Do not create indexes (not recommended)
'''


class OSMHandler(osmium.SimpleHandler):
    """
    Define a class OSMHandler that subclasses osmium.SimpleHandler
    Define methods 'node', 'way' and 'relation'
    These methods will be called by Osmium's parser whenever it encounters
    a node, way or relation in the .osm.pbf file
    """
    def __init__(self, cur):
        osmium.SimpleHandler.__init__(self)
        #
        self.cur = cur
        #
        self.cur.execute('PRAGMA journal_mode = OFF')  # tuning database
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
        self.cur.execute('INSERT INTO nodes (node_id,lon,lat) VALUES (?,?,?)',
                         (n.id, n.location.lon, n.location.lat))
        for tag in n.tags:
            self.cur.execute('INSERT INTO node_tags (node_id,key,value) VALUES (?,?,?)',
                             (n.id, tag.k, tag.v))

    def way(self, w):
        """Insert way in database"""
        node_order = 1
        for node in w.nodes:
            self.cur.execute('INSERT INTO way_nodes (way_id,node_id,node_order) VALUES (?,?,?)',
                             (w.id, node.ref, node_order))
            node_order += 1
        for tag in w.tags:
            self.cur.execute('INSERT INTO way_tags (way_id,key,value) VALUES (?,?,?)',
                             (w.id, tag.k, tag.v))

    def relation(self, r):
        """Insert relation in database"""
        longtype = {'n': 'node', 'w': 'way', 'r': 'relation'}
        member_order = 1
        for member in r.members:
            self.cur.execute('INSERT INTO relation_members '
                             '(relation_id,ref,ref_id,role,member_order) VALUES (?,?,?,?,?)',
                             (r.id, longtype[member.type], member.ref, member.role, member_order))
            member_order += 1
        for tag in r.tags:
            self.cur.execute('INSERT INTO relation_tags (relation_id,key,value) VALUES (?,?,?)',
                             (r.id, tag.k, tag.v))


def add_index(cur):
    """Create the indexes in the database"""
    cur.executescript('''
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


def distance(lon1, lat1, lon2, lat2):
    """Calculates great circle distance between two coordinates in degrees"""
    # Avoid a math.acos ValueError if the two points are identical
    if lon1 == lon2 and lat1 == lat2:
        return 0
    lon1 = math.radians(lon1)   # Conversion degree to radians
    lat1 = math.radians(lat1)
    lon2 = math.radians(lon2)
    lat2 = math.radians(lat2)
    # Use earth radius Europe 6371 km (alternatively radius equator 6378 km)
    dist = math.acos(
                math.sin(lat1) * math.sin(lat2) +
                math.cos(lat1) * math.cos(lat2) * math.cos(lon2 - lon1)
            ) * 6371000
    return dist     # distance in meters


def add_graph(cur):
    """Create the graph table in the database"""
    cur.execute('BEGIN TRANSACTION')
    cur.execute('''
    CREATE TABLE graph (
     edge_id       INTEGER PRIMARY KEY,  -- edge ID
     start_node_id INTEGER,              -- edge start node ID
     end_node_id   INTEGER,              -- edge end node ID
     dist          INTEGER,              -- distance in meters
     way_id        INTEGER,              -- way ID
     nodes         INTEGER,              -- number of nodes
     permit        INTEGER DEFAULT 255   -- bit field access
    )
    ''')
    # Create a table with all nodes that are crossing points
    cur.executescript('''
    CREATE TEMP TABLE highway_nodes_crossing (
     node_id INTEGER PRIMARY KEY
    );
    INSERT INTO highway_nodes_crossing
    SELECT node_id FROM
    (
     SELECT wn.node_id
     FROM way_tags AS wt
     LEFT JOIN way_nodes AS wn ON wt.way_id=wn.way_id
     WHERE wt.key='highway'
    )
    GROUP BY node_id HAVING count(*)>1;
    ''')
    #
    prev_lon = 0
    prev_lat = 0
    prev_way_id = -1
    prev_node_id = -1
    edge_active = False
    start_node_id = -1
    dist = 0
    nodes = 1
    cur.execute('''
    SELECT
     wn.way_id,wn.node_id,
     ifnull(hnc.node_id,-1) AS node_id_crossing,
     n.lon,n.lat
    FROM way_tags AS wt
    LEFT JOIN way_nodes AS wn ON wt.way_id=wn.way_id
    LEFT JOIN highway_nodes_crossing AS hnc ON wn.node_id=hnc.node_id
    LEFT JOIN nodes AS n ON wn.node_id=n.node_id
    WHERE wt.key='highway'
    ORDER BY wn.way_id,wn.node_order
    ''')
    for (way_id, node_id, node_id_crossing, lon, lat) in cur.fetchall():
        # If a new way is active but there are still remnants of the previous way, create a new edge.
        if way_id != prev_way_id and edge_active:
            cur.execute('INSERT INTO graph (start_node_id,end_node_id,dist,way_id,nodes) VALUES (?,?,?,?,?)',
                        (start_node_id, prev_node_id, round(dist), prev_way_id, nodes))
            edge_active = False
        dist = dist + distance(prev_lon, prev_lat, lon, lat)
        nodes += 1
        edge_active = True
        # If way_id changes or crossing node is present then an edge begins or ends.
        if way_id != prev_way_id:
            start_node_id = node_id
            dist = 0
            nodes = 1
        if node_id_crossing > -1 and way_id == prev_way_id:
            if start_node_id != -1:
                cur.execute('INSERT INTO graph (start_node_id,end_node_id,dist,way_id,nodes) VALUES (?,?,?,?,?)',
                            (start_node_id, node_id, round(dist), way_id, nodes))
                edge_active = False
            start_node_id = node_id
            dist = 0
            nodes = 1
        prev_lon = lon
        prev_lat = lat
        prev_way_id = way_id
        prev_node_id = node_id
    if edge_active:
        cur.execute('INSERT INTO graph (start_node_id,end_node_id,dist,way_id,nodes) VALUES (?,?,?,?,?)',
                    (start_node_id, node_id, round(dist), way_id, nodes))
    cur.execute('CREATE INDEX graph__way_id ON graph (way_id)')
    cur.execute('COMMIT TRANSACTION')


def main():
    """Main function: entry point for execution"""
    db_name = ''
    osm_file_name = ''
    read = False
    graph = False
    index = True
    # Parse parameter
    if len(sys.argv) == 1:
        print(help)
        print('SQLite '+sqlite3.sqlite_version+' is used.\n')
        sys.exit(1)
    db_name = sys.argv[1]
    i = 2
    while i < len(sys.argv):
        if sys.argv[i] == 'read' and len(sys.argv) >= i+2:
            read = True
            osm_file_name = sys.argv[i+1]
            i += 1
        elif sys.argv[i] == 'graph':
            graph = True
        elif sys.argv[i] == 'node' and len(sys.argv) >= i+2:
            node_id = sys.argv[i+1]
            i += 1
        elif sys.argv[i] == 'way' and len(sys.argv) >= i+2:
            way_id = sys.argv[i+1]
            i += 1
        elif sys.argv[i] == 'relation' and len(sys.argv) >= i+2:
            relation_id = sys.argv[i+1]
            i += 1
        elif sys.argv[i] == 'noindex':
            index = False
        else:
            print("Invalid option:", sys.argv[i])
            sys.exit(1)
        i += 1
    # open database connection
    con = sqlite3.connect(db_name)
    cur = con.cursor()
    # Execute options
    if read:
        cur.execute('PRAGMA journal_mode = OFF')
        cur.execute('PRAGMA page_size = 65536')
        osm_handler = OSMHandler(cur)
        osm_handler.apply_file(osm_file_name)
        del osm_handler
        if index:
            add_index(cur)
        cur.execute('ANALYZE')
    if graph:
        add_graph(cur)
    # Close database connection
    con.commit()
    con.close()


if __name__ == '__main__':
    main()
