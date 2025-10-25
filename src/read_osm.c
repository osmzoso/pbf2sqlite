/*
** Reading OpenStreetMap data with the ReadOSM library
** https://www.gaia-gis.it/fossil/readosm/index
*/
#include "pbf2sqlite.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <readosm.h>

sqlite3_stmt *stmt_insert_nodes, *stmt_insert_node_tags, *stmt_insert_way_nodes,
             *stmt_insert_way_tags, *stmt_insert_relation_members, *stmt_insert_relation_tags;

/*
** readosm tag node
*/
static int callback_node (const void *user_data, const readosm_node * node) {
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
    abort_db_error(db, rc);
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
        abort_db_error(db, rc);
      }
    }
  }
  return READOSM_OK;
}

/*
** readosm tag way
*/
static int callback_way (const void *user_data, const readosm_way * way) {
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
        abort_db_error(db, rc);
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
        abort_db_error(db, rc);
      }
    }
  }
  return READOSM_OK;
}

/*
** readosm tag relation
*/
static int callback_relation (const void *user_data, const readosm_relation * relation) {
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
        abort_db_error(db, rc);
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
        abort_db_error(db, rc);
      }
    }
  }
  return READOSM_OK;
}

int read_osm_file(sqlite3 *db, char *filename) {
  const void *osm_handle;
  int ret;
  /* 1. Start transaction */
  rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* 2. Add tables */
  rc = sqlite3_exec(db,
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
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* 3. Create prepared statements */
  rc = sqlite3_prepare_v2(db,
         "INSERT INTO nodes (node_id,lat,lon) VALUES (?1,?2,?3)",
         -1, &stmt_insert_nodes, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
         "INSERT INTO node_tags (node_id,key,value) VALUES (?1,?2,?3)",
         -1, &stmt_insert_node_tags, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
         "INSERT INTO way_nodes (way_id,node_id,node_order) VALUES (?1,?2,?3)",
         -1, &stmt_insert_way_nodes, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
         "INSERT INTO way_tags (way_id,key,value) VALUES (?1,?2,?3)",
         -1, &stmt_insert_way_tags, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
         "INSERT INTO relation_members (relation_id,ref,ref_id,role,member_order) VALUES (?1,?2,?3,?4,?5)",
         -1, &stmt_insert_relation_members, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
         "INSERT INTO relation_tags (relation_id,key,value) VALUES (?1,?2,?3)",
         -1, &stmt_insert_relation_tags, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* 4. Opening the OSM file */
  ret = readosm_open(filename, &osm_handle);
  if( ret!=READOSM_OK ) {
    fprintf(stderr, "OPEN error: %d\n", ret);
    readosm_close(osm_handle);
    return EXIT_FAILURE;
  }
  /* 5. Parsing the OSM file */
  ret = readosm_parse(osm_handle, (const void *) 0,
          callback_node, callback_way, callback_relation);
  if( ret!=READOSM_OK ) {
    fprintf(stderr, "PARSE error: %d\n", ret);
    readosm_close(osm_handle);
    return EXIT_FAILURE;
  }
  /* 6. Closing the OSM file */
  readosm_close(osm_handle);
  /* 7. End transaction */
  rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* 8. Destroy prepared statements */
  sqlite3_finalize(stmt_insert_nodes);
  sqlite3_finalize(stmt_insert_node_tags);
  sqlite3_finalize(stmt_insert_way_nodes);
  sqlite3_finalize(stmt_insert_way_tags);
  sqlite3_finalize(stmt_insert_relation_members);
  sqlite3_finalize(stmt_insert_relation_tags);
  return EXIT_SUCCESS;
}
