/*
** pbf2sqlite
*/
#include "pbf2sqlite.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sqlite3.h>
#include <readosm.h>

void show_help() {
  fprintf(stdout,
  "pbf2sqlite " PBF2SQLITE_VERSION "\n"
  "\n"
  "Reads OpenStreetMap PBF data into a SQLite database.\n"
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
  fprintf(stdout, "(SQLite %s and readosm %s is used)\n\n",
                    sqlite3_libversion(), readosm_version());
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
void abort_db_error(sqlite3 *db, int rc) {
  fprintf(stderr, "abort pbf2sqlite - (%i) %s - %s\n", rc, sqlite3_errstr(rc), sqlite3_errmsg(db));
  sqlite3_close(db);
  exit(EXIT_FAILURE);
}

int64_t str_to_int64(const char *str) {
  if( str==NULL ) return 0;  /* Handle NULL input safely */
  char *endptr;
  errno = 0; /* Reset errno before conversion */
  int64_t result = strtoll(str, &endptr, 10);
  /* Check for conversion errors */
  if( errno==ERANGE ) {
    fprintf(stderr, "Error: Overflow or underflow occurred\n");
    return 0; /* Indicate failure */
  }
  if( endptr==str || *endptr!='\0' ) {
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
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  int i = 2;
  while( i<argc ) {
    if( strcmp("read", argv[i])==0 && argc>=i+2 ) {
      rc = sqlite3_exec(db, " PRAGMA journal_mode = OFF;"
                            " PRAGMA page_size = 65536;"
                            " BEGIN TRANSACTION;", NULL, NULL, NULL);
      if( rc!=SQLITE_OK ) abort_db_error(db, rc);
      add_tables(db);
      create_prep_stmt(db);
      read_osm_file(argv[i+1]);
      destroy_prep_stmt();
      add_index(db);
      rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
      if( rc!=SQLITE_OK ) abort_db_error(db, rc);
      rc = sqlite3_exec(db, "ANALYZE", NULL, NULL, NULL);
      if( rc!=SQLITE_OK ) abort_db_error(db, rc);
      i++;
    }
    else if( strcmp("rtree", argv[i])==0 ) add_rtree(db);
    else if( strcmp("addr", argv[i])==0 ) add_addr(db);
    else if( strcmp("graph", argv[i])==0 ) add_graph(db);
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
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);

  return 0;
}
