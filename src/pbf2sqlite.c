/*
/ pbf2sqlite.c
/
/ libreadosm Sample code
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ ------------------------------------------------------------------------------
/
/ Version: MPL 1.1/GPL 2.0/LGPL 2.1
/
/ The contents of this file are subject to the Mozilla Public License Version
/ 1.1 (the "License"); you may not use this file except in compliance with
/ the License. You may obtain a copy of the License at
/ http://www.mozilla.org/MPL/
/
/ Software distributed under the License is distributed on an "AS IS" basis,
/ WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
/ for the specific language governing rights and limitations under the
/ License.
/
/ The Original Code is the ReadOSM library
/
/ The Initial Developer of the Original Code is Alessandro Furieri
/
/ Portions created by the Initial Developer are Copyright (C) 2012-2017
/ the Initial Developer. All Rights Reserved.
/
/ Contributor(s):
/
/ Alternatively, the contents of this file may be used under the terms of
/ either the GNU General Public License Version 2 or later (the "GPL"), or
/ the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
/ in which case the provisions of the GPL or the LGPL are applicable instead
/ of those above. If you wish to allow use of your version of this file only
/ under the terms of either the GPL or the LGPL, and not to allow others to
/ use your version of this file under the terms of the MPL, indicate your
/ decision by deleting the provisions above and replace them with the notice
/ and other provisions required by the GPL or the LGPL. If you do not delete
/ the provisions above, a recipient may use your version of this file under
/ the terms of any one of the MPL, the GPL or the LGPL.
/
*/

#include <stdlib.h>
#include <stdio.h>

#include <sqlite3.h>
#include <readosm.h>

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


/*
** readosm tag node callback function
*/
static int print_node (const void *user_data, const readosm_node * node) {
  int i;
  const readosm_tag *tag;

  sqlite3_bind_int64(stmt_insert_nodes, 1, node->id);

  if (node->latitude != READOSM_UNDEFINED) {
    sqlite3_bind_double(stmt_insert_nodes, 2, node->latitude);
  }
  if (node->longitude != READOSM_UNDEFINED) {
    sqlite3_bind_double(stmt_insert_nodes, 3, node->longitude);
  }

  rc = sqlite3_step(stmt_insert_nodes);
  if( rc==SQLITE_DONE ) {
    sqlite3_reset(stmt_insert_nodes);
  } else {
    abort_db_error();
  }

  if (node->tag_count != 0) {
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

  if (way->node_ref_count != 0) {
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

  if (way->tag_count != 0) {
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
      if (member->role != NULL) {
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

int main (int argc, char *argv[]) {
  const void *osm_handle;
  int ret;

  if (argc != 3) {
    fprintf (stderr, "Usage:\npbf2sqlite DATABASE OSM_PBF_FILE\n");
    return -1;
  }

  /* STEP #0 Open database */
  rc = sqlite3_open(argv[1], &db);  /* Open database connection */
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_exec(db, " PRAGMA journal_mode = OFF;"
                        " PRAGMA page_size = 65536;"
                        " BEGIN TRANSACTION;", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  add_tables();
  create_prep_stmt();

  /*
  * STEP #1: opening the OSM file
  * this can indifferently be an OSM XML encoded file (.osm)
  * or an OSM Protocol Buffer encoded file (.pbf)
  * the library will transparently perform any required
  * action in both cases.
  */
  ret = readosm_open (argv[2], &osm_handle);
  if (ret != READOSM_OK) {
    fprintf (stderr, "OPEN error: %d\n", ret);
    goto stop;
  }

  /*
  * STEP #2: parsing the OSM file
  * this task is unbelievably simple
  *
  * you are simply required to pass the appropriate
  * pointers for callback funtions respectively intended
  * to process Node-objects, Way-objects and Relation-objects
  *
  * the library will then parse the whole input file, calling
  * the appropriate callback handling function for each OSM object
  * found: please see the callback functions implementing code
  * to better understand how it works
  *
  * important notice: in this first example we'll not use at
  * all the USER_DATA pointer. so the second arg will simply
  * be (const void *)0 [i.e. NULL]
  */
  ret =
    readosm_parse (osm_handle, (const void *) 0, print_node, print_way,
                   print_relation);
  if (ret != READOSM_OK) {
    fprintf (stderr, "PARSE error: %d\n", ret);
    goto stop;
  }

  fprintf (stderr, "Ok, OSM input file successfully parsed\n");

stop:
  /*
  * STEP #3: closing the OSM file
  * this will release any internal memory allocation
  */
  readosm_close (osm_handle);

  /* STEP #4 Close database */
  destroy_prep_stmt();
  add_index();
  rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error();
  rc = sqlite3_close(db);  /* Close database connection */
  if( rc!=SQLITE_OK ) abort_db_error();

  return 0;
}
