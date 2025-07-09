/*
** pbf2sqlite
*/
#include "pbf2sqlite.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sqlite3.h>
#include <readosm.h>

/* Mathematical Constants */
#ifndef M_PI
# define M_PI   3.141592653589793238462643383279502884
#endif

void show_help() {
  fprintf(stderr,
  "pbf2sqlite 0.2 ALPHA\n"
  "\n"
  "Reads an OpenStreetMap .osm.pbf file into an SQLite database.\n"
  "\n"
  "Usage:\npbf2sqlite DATABASE [OPTION ...]\n"
  "\n"
  "Options:\n"
  "  read FILE     Reads FILE into the database\n"
  "                (.osm.pbf or .osm)\n"
  "  rtree         Add R*Tree indexes\n"
  "  addr          Add address tables\n"
  "  graph         Add graph table\n"
  "\n"
  "Other options:\n"
  "  node ID       Show node data\n"
  "  way ID        Show way data\n"
  "  relation ID   Show relation data\n"
  "\n"
  );
  fprintf(stderr, "(SQLite %s is used)\n\n", sqlite3_libversion());
}

/*
** Public variables
*/
sqlite3 *db;         /* SQLite Database connection */
int rc;              /* SQLite Result code */
sqlite3_stmt *stmt_insert_nodes, *stmt_insert_node_tags, *stmt_insert_way_nodes,
             *stmt_insert_way_tags, *stmt_insert_relation_members, *stmt_insert_relation_tags;

/*
** Shows last result code and then aborts the program
*/
void abort_db_error() {
  fprintf(stderr, "abort pbf2sqlite - (%i) %s - %s\n", rc, sqlite3_errstr(rc), sqlite3_errmsg(db));
  sqlite3_close(db);
  exit(EXIT_FAILURE);
}

/*
** create tables, indexes and prepared insert statements
*/
void add_tables() {
  rc = sqlite3_exec(
         db,
         " CREATE TABLE nodes (\n"
         "  node_id      INTEGER PRIMARY KEY,  -- node ID\n"
         "  lon          REAL,                 -- longitude\n"
         "  lat          REAL                  -- latitude\n"
         " );\n"

         " CREATE TABLE node_tags (\n"
         "  node_id      INTEGER,              -- node ID\n"
         "  key          TEXT,                 -- tag key\n"
         "  value        TEXT                  -- tag value\n"
         " );\n"

         " CREATE TABLE way_nodes (\n"
         "  way_id       INTEGER,              -- way ID\n"
         "  node_id      INTEGER,              -- node ID\n"
         "  node_order   INTEGER               -- node order\n"
         " );\n"

         " CREATE TABLE way_tags (\n"
         "  way_id       INTEGER,              -- way ID\n"
         "  key          TEXT,                 -- tag key\n"
         "  value        TEXT                  -- tag value\n"
         " );\n"

         " CREATE TABLE relation_members (\n"
         "  relation_id  INTEGER,              -- relation ID\n"
         "  ref          TEXT,                 -- reference ('node','way','relation')\n"
         "  ref_id       INTEGER,              -- node, way or relation ID\n"
         "  role         TEXT,                 -- describes a particular feature\n"
         "  member_order INTEGER               -- member order\n"
         " );\n"

         " CREATE TABLE relation_tags (\n"
         "  relation_id  INTEGER,              -- relation ID\n"
         "  key          TEXT,                 -- tag key\n"
         "  value        TEXT                  -- tag value\n"
         " );\n",
         NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
}

void create_prep_stmt() {
  rc = sqlite3_prepare_v2(
         db,
         "INSERT INTO nodes (node_id,lat,lon) VALUES (?1,?2,?3)",
         -1, &stmt_insert_nodes, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_prepare_v2(
         db,
         "INSERT INTO node_tags (node_id,key,value) VALUES (?1,?2,?3)",
         -1, &stmt_insert_node_tags, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_prepare_v2(
         db,
         "INSERT INTO way_nodes (way_id,node_id,node_order) VALUES (?1,?2,?3)",
         -1, &stmt_insert_way_nodes, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_prepare_v2(
         db,
         "INSERT INTO way_tags (way_id,key,value) VALUES (?1,?2,?3)",
         -1, &stmt_insert_way_tags, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_prepare_v2(
         db,
         "INSERT INTO relation_members (relation_id,ref,ref_id,role,member_order) VALUES (?1,?2,?3,?4,?5)",
         -1, &stmt_insert_relation_members, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_prepare_v2(
         db,
         "INSERT INTO relation_tags (relation_id,key,value) VALUES (?1,?2,?3)",
         -1, &stmt_insert_relation_tags, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
}

void destroy_prep_stmt() {
  sqlite3_finalize(stmt_insert_nodes);
  sqlite3_finalize(stmt_insert_node_tags);
  sqlite3_finalize(stmt_insert_way_nodes);
  sqlite3_finalize(stmt_insert_way_tags);
  sqlite3_finalize(stmt_insert_relation_members);
  sqlite3_finalize(stmt_insert_relation_tags);
}

void add_index() {
  rc = sqlite3_exec(
    db,
    " CREATE INDEX node_tags__node_id            ON node_tags (node_id);"
    " CREATE INDEX node_tags__key                ON node_tags (key);"
    " CREATE INDEX way_tags__way_id              ON way_tags (way_id);"
    " CREATE INDEX way_tags__key                 ON way_tags (key);"
    " CREATE INDEX way_nodes__way_id             ON way_nodes (way_id, node_order);"
    " CREATE INDEX way_nodes__node_id            ON way_nodes (node_id);"
    " CREATE INDEX relation_members__relation_id ON relation_members (relation_id, member_order);"
    " CREATE INDEX relation_members__ref_id      ON relation_members (ref_id);"
    " CREATE INDEX relation_tags__relation_id    ON relation_tags (relation_id);"
    " CREATE INDEX relation_tags__key            ON relation_tags (key);",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
}

void add_rtree() {
  rc = sqlite3_exec(
    db,
    /*
    ** Create R*Tree index 'rtree_way'
    */
    " CREATE VIRTUAL TABLE rtree_way USING rtree(way_id, min_lat, max_lat, min_lon, max_lon);"
    " INSERT INTO rtree_way (way_id, min_lat, max_lat, min_lon, max_lon)"
    " SELECT way_nodes.way_id,min(nodes.lat),max(nodes.lat),min(nodes.lon),max(nodes.lon)"
    " FROM way_nodes"
    " LEFT JOIN nodes ON way_nodes.node_id=nodes.node_id"
    " GROUP BY way_nodes.way_id;"
    " CREATE VIRTUAL TABLE rtree_node USING rtree(node_id, min_lat, max_lat, min_lon, max_lon);"
    " INSERT INTO rtree_node (node_id, min_lat, max_lat, min_lon, max_lon)"
    " SELECT DISTINCT nodes.node_id,nodes.lat,nodes.lat,nodes.lon,nodes.lon"
    " FROM nodes"
    " LEFT JOIN node_tags ON nodes.node_id=node_tags.node_id"
    " WHERE node_tags.node_id IS NOT NULL;",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
}

void add_addr() {
  rc = sqlite3_exec(
    db,
    " BEGIN TRANSACTION;"
    " /*"
    " ** Create address tables"
    " */"
    " CREATE TABLE addr_street (\n"
    "  street_id   INTEGER PRIMARY KEY, -- street ID\n"
    "  postcode    TEXT,                -- postcode\n"
    "  city        TEXT,                -- city name\n"
    "  street      TEXT,                -- street name\n"
    "  min_lon     REAL,                -- boundingbox street min longitude\n"
    "  min_lat     REAL,                -- boundingbox street min latitude\n"
    "  max_lon     REAL,                -- boundingbox street max longitude\n"
    "  max_lat     REAL                 -- boundingbox street max latitude\n"
    " );\n"
    " CREATE TABLE addr_housenumber (\n"
    "  housenumber_id INTEGER PRIMARY KEY, -- housenumber ID\n"
    "  street_id      INTEGER,             -- street ID\n"
    "  housenumber    TEXT,                -- housenumber\n"
    "  lon            REAL,                -- longitude\n"
    "  lat            REAL,                -- latitude\n"
    "  way_id         INTEGER,             -- way ID\n"
    "  node_id        INTEGER              -- node ID\n"
    " );\n"
    " CREATE VIEW addr_view AS"
    " SELECT s.street_id,s.postcode,s.city,s.street,h.housenumber,h.lon,h.lat,h.way_id,h.node_id"
    " FROM addr_street AS s"
    " LEFT JOIN addr_housenumber AS h ON s.street_id=h.street_id;"
    " /*"
    " ** 1. Determine address data from way tags"
    " */"
    " CREATE TEMP TABLE tmp_addr_way (\n"
    "  way_id      INTEGER PRIMARY KEY,"
    "  postcode    TEXT,"
    "  city        TEXT,"
    "  street      TEXT,"
    "  housenumber TEXT"
    " );\n"
    " INSERT INTO tmp_addr_way"
    "  SELECT way_id,value AS postcode,'','',''"
    "  FROM way_tags WHERE key='addr:postcode'"
    "  ON CONFLICT(way_id) DO UPDATE SET postcode=excluded.postcode;"
    " INSERT INTO tmp_addr_way"
    "  SELECT way_id,'',value AS city,'',''"
    "  FROM way_tags WHERE key='addr:city'"
    "  ON CONFLICT(way_id) DO UPDATE SET city=excluded.city;"
    " INSERT INTO tmp_addr_way"
    "  SELECT way_id,'','',value AS street,''"
    "  FROM way_tags WHERE key='addr:street'"
    "  ON CONFLICT(way_id) DO UPDATE SET street=excluded.street;"
    " INSERT INTO tmp_addr_way"
    "  SELECT way_id,'','','',value AS housenumber"
    "  FROM way_tags WHERE key='addr:housenumber'"
    "  ON CONFLICT(way_id) DO UPDATE SET housenumber=excluded.housenumber;"
    " /*"
    " ** 2. Calculate coordinates of address data from way tags"
    " */"
    " CREATE TEMP TABLE tmp_addr_way_coordinates AS"
    " SELECT way.way_id AS way_id,round(avg(n.lon),7) AS lon,round(avg(n.lat),7) AS lat"
    " FROM tmp_addr_way AS way"
    " LEFT JOIN way_nodes AS wn ON way.way_id=wn.way_id"
    " LEFT JOIN nodes     AS n  ON wn.node_id=n.node_id"
    " GROUP BY way.way_id;"
    " CREATE INDEX tmp_addr_way_coordinates_way_id ON tmp_addr_way_coordinates (way_id);\n"
    " /*"
    " ** 3. Determine address data from node tags"
    " */"
    " CREATE TEMP TABLE tmp_addr_node (\n"
    "  node_id     INTEGER PRIMARY KEY,"
    "  postcode    TEXT,"
    "  city        TEXT,"
    "  street      TEXT,"
    "  housenumber TEXT"
    " );\n"
    " INSERT INTO tmp_addr_node"
    "  SELECT node_id,value AS postcode,'','',''"
    "  FROM node_tags WHERE key='addr:postcode'"
    "  ON CONFLICT(node_id) DO UPDATE SET postcode=excluded.postcode;"
    " INSERT INTO tmp_addr_node"
    "  SELECT node_id,'',value AS city,'',''"
    "  FROM node_tags WHERE key='addr:city'"
    "  ON CONFLICT(node_id) DO UPDATE SET city=excluded.city;"
    " INSERT INTO tmp_addr_node"
    "  SELECT node_id,'','',value AS street,''"
    "  FROM node_tags WHERE key='addr:street'"
    "  ON CONFLICT(node_id) DO UPDATE SET street=excluded.street;"
    " INSERT INTO tmp_addr_node"
    "  SELECT node_id,'','','',value AS housenumber"
    "  FROM node_tags WHERE key='addr:housenumber'"
    "  ON CONFLICT(node_id) DO UPDATE SET housenumber=excluded.housenumber;"
    " /*"
    " ** 4. Create temporary overall table with all addresses"
    " */"
    " CREATE TEMP TABLE tmp_addr (\n"
    "  addr_id     INTEGER PRIMARY KEY,"
    "  way_id      INTEGER,"
    "  node_id     INTEGER,"
    "  postcode    TEXT,"
    "  city        TEXT,"
    "  street      TEXT,"
    "  housenumber TEXT,"
    "  lon         REAL,"
    "  lat         REAL"
    " );\n"
    " INSERT INTO tmp_addr (way_id,node_id,postcode,city,street,housenumber,lon,lat)"
    "  SELECT w.way_id,-1 AS node_id,w.postcode,w.city,w.street,w.housenumber,c.lon,c.lat"
    "  FROM tmp_addr_way AS w"
    "  LEFT JOIN tmp_addr_way_coordinates AS c ON w.way_id=c.way_id"
    " UNION ALL"
    "  SELECT -1 AS way_id,n.node_id,n.postcode,n.city,n.street,n.housenumber,c.lon,c.lat"
    "  FROM tmp_addr_node AS n"
    "  LEFT JOIN nodes AS c ON n.node_id=c.node_id"
    " ORDER BY postcode,city,street,housenumber;"
    " /*"
    " ** 5. Fill tables 'addr_street' and 'addr_housenumber'"
    " */"
    " INSERT INTO addr_street (postcode,city,street,min_lon,min_lat,max_lon,max_lat)"
    "  SELECT postcode,city,street,min(lon),min(lat),max(lon),max(lat)"
    "  FROM tmp_addr"
    "  GROUP BY postcode,city,street;"
    " CREATE INDEX addr_street__postcode_city_street ON addr_street (postcode,city,street);\n"
    " INSERT INTO addr_housenumber (street_id,housenumber,lon,lat,way_id,node_id)"
    "  SELECT s.street_id,a.housenumber,a.lon,a.lat,a.way_id,a.node_id"
    "  FROM tmp_addr AS a"
    "  LEFT JOIN addr_street AS s ON a.postcode=s.postcode AND a.city=s.city AND a.street=s.street;"
    " CREATE INDEX addr_housenumber__street_id ON addr_housenumber (street_id);\n"
    " /*"
    " ** 6. Delete temporary tables"
    " */"
    " DROP TABLE tmp_addr_way;"
    " DROP TABLE tmp_addr_way_coordinates;"
    " DROP TABLE tmp_addr_node;"
    " DROP TABLE tmp_addr;"
    " COMMIT TRANSACTION;",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
}

/* Calculates great circle distance between two coordinates in degrees */
double distance(double lon1, double lat1, double lon2, double lat2) {
  /* Avoid a acos error if the two points are identical */
  if( lon1 == lon2 && lat1 == lat2 ) return 0;
  lon1 = lon1 * (M_PI / 180.0);   /* Conversion degree to radians */
  lat1 = lat1 * (M_PI / 180.0);
  lon2 = lon2 * (M_PI / 180.0);
  lat2 = lat2 * (M_PI / 180.0);
  /* Use earth radius Europe 6371 km (alternatively radius equator 6378 km) */
  double dist = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon2 - lon1)) * 6371000;
  return dist;    /* distance in meters */
}

void add_graph() {
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  rc = sqlite3_exec(
    db,
    " CREATE TABLE graph (\n"
    "  edge_id       INTEGER PRIMARY KEY,  -- edge ID\n"
    "  start_node_id INTEGER,              -- edge start node ID\n"
    "  end_node_id   INTEGER,              -- edge end node ID\n"
    "  dist          INTEGER,              -- distance in meters\n"
    "  way_id        INTEGER,              -- way ID\n"
    "  permit        INTEGER DEFAULT 15    -- bit field access\n"
    " )\n",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  /* Create a table with all nodes that are crossing points */
  rc = sqlite3_exec(
    db,
    " CREATE TEMP TABLE highway_nodes_crossing"
    " ("
    "  node_id INTEGER PRIMARY KEY"
    " )",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_exec(
    db,
    " INSERT INTO highway_nodes_crossing"
    " SELECT node_id FROM"
    " ("
    "  SELECT wn.node_id"
    "  FROM way_tags AS wt"
    "  LEFT JOIN way_nodes AS wn ON wt.way_id=wn.way_id"
    "  WHERE wt.key='highway'"
    " )"
    " GROUP BY node_id HAVING count(*)>1",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  /* */
  double prev_lon = 0;
  double prev_lat = 0;
  int64_t prev_way_id = -1;
  int64_t prev_node_id = -1;
  int edge_active = 0;
  int64_t start_node_id = -1;
  double dist = 0;

  sqlite3_stmt *stmt_insert_graph;
  rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO graph (start_node_id,end_node_id,dist,way_id) VALUES (?1,?2,?3,?4)",
    -1, &stmt_insert_graph, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();

  sqlite3_stmt *stmt = NULL;
  rc = sqlite3_prepare_v2(
    db,
    " SELECT"
    "  wn.way_id,wn.node_id,"
    "  ifnull(hnc.node_id,-1) AS node_id_crossing,"
    "  n.lon,n.lat"
    " FROM way_tags AS wt"
    " LEFT JOIN way_nodes AS wn ON wt.way_id=wn.way_id"
    " LEFT JOIN highway_nodes_crossing AS hnc ON wn.node_id=hnc.node_id"
    " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
    " WHERE wt.key='highway'"
    " ORDER BY wn.way_id,wn.node_order",
    -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();

  int64_t way_id;
  int64_t node_id;
  int64_t node_id_crossing;
  double lon;
  double lat;
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    way_id = sqlite3_column_int64(stmt, 0);
    node_id = sqlite3_column_int64(stmt, 1);
    node_id_crossing = sqlite3_column_int64(stmt, 2);
    lon = sqlite3_column_double(stmt, 3);
    lat = sqlite3_column_double(stmt, 4);
    /* If a new way is active but there are still remnants of the previous way, create a new edge. */
    if( way_id != prev_way_id && edge_active ) {
      sqlite3_bind_int64(stmt_insert_graph, 1, start_node_id);
      sqlite3_bind_int64(stmt_insert_graph, 2, prev_node_id);
      sqlite3_bind_int  (stmt_insert_graph, 3, lroundf(dist));
      sqlite3_bind_int64(stmt_insert_graph, 4, prev_way_id);
      rc = sqlite3_step(stmt_insert_graph);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_graph);
      } else {
        abort_db_error();
      }
      edge_active = 0;
    }
    dist = dist + distance(prev_lon, prev_lat, lon, lat);
    edge_active = 1;
    /* If way_id changes or crossing node is present then an edge begins or ends. */
    if( way_id != prev_way_id ) {
      start_node_id = node_id;
      dist = 0;
    }
    if( node_id_crossing > -1 && way_id == prev_way_id ) {
      if( start_node_id != -1 ) {
        sqlite3_bind_int64(stmt_insert_graph, 1, start_node_id);
        sqlite3_bind_int64(stmt_insert_graph, 2, node_id);
        sqlite3_bind_int  (stmt_insert_graph, 3, lroundf(dist));
        sqlite3_bind_int64(stmt_insert_graph, 4, way_id);
        rc = sqlite3_step(stmt_insert_graph);
        if( rc==SQLITE_DONE ) {
          sqlite3_reset(stmt_insert_graph);
        } else {
          abort_db_error();
        }
        edge_active = 0;
      }
      start_node_id = node_id;
      dist = 0;
    }
    prev_lon = lon;
    prev_lat = lat;
    prev_way_id = way_id;
    prev_node_id = node_id;
  }
  sqlite3_finalize(stmt);
  if( edge_active ) {
    sqlite3_bind_int64(stmt_insert_graph, 1, start_node_id);
    sqlite3_bind_int64(stmt_insert_graph, 2, node_id);
    sqlite3_bind_int  (stmt_insert_graph, 3, lroundf(dist));
    sqlite3_bind_int64(stmt_insert_graph, 4, way_id);
    rc = sqlite3_step(stmt_insert_graph);
    if( rc==SQLITE_DONE ) {
      sqlite3_reset(stmt_insert_graph);
    } else {
      abort_db_error();
    }
  }
  sqlite3_finalize(stmt_insert_graph);
  rc = sqlite3_exec(db, "CREATE INDEX graph__way_id ON graph (way_id)", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
}

/*
** readosm tag node callback function
*/
static int print_node (const void *user_data, const readosm_node * node) {
  int i;
  const readosm_tag *tag;

  sqlite3_bind_int64(stmt_insert_nodes, 1, node->id);

  if( node->latitude!=READOSM_UNDEFINED ) {
    sqlite3_bind_double(stmt_insert_nodes, 2, node->latitude);
  }
  if( node->longitude!=READOSM_UNDEFINED ) {
    sqlite3_bind_double(stmt_insert_nodes, 3, node->longitude);
  }

  rc = sqlite3_step(stmt_insert_nodes);
  if( rc==SQLITE_DONE ) {
    sqlite3_reset(stmt_insert_nodes);
  } else {
    abort_db_error();
  }

  if( node->tag_count!=0 ) {
    for (i = 0; i < node->tag_count; i++) {
      tag = node->tags + i;
      sqlite3_bind_int64(stmt_insert_node_tags, 1, node->id);
      sqlite3_bind_text (stmt_insert_node_tags, 2, tag->key, -1, NULL);
      sqlite3_bind_text (stmt_insert_node_tags, 3, tag->value, -1, NULL);
      rc = sqlite3_step(stmt_insert_node_tags);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_node_tags);
      } else {
        abort_db_error();
      }
    }
  }
  return READOSM_OK;
}

/*
** readosm tag way callback function
*/
static int print_way (const void *user_data, const readosm_way * way) {
  int i;
  const readosm_tag *tag;

  if( way->node_ref_count!=0 ) {
    for (i = 0; i < way->node_ref_count; i++) {
      sqlite3_bind_int64(stmt_insert_way_nodes, 1, way->id);
      sqlite3_bind_int64(stmt_insert_way_nodes, 2, *(way->node_refs + i));
      sqlite3_bind_int  (stmt_insert_way_nodes, 3, i+1);
      rc = sqlite3_step(stmt_insert_way_nodes);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_way_nodes);
      } else {
        abort_db_error();
      }
    }
  }

  if( way->tag_count!=0 ) {
    for (i = 0; i < way->tag_count; i++) {
      tag = way->tags + i;
      sqlite3_bind_int64(stmt_insert_way_tags, 1, way->id);
      sqlite3_bind_text (stmt_insert_way_tags, 2, tag->key, -1, NULL);
      sqlite3_bind_text (stmt_insert_way_tags, 3, tag->value, -1, NULL);
      rc = sqlite3_step(stmt_insert_way_tags);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_way_tags);
      } else {
        abort_db_error();
      }
    }
  }
  return READOSM_OK;
}

/*
** readosm tag relation callback function
*/
static int print_relation (const void *user_data, const readosm_relation * relation) {
  int i;
  const readosm_member *member;
  const readosm_tag *tag;

  sqlite3_bind_int64(stmt_insert_relation_members, 1, relation->id);

  if(relation->member_count != 0) {
    for (i = 0; i < relation->member_count; i++) {
      member = relation->members + i;
      sqlite3_bind_int64(stmt_insert_relation_members, 3, member->id);
      /* any <member> may be of "node", "way" or "relation" type */
      switch (member->member_type) {
      case READOSM_MEMBER_NODE:
        sqlite3_bind_text(stmt_insert_relation_members, 2, "node", -1, NULL);
        break;
      case READOSM_MEMBER_WAY:
        sqlite3_bind_text(stmt_insert_relation_members, 2, "way", -1, NULL);
        break;
      case READOSM_MEMBER_RELATION:
        sqlite3_bind_text(stmt_insert_relation_members, 2, "relation", -1, NULL);
        break;
      default:
        sqlite3_bind_text(stmt_insert_relation_members, 2, "", -1, NULL);
        break;
      };
      if( member->role!=NULL ) {
        sqlite3_bind_text(stmt_insert_relation_members, 4, member->role, -1, NULL);
      } else {
        sqlite3_bind_text(stmt_insert_relation_members, 4, "", -1, NULL);
      }
      sqlite3_bind_int(stmt_insert_relation_members, 5, i+1);
      rc = sqlite3_step(stmt_insert_relation_members);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_relation_members);
      } else {
        abort_db_error();
      }
    }
  }
  if( relation->tag_count != 0 ) {
    for (i = 0; i < relation->tag_count; i++) {
      tag = relation->tags + i;
      sqlite3_bind_int64(stmt_insert_relation_tags, 1, relation->id);
      sqlite3_bind_text (stmt_insert_relation_tags, 2, tag->key, -1, NULL);
      sqlite3_bind_text (stmt_insert_relation_tags, 3, tag->value, -1, NULL);
      rc = sqlite3_step(stmt_insert_relation_tags);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_relation_tags);
      } else {
        abort_db_error();
      }
    }
  }
  return READOSM_OK;
}

int read_osm_file(char *filename) {
  const void *osm_handle;
  int ret;
  /* STEP #1: opening the OSM file */
  ret = readosm_open(filename, &osm_handle);
  if( ret!=READOSM_OK ) {
    fprintf(stderr, "OPEN error: %d\n", ret);
    readosm_close(osm_handle);
    return EXIT_FAILURE;
  }
  /* STEP #2: parsing the OSM file */
  ret = readosm_parse(osm_handle, (const void *) 0,
          print_node, print_way, print_relation);
  if( ret!=READOSM_OK ) {
    fprintf(stderr, "PARSE error: %d\n", ret);
    readosm_close(osm_handle);
    return EXIT_FAILURE;
  }
  /* STEP #3: closing the OSM file */
  readosm_close(osm_handle);
  return EXIT_SUCCESS;
}

/*
** Show data
*/
int64_t str_to_int64(const char *str) {
  if (str == NULL) {
    return 0; /* Handle NULL input safely */
  }
  char *endptr;
  errno = 0; /* Reset errno before conversion */
  int64_t result = strtoll(str, &endptr, 10);
  /* Check for conversion errors */
  if (errno == ERANGE) {
    fprintf(stderr, "Error: Overflow or underflow occurred\n");
    return 0; /* Indicate failure */
  }
  if (endptr == str || *endptr != '\0') {
    fprintf(stderr, "Error: Invalid input - not a valid integer string\n");
    return 0; /* Indicate failure */
  }
  return result;
}

/*
** Main
*/
int main(int argc, char **argv) {
  if( argc==1 ) {
    show_help();
    return EXIT_FAILURE;
  }
  rc = sqlite3_open(argv[1], &db);  /* Open database connection */
  if( rc!=SQLITE_OK ) abort_db_error();
  int i = 2;
  while( i<argc ) {
    if( strcmp("read", argv[i])==0 && argc>=i+2 ) {
      rc = sqlite3_exec(db, " PRAGMA journal_mode = OFF;"
                            " PRAGMA page_size = 65536;"
                            " BEGIN TRANSACTION;", NULL, NULL, NULL);
      if( rc!=SQLITE_OK ) abort_db_error();
      add_tables();
      create_prep_stmt();
      read_osm_file(argv[i+1]);
      destroy_prep_stmt();
      add_index();
      rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
      if( rc!=SQLITE_OK ) abort_db_error();
      i++;
    }
    else if( strcmp("rtree", argv[i])==0 ) add_rtree();
    else if( strcmp("addr", argv[i])==0 ) add_addr();
    else if( strcmp("graph", argv[i])==0 ) add_graph();
    else if( strcmp("node", argv[i])==0 && argc>=i+2 ) {
      show_node(db, str_to_int64(argv[i+1]));
      i++;
    }
    else if( strcmp("way", argv[i])==0 && argc>=i+2 ) {
      show_way(db, str_to_int64(argv[i+1]));
      i++;
    }
    else if( strcmp("relation", argv[i])==0 && argc>=i+2 ) {
      show_relation(db, str_to_int64(argv[i+1]));
      i++;
    }
    else fprintf(stderr, "pbf2sqlite - Parameter error: '%s'?\n", argv[i]);
    i++;
  }


  rc = sqlite3_close(db);  /* Close database connection */
  if( rc!=SQLITE_OK ) abort_db_error();

  return 0;
}
