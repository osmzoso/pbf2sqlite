#!/usr/bin/env python
"""
Reads OpenStreetMap PBF data into a SQLite database
"""
import sys
import sqlite3
import math
import osmium


def show_help():
    """Show built-in help"""
    print(f'{sys.argv[0]} 0.2\n'
          '\n'
          'Reads OpenStreetMap PBF data into a SQLite database.\n'
          '\n'
          'Usage:\n'
          f'{sys.argv[0]} DATABASE [OPTION ...]\n'
          '\n'
          'Options:\n'
          '  read FILE     Reads FILE into the database\n'
          '                (.osm.pbf or .osm)\n'
          '  rtree         Add R*Tree indexes\n'
          '  addr          Add address tables\n'
          '  graph         Add graph table\n'
          '\n'
          'Other options:\n'
          '  node ID       Show node data\n'
          '  way ID        Show way data\n'
          '  relation ID   Show relation data\n'
          )
    print('(SQLite '+sqlite3.sqlite_version+' is used)\n')


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
        # print("Node:", n)
        # print("  Node ID:", n.id)
        # print("  Node Version:", n.version)
        # print("  Node Timestamp:", n.timestamp)
        # print("  Node User ID:", n.uid)
        # print("  Node Tags:", {tag.k: tag.v for tag in n.tags})
        # print("  Node Location (Lat, Lon):", (n.location.lat, n.location.lon))
        self.cur.execute('INSERT INTO nodes (node_id,lon,lat) VALUES (?,?,?)',
                         (n.id, n.location.lon, n.location.lat))
        for tag in n.tags:
            self.cur.execute('INSERT INTO node_tags (node_id,key,value) VALUES (?,?,?)',
                             (n.id, tag.k, tag.v))

    def way(self, w):
        """Insert way in database"""
        # print("Way:", w)
        # print("  Way ID:", w.id)
        # print("  Way Version:", w.version)
        # print("  Way Timestamp:", w.timestamp)
        # print("  Way User ID:", w.uid)
        # print("  Way Tags:", {tag.k: tag.v for tag in w.tags})
        # print("  Way Nodes:", [node.ref for node in w.nodes])
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
        # print("Relation:", r)
        # print("  Relation ID:", r.id)
        # print("  Relation Version:", r.version)
        # print("  Relation Timestamp:", r.timestamp)
        # print("  Relation User ID:", r.uid)
        # print("  Relation Tags:", {tag.k: tag.v for tag in r.tags})
        # print("  Relation Members:",
        #          [(member.type, member.ref, member.role) for member in r.members])
        # osmium member.type is shortened ('n', 'w' or 'r'), hence this conversion list
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


def add_rtree(cur):
    """Create the R*Tree indexes in the database"""
    cur.executescript('''
    CREATE VIRTUAL TABLE rtree_way USING rtree(way_id, min_lat, max_lat, min_lon, max_lon);
    INSERT INTO rtree_way (way_id, min_lat, max_lat, min_lon, max_lon)
    SELECT way_nodes.way_id,min(nodes.lat),max(nodes.lat),min(nodes.lon),max(nodes.lon)
    FROM way_nodes
    LEFT JOIN nodes ON way_nodes.node_id=nodes.node_id
    GROUP BY way_nodes.way_id;
    CREATE VIRTUAL TABLE rtree_node USING rtree(node_id, min_lat, max_lat, min_lon, max_lon);
    INSERT INTO rtree_node (node_id, min_lat, max_lat, min_lon, max_lon)
    SELECT DISTINCT nodes.node_id,nodes.lat,nodes.lat,nodes.lon,nodes.lon
    FROM nodes
    LEFT JOIN node_tags ON nodes.node_id=node_tags.node_id
    WHERE node_tags.node_id IS NOT NULL;
    ''')


def add_addr(cur):
    """Create the address tables in the database"""
    cur.executescript('''
    BEGIN TRANSACTION;
    /*
    ** Create address tables
    */
    CREATE TABLE addr_street (
     street_id   INTEGER PRIMARY KEY, -- street ID
     postcode    TEXT,                -- postcode
     city        TEXT,                -- city name
     street      TEXT,                -- street name
     min_lon     REAL,                -- boundingbox street min longitude
     min_lat     REAL,                -- boundingbox street min latitude
     max_lon     REAL,                -- boundingbox street max longitude
     max_lat     REAL                 -- boundingbox street max latitude
    );
    CREATE TABLE addr_housenumber (
     housenumber_id INTEGER PRIMARY KEY, -- housenumber ID
     street_id      INTEGER,             -- street ID
     housenumber    TEXT,                -- housenumber
     lon            REAL,                -- longitude
     lat            REAL,                -- latitude
     way_id         INTEGER,             -- way ID
     node_id        INTEGER              -- node ID
    );
    CREATE VIEW addr_view AS
    SELECT s.street_id,s.postcode,s.city,s.street,h.housenumber,h.lon,h.lat,h.way_id,h.node_id
    FROM addr_street AS s
    LEFT JOIN addr_housenumber AS h ON s.street_id=h.street_id;
    /*
    ** 1. Determine address data from way tags
    */
    CREATE TEMP TABLE tmp_addr_way (
     way_id      INTEGER PRIMARY KEY,
     postcode    TEXT,
     city        TEXT,
     street      TEXT,
     housenumber TEXT
    );
    INSERT INTO tmp_addr_way
     SELECT way_id,value AS postcode,'','',''
     FROM way_tags WHERE key='addr:postcode'
     ON CONFLICT(way_id) DO UPDATE SET postcode=excluded.postcode;
    INSERT INTO tmp_addr_way
     SELECT way_id,'',value AS city,'',''
     FROM way_tags WHERE key='addr:city'
     ON CONFLICT(way_id) DO UPDATE SET city=excluded.city;
    INSERT INTO tmp_addr_way
     SELECT way_id,'','',value AS street,''
     FROM way_tags WHERE key='addr:street'
     ON CONFLICT(way_id) DO UPDATE SET street=excluded.street;
    INSERT INTO tmp_addr_way
     SELECT way_id,'','','',value AS housenumber
     FROM way_tags WHERE key='addr:housenumber'
     ON CONFLICT(way_id) DO UPDATE SET housenumber=excluded.housenumber;
    /*
    ** 2. Calculate coordinates of address data from way tags
    */
    CREATE TEMP TABLE tmp_addr_way_coordinates AS
    SELECT way.way_id AS way_id,round(avg(n.lon),7) AS lon,round(avg(n.lat),7) AS lat
    FROM tmp_addr_way AS way
    LEFT JOIN way_nodes AS wn ON way.way_id=wn.way_id
    LEFT JOIN nodes     AS n  ON wn.node_id=n.node_id
    GROUP BY way.way_id;
    CREATE INDEX tmp_addr_way_coordinates_way_id ON tmp_addr_way_coordinates (way_id);
    /*
    ** 3. Determine address data from node tags
    */
    CREATE TEMP TABLE tmp_addr_node (
     node_id     INTEGER PRIMARY KEY,
     postcode    TEXT,
     city        TEXT,
     street      TEXT,
     housenumber TEXT
    );
    INSERT INTO tmp_addr_node
     SELECT node_id,value AS postcode,'','',''
     FROM node_tags WHERE key='addr:postcode'
     ON CONFLICT(node_id) DO UPDATE SET postcode=excluded.postcode;
    INSERT INTO tmp_addr_node
     SELECT node_id,'',value AS city,'',''
     FROM node_tags WHERE key='addr:city'
     ON CONFLICT(node_id) DO UPDATE SET city=excluded.city;
    INSERT INTO tmp_addr_node
     SELECT node_id,'','',value AS street,''
     FROM node_tags WHERE key='addr:street'
     ON CONFLICT(node_id) DO UPDATE SET street=excluded.street;
    INSERT INTO tmp_addr_node
     SELECT node_id,'','','',value AS housenumber
     FROM node_tags WHERE key='addr:housenumber'
     ON CONFLICT(node_id) DO UPDATE SET housenumber=excluded.housenumber;
    /*
    ** 4. Create temporary overall table with all addresses
    */
    CREATE TEMP TABLE tmp_addr (
     addr_id     INTEGER PRIMARY KEY,
     way_id      INTEGER,
     node_id     INTEGER,
     postcode    TEXT,
     city        TEXT,
     street      TEXT,
     housenumber TEXT,
     lon         REAL,
     lat         REAL
    );
    INSERT INTO tmp_addr (way_id,node_id,postcode,city,street,housenumber,lon,lat)
     SELECT w.way_id,-1 AS node_id,w.postcode,w.city,w.street,w.housenumber,c.lon,c.lat
     FROM tmp_addr_way AS w
     LEFT JOIN tmp_addr_way_coordinates AS c ON w.way_id=c.way_id
    UNION ALL
     SELECT -1 AS way_id,n.node_id,n.postcode,n.city,n.street,n.housenumber,c.lon,c.lat
     FROM tmp_addr_node AS n
     LEFT JOIN nodes AS c ON n.node_id=c.node_id
    ORDER BY postcode,city,street,housenumber;
    /*
    ** 5. Fill tables 'addr_street' and 'addr_housenumber'
    */
    INSERT INTO addr_street (postcode,city,street,min_lon,min_lat,max_lon,max_lat)
     SELECT postcode,city,street,min(lon),min(lat),max(lon),max(lat)
     FROM tmp_addr
     GROUP BY postcode,city,street;
    CREATE INDEX addr_street__postcode_city_street ON addr_street (postcode,city,street);
    INSERT INTO addr_housenumber (street_id,housenumber,lon,lat,way_id,node_id)
     SELECT s.street_id,a.housenumber,a.lon,a.lat,a.way_id,a.node_id
     FROM tmp_addr AS a
     LEFT JOIN addr_street AS s ON a.postcode=s.postcode AND a.city=s.city AND a.street=s.street;
    CREATE INDEX addr_housenumber__street_id ON addr_housenumber (street_id);
    /*
    ** 6. Delete temporary tables
    */
    DROP TABLE tmp_addr_way;
    DROP TABLE tmp_addr_way_coordinates;
    DROP TABLE tmp_addr_node;
    DROP TABLE tmp_addr;
    COMMIT TRANSACTION;
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


def fill_graph_permit(cur):
    """Fill the field 'permit_v2' in table 'graph'"""
    cur.execute('SELECT DISTINCT way_id FROM graph')
    for (way_id,) in cur.fetchall():
        mask_set = 0b00000000
        mask_clear = 0b11111111
        cur.execute('''
        SELECT gp.set_bit,gp.clear_bit
        FROM way_tags AS wt
        JOIN graph_permit AS gp ON wt.key=gp.key AND wt.value=gp.value
        WHERE wt.way_id=?
        ''', (way_id,))
        for (set_bit, clear_bit) in cur.fetchall():
            mask_set = mask_set | set_bit        # bitwise or
            mask_clear = mask_clear & clear_bit  # bitwise and
        permit = 0b00000000
        permit = permit | mask_set
        permit = permit & mask_clear
        cur.execute('UPDATE graph SET permit=? WHERE way_id=?',
                    (permit, way_id))


def create_table_graph_permit(cur):
    """Create table graph_permit"""
    cur.executescript('''
    /*
    **
    ** Bits in the bitfield "permit":
    **  Bit 0: foot
    **  Bit 1: bike
    **  Bit 2: car
    **  Bit 3: paved
    **  Bit 4: bike_oneway
    **  Bit 5: car_oneway
    **  Bit 6: (not used)
    **  Bit 7: (not used)
    **
    */
    BEGIN TRANSACTION;
    CREATE TABLE graph_permit(
      key     TEXT,
      value   TEXT,
      set_bit INTEGER,
      clear_bit INTEGER
    );
    /*
    ** Tags to set permit bits
    */
    /* Tags foot -> set bits 00000001 (dec 1) */
    INSERT INTO graph_permit VALUES ('highway','pedestrian',            1, 255);
    INSERT INTO graph_permit VALUES ('highway','track',                 1, 255);
    INSERT INTO graph_permit VALUES ('highway','footway',               1, 255);
    INSERT INTO graph_permit VALUES ('highway','steps',                 1, 255);
    INSERT INTO graph_permit VALUES ('highway','path',                  1, 255);
    INSERT INTO graph_permit VALUES ('highway','construction',          1, 255);
    INSERT INTO graph_permit VALUES ('foot','yes',                      1, 255);
    INSERT INTO graph_permit VALUES ('foot','designated',               1, 255);
    INSERT INTO graph_permit VALUES ('sidewalk','both',                 1, 255);
    INSERT INTO graph_permit VALUES ('sidewalk:both','yes',             1, 255);
    INSERT INTO graph_permit VALUES ('sidewalk','right',                1, 255);
    INSERT INTO graph_permit VALUES ('sidewalk:right','yes',            1, 255);
    INSERT INTO graph_permit VALUES ('sidewalk','left',                 1, 255);
    INSERT INTO graph_permit VALUES ('sidewalk:left','yes',             1, 255);
    INSERT INTO graph_permit VALUES ('sidewalk','yes',                  1, 255);
    /* Tags foot & bike -> set bits 00000011 (dec 3) */
    INSERT INTO graph_permit VALUES ('highway','residential',           3, 255);
    INSERT INTO graph_permit VALUES ('highway','living_street',         3, 255);
    INSERT INTO graph_permit VALUES ('highway','service',               3, 255);
    INSERT INTO graph_permit VALUES ('highway','track',                 3, 255);
    INSERT INTO graph_permit VALUES ('highway','unclassified',          3, 255);
    /* Tags bike -> set bits 00000010 (dec 2) */
    INSERT INTO graph_permit VALUES ('highway','cycleway',              2, 255);
    INSERT INTO graph_permit VALUES ('bicycle','yes',                   2, 255);
    INSERT INTO graph_permit VALUES ('bicycle','designated',            2, 255);
    /* Tags bike & car -> set bits 00000110 (dec 6) */
    INSERT INTO graph_permit VALUES ('highway','primary',               6, 255);
    INSERT INTO graph_permit VALUES ('highway','primary_link',          6, 255);
    INSERT INTO graph_permit VALUES ('highway','secondary',             6, 255);
    INSERT INTO graph_permit VALUES ('highway','secondary_link',        6, 255);
    INSERT INTO graph_permit VALUES ('highway','tertiary',              6, 255);
    INSERT INTO graph_permit VALUES ('highway','tertiary_link',         6, 255);
    INSERT INTO graph_permit VALUES ('highway','unclassified',          6, 255);
    INSERT INTO graph_permit VALUES ('highway','residential',           6, 255);
    /* Tags car -> set bits 00000100 (dec 4) */
    INSERT INTO graph_permit VALUES ('highway','motorway',              4, 255);
    INSERT INTO graph_permit VALUES ('highway','motorway_link',         4, 255);
    INSERT INTO graph_permit VALUES ('highway','trunk',                 4, 255);
    INSERT INTO graph_permit VALUES ('highway','trunk_link',            4, 255);
    /* Tags paved -> set bits 00001000 (dec 8) */
    INSERT INTO graph_permit VALUES ('surface','asphalt',               8, 255);
    INSERT INTO graph_permit VALUES ('surface','sett',                  8, 255);
    INSERT INTO graph_permit VALUES ('surface','paving_stones',         8, 255);
    /* Tags oneway -> set bits 00110000 (dec 48) */
    INSERT INTO graph_permit VALUES ('oneway','yes',                   48, 255);
    /*
    ** Tags to clear permit bits
    */
    /* Tags no foot -> clear bits 11111110 (dec 254) */
    INSERT INTO graph_permit VALUES ('sidewalk','separate',             0, 254);
    INSERT INTO graph_permit VALUES ('foot','use_sidepath',             0, 254);
    INSERT INTO graph_permit VALUES ('access','no',                     0, 254);
    /* Tags no bike -> clear bits 11111101 (dec 253) */
    INSERT INTO graph_permit VALUES ('cycleway','separate',             0, 253);
    INSERT INTO graph_permit VALUES ('cycleway:both','separate',        0, 253);
    INSERT INTO graph_permit VALUES ('cycleway:right','separate',       0, 253);
    INSERT INTO graph_permit VALUES ('cycleway:left','separate',        0, 253);
    INSERT INTO graph_permit VALUES ('bicycle','use_sidepath',          0, 253);
    INSERT INTO graph_permit VALUES ('access','no',                     0, 253);
    /* Tags no oneway -> clear bits 11101111 (dec 239) */
    INSERT INTO graph_permit VALUES ('oneway:bicycle','no',             0, 239);
    /* create index */
    CREATE INDEX graph_permit__key_value ON graph_permit (key, value);
    COMMIT TRANSACTION;
    ''')


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
     permit        INTEGER DEFAULT 255   -- bit field access
    )
    ''')
    # Create a table with all nodes that are crossing points
    cur.execute('''
    CREATE TEMP TABLE highway_nodes_crossing
    (
     node_id INTEGER PRIMARY KEY
    )
    ''')
    cur.execute('''
    INSERT INTO highway_nodes_crossing
    SELECT node_id FROM
    (
     SELECT wn.node_id
     FROM way_tags AS wt
     LEFT JOIN way_nodes AS wn ON wt.way_id=wn.way_id
     WHERE wt.key='highway'
    )
    GROUP BY node_id HAVING count(*)>1
    ''')
    #
    prev_lon = 0
    prev_lat = 0
    prev_way_id = -1
    prev_node_id = -1
    edge_active = False
    start_node_id = -1
    dist = 0
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
            cur.execute('INSERT INTO graph (start_node_id,end_node_id,dist,way_id) VALUES (?,?,?,?)',
                       (start_node_id, prev_node_id, round(dist), prev_way_id))
            edge_active = False
        dist = dist + distance(prev_lon, prev_lat, lon, lat)
        edge_active = True
        # If way_id changes or crossing node is present then an edge begins or ends.
        if way_id != prev_way_id:
            start_node_id = node_id
            dist = 0
        if node_id_crossing > -1 and way_id == prev_way_id:
            if start_node_id != -1:
                cur.execute('INSERT INTO graph (start_node_id,end_node_id,dist,way_id) VALUES (?,?,?,?)',
                           (start_node_id, node_id, round(dist), way_id))
                edge_active = False
            start_node_id = node_id
            dist = 0
        prev_lon = lon
        prev_lat = lat
        prev_way_id = way_id
        prev_node_id = node_id
    if edge_active:
        cur.execute('INSERT INTO graph (start_node_id,end_node_id,dist,way_id) VALUES (?,?,?,?)',
                   (start_node_id, node_id, round(dist), way_id))
    cur.execute('CREATE INDEX graph__way_id ON graph (way_id)')
    cur.execute('COMMIT TRANSACTION')
    #
    create_table_graph_permit(cur)
    fill_graph_permit(cur)


def show_node(cur, node_id):
    """
    Displays OSM data of a node
    """
    # Location
    cur.execute('SELECT lon,lat FROM nodes WHERE node_id=?', (node_id,))
    for (lon, lat) in cur.fetchall():
        print(f'node {node_id} location {lon} {lat}')
    # Tags
    cur.execute('SELECT key,value FROM node_tags WHERE node_id=?', (node_id,))
    for (key, value) in cur.fetchall():
        print(f'node {node_id} tag "{key}":"{value}"')
    # Part of relation
    cur.execute("""
    SELECT relation_id,role
    FROM relation_members
    WHERE ref_id=? AND ref='node'""", (node_id,))
    for (relation_id, role) in cur.fetchall():
        print(f'node {node_id} part_of_relation {relation_id} {role}')


def show_way(cur, way_id):
    """
    Displays OSM data of a way
    """
    # Tags
    cur.execute('SELECT key,value FROM way_tags WHERE way_id=?', (way_id,))
    for (key, value) in cur.fetchall():
        print(f'way {way_id} tag "{key}":"{value}"')
    # Part of relation
    cur.execute("""
    SELECT relation_id,role
    FROM relation_members
    WHERE ref_id=? AND ref='way'""", (way_id,))
    for (relation_id, role) in cur.fetchall():
        print(f'way {way_id} part_of_relation {relation_id:15d} {role}')
    # Nodes
    cur.execute('''
    SELECT wn.node_id,n.lat,n.lon
    FROM way_nodes AS wn
    LEFT JOIN nodes AS n ON wn.node_id=n.node_id
    WHERE wn.way_id=?
    ORDER BY wn.node_order''', (way_id,))
    for (node_id, lat, lon) in cur.fetchall():
        print(f'way {way_id} node {node_id:15d} {lat:12.7f} {lon:12.7f}')


def show_relation(cur, relation_id):
    """
    Displays OSM data of a relation
    """
    # Tags
    cur.execute('SELECT key,value FROM relation_tags WHERE relation_id=?',
                (relation_id,))
    for (key, value) in cur.fetchall():
        print(f'relation {relation_id} tag "{key}":"{value}"')
    # Part of relation
    cur.execute("""
    SELECT relation_id,role
    FROM relation_members
    WHERE ref_id=? AND ref='relation'""", (relation_id,))
    for (part_relation_id, role) in cur.fetchall():
        print(f'relation {relation_id} part_of_relation '
              f'{part_relation_id} {role}')
    # Members
    cur.execute('''
    SELECT ref,ref_id,role
    FROM relation_members
    WHERE relation_id=?
    ORDER BY member_order''', (relation_id,))
    for (ref, ref_id, role) in cur.fetchall():
        print(f'relation {relation_id} member {ref} {ref_id} {role}')


def main():
    """Main function: entry point for execution"""
    if len(sys.argv) == 1:
        show_help()
        sys.exit(1)
    con = sqlite3.connect(sys.argv[1])  # open database connection
    cur = con.cursor()                  # new database cursor
    i = 2
    while i < len(sys.argv):
        if sys.argv[i] == 'read':
            cur.execute('PRAGMA journal_mode = OFF')
            cur.execute('PRAGMA page_size = 65536')
            osm_handler = OSMHandler(cur)
            osm_handler.apply_file(sys.argv[i+1])
            del osm_handler
            add_index(cur)
            cur.execute('ANALYZE')
            i += 1
        elif sys.argv[i] == 'rtree':
            add_rtree(cur)
        elif sys.argv[i] == 'addr':
            add_addr(cur)
        elif sys.argv[i] == 'graph':
            add_graph(cur)
        elif sys.argv[i] == 'node' and len(sys.argv) >= i+2:
            show_node(cur, sys.argv[i+1])
            i += 1
        elif sys.argv[i] == 'way' and len(sys.argv) >= i+2:
            show_way(cur, sys.argv[i+1])
            i += 1
        elif sys.argv[i] == 'relation' and len(sys.argv) >= i+2:
            show_relation(cur, sys.argv[i+1])
            i += 1
        else:
            print(f"{sys.argv[0]} - Parameter error: '"+sys.argv[i]+"'?")
        i += 1
    con.commit()                        # commit
    con.close()                         # close database connection


if __name__ == '__main__':
    main()
