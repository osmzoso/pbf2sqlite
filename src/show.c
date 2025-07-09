/*
** Show data
*/
#include "pbf2sqlite.h"

#include <stdio.h>
#include <stdint.h>

void show_node(sqlite3 *db, const int64_t node_id) {
  sqlite3_stmt *stmt;
  /* Location */
  rc = sqlite3_prepare_v2(db,
    "SELECT node_id,lon,lat FROM nodes WHERE node_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, node_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    int64_t node_id = sqlite3_column_int64(stmt, 0);
    double lon = sqlite3_column_double(stmt, 1);
    double lat = sqlite3_column_double(stmt, 2);
    printf("node %ld location %.7f %.7f\n", node_id, lon, lat);
  }
  sqlite3_finalize(stmt);
  /* Tags */
  rc = sqlite3_prepare_v2(db,
    "SELECT key,value FROM node_tags WHERE node_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, node_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    const char *key = (const char *)sqlite3_column_text(stmt, 0);
    const char *value = (const char *)sqlite3_column_text(stmt, 1);
    printf("node %ld tag \"%s\":\"%s\"\n", node_id, key, value);
  }
  sqlite3_finalize(stmt);
  /* Part of relation */
  rc = sqlite3_prepare_v2(db,
    " SELECT relation_id,role"
    " FROM relation_members"
    " WHERE ref_id=? AND ref='node'", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, node_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    int64_t relation_id = sqlite3_column_int64(stmt, 0);
    const char *role = (const char *)sqlite3_column_text(stmt, 1);
    printf("node %ld part_of_relation %15ld %s\n", node_id, relation_id, role);
  }
  sqlite3_finalize(stmt);
}

void show_way(sqlite3 *db, const int64_t way_id) {
  sqlite3_stmt *stmt;
  /* Tags */
  rc = sqlite3_prepare_v2(db,
    "SELECT key,value FROM way_tags WHERE way_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, way_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    const char *key = (const char *)sqlite3_column_text(stmt, 0);
    const char *value = (const char *)sqlite3_column_text(stmt, 1);
    printf("way %ld tag \"%s\":\"%s\"\n", way_id, key, value);
  }
  sqlite3_finalize(stmt);
  /* Part of relation */
  rc = sqlite3_prepare_v2(db,
    " SELECT relation_id,role"
    " FROM relation_members"
    " WHERE ref_id=? AND ref='way'", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, way_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    int64_t relation_id = sqlite3_column_int64(stmt, 0);
    const char *role = (const char *)sqlite3_column_text(stmt, 1);
    printf("way %ld part_of_relation %15ld %s\n", way_id, relation_id, role);
  }
  sqlite3_finalize(stmt);
  /* Nodes */
  rc = sqlite3_prepare_v2(db,
    " SELECT wn.node_id,n.lat,n.lon"
    " FROM way_nodes AS wn"
    " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
    " WHERE wn.way_id=?"
    " ORDER BY wn.node_order", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, way_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    int64_t node_id = sqlite3_column_int64(stmt, 0);
    double lon = sqlite3_column_double(stmt, 1);
    double lat = sqlite3_column_double(stmt, 2);
    printf("way %ld node %15ld %.7f %.7f\n", way_id, node_id, lon, lat);
  }
  sqlite3_finalize(stmt);
}

void show_relation(sqlite3 *db, const int64_t relation_id) {
  sqlite3_stmt *stmt;
  /* Tags */
  rc = sqlite3_prepare_v2(db,
    "SELECT key,value FROM relation_tags WHERE relation_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, relation_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("relation %ld tag \"%s\":\"%s\"\n", relation_id,
             (const char *)sqlite3_column_text(stmt, 0),
             (const char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Part of relation */
  rc = sqlite3_prepare_v2(db,
    " SELECT relation_id,role"
    " FROM relation_members"
    " WHERE ref_id=? AND ref='relation'", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, relation_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("relation %ld part_of_relation %15ld %s\n", relation_id,
             (const int64_t)sqlite3_column_int64(stmt, 0),
             (const char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Members */
  rc = sqlite3_prepare_v2(db,
    " SELECT ref,ref_id,role"
    " FROM relation_members"
    " WHERE relation_id=?"
    " ORDER BY member_order", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  sqlite3_bind_int64(stmt, 1, relation_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("relation %ld member %s %15ld %s\n", relation_id,
             (const char *)sqlite3_column_text(stmt, 0), 
             (const int64_t)sqlite3_column_int64(stmt, 1),
             (const char *)sqlite3_column_text(stmt, 2) );
  }
  sqlite3_finalize(stmt);
}
