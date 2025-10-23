/*
**
*/
#include "pbf2sqlite.h"

#include <math.h>
#include <stddef.h>

void add_rtree(sqlite3 *db) {
  const char *sql = 
  #include "opt_rtree.sql"
  ;
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
}

void add_addr(sqlite3 *db) {
  const char *sql = 
  #include "opt_addr.sql"
  ;
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
}

void fill_graph_permit(sqlite3 *db) {
  sqlite3_stmt *stmt, *stmt_mask, *stmt_update;
  int64_t way_id;
  int mask_set, mask_clear, set_bit, clear_bit, permit;
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  /* prepare statements */
  rc = sqlite3_prepare_v2(db, "SELECT DISTINCT way_id FROM graph", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
    " SELECT gp.set_bit,gp.clear_bit"
    " FROM way_tags AS wt"
    " JOIN graph_permit AS gp ON wt.key=gp.key AND wt.value=gp.value"
    " WHERE wt.way_id=?",
     -1, &stmt_mask, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db, "UPDATE graph SET permit=? WHERE way_id=?", -1, &stmt_update, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* get the ways */
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    way_id = (int64_t)sqlite3_column_int64(stmt, 0);
    mask_set = 0;
    mask_clear = 255;
    /* get the bitmasks for the tags */
    sqlite3_reset(stmt_mask);
    sqlite3_bind_int64(stmt_mask, 1, way_id);
    while( sqlite3_step(stmt_mask)==SQLITE_ROW ) {
      set_bit = (int)sqlite3_column_int(stmt_mask, 0);
      clear_bit = (int)sqlite3_column_int(stmt_mask, 1);
      mask_set = mask_set | set_bit;        /* bitwise or */
      mask_clear = mask_clear & clear_bit;  /* bitwise and */
    }
    permit = 0;
    permit = permit | mask_set;
    permit = permit & mask_clear;
    /* update */
    sqlite3_bind_int(stmt_update, 1, permit);
    sqlite3_bind_int64(stmt_update, 2, way_id);
    rc = sqlite3_step(stmt_update);
    if( rc!=SQLITE_DONE ) abort_db_error(db, rc);
    sqlite3_reset(stmt_update);
    sqlite3_clear_bindings(stmt_update);
  }
  rc = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
  sqlite3_finalize(stmt);
  sqlite3_finalize(stmt_mask);
  sqlite3_finalize(stmt_update);
}

void create_table_graph_permit(sqlite3 *db) {
  sqlite3_stmt *stmt_check;
  /* do not create the table if it already exists */
  rc = sqlite3_prepare_v2(db,
    " SELECT name FROM sqlite_master"
    " WHERE type='table' AND name='graph_permit'",
    -1, &stmt_check, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  if( sqlite3_step(stmt_check)==SQLITE_ROW ) {
        sqlite3_finalize(stmt_check);
        return;
  }
  sqlite3_finalize(stmt_check);
  /* else create the table */
  const char *sql = 
  #include "opt_graph_permit.sql"
  ;
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
}

void add_graph(sqlite3 *db) {
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  rc = sqlite3_exec(
    db,
    " CREATE TABLE graph (\n"
    "  edge_id       INTEGER PRIMARY KEY,  -- edge ID\n"
    "  start_node_id INTEGER,              -- edge start node ID\n"
    "  end_node_id   INTEGER,              -- edge end node ID\n"
    "  dist          INTEGER,              -- distance in meters\n"
    "  way_id        INTEGER,              -- way ID\n"
    "  nodes         INTEGER,              -- number of nodes\n"
    "  permit        INTEGER DEFAULT 15    -- bit field access\n"
    " )\n",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* Create a table with all nodes that are crossing points */
  rc = sqlite3_exec(
    db,
    " CREATE TEMP TABLE highway_nodes_crossing ("
    "  node_id INTEGER PRIMARY KEY"
    " );"
    " INSERT INTO highway_nodes_crossing"
    " SELECT node_id FROM"
    " ("
    "  SELECT wn.node_id"
    "  FROM way_tags AS wt"
    "  LEFT JOIN way_nodes AS wn ON wt.way_id=wn.way_id"
    "  WHERE wt.key='highway'"
    " )"
    " GROUP BY node_id HAVING count(*)>1;",
    NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* */
  double prev_lon = 0;
  double prev_lat = 0;
  int64_t prev_way_id = -1;
  int64_t prev_node_id = -1;
  int edge_active = 0;
  int64_t start_node_id = -1;
  double dist = 0;
  int nodes = 1;

  sqlite3_stmt *stmt_insert_graph;
  rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO graph (start_node_id,end_node_id,dist,way_id,nodes) VALUES (?1,?2,?3,?4,?5)",
    -1, &stmt_insert_graph, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);

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
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);

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
      sqlite3_bind_int  (stmt_insert_graph, 5, nodes);
      rc = sqlite3_step(stmt_insert_graph);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_graph);
      } else {
        abort_db_error(db, rc);
      }
      edge_active = 0;
    }
    dist = dist + distance(prev_lon, prev_lat, lon, lat);
    nodes++;
    edge_active = 1;
    /* If way_id changes or crossing node is present then an edge begins or ends. */
    if( way_id != prev_way_id ) {
      start_node_id = node_id;
      dist = 0;
      nodes = 1;
    }
    if( node_id_crossing > -1 && way_id == prev_way_id ) {
      if( start_node_id != -1 ) {
        sqlite3_bind_int64(stmt_insert_graph, 1, start_node_id);
        sqlite3_bind_int64(stmt_insert_graph, 2, node_id);
        sqlite3_bind_int  (stmt_insert_graph, 3, lroundf(dist));
        sqlite3_bind_int64(stmt_insert_graph, 4, way_id);
        sqlite3_bind_int  (stmt_insert_graph, 5, nodes);
        rc = sqlite3_step(stmt_insert_graph);
        if( rc==SQLITE_DONE ) {
          sqlite3_reset(stmt_insert_graph);
        } else {
          abort_db_error(db, rc);
        }
        edge_active = 0;
      }
      start_node_id = node_id;
      dist = 0;
      nodes = 1;
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
    sqlite3_bind_int  (stmt_insert_graph, 5, nodes);
    rc = sqlite3_step(stmt_insert_graph);
    if( rc==SQLITE_DONE ) {
      sqlite3_reset(stmt_insert_graph);
    } else {
      abort_db_error(db, rc);
    }
  }
  sqlite3_finalize(stmt_insert_graph);
  rc = sqlite3_exec(db, "CREATE INDEX graph__way_id ON graph (way_id)", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  create_table_graph_permit(db);
  fill_graph_permit(db);
}
