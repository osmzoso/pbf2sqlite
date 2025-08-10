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

static char *help =
  "pbf2sqlite " PBF2SQLITE_VERSION "\n"
  "\n"
  "Reads OpenStreetMap PBF data into a SQLite database.\n"
  "\n"
  "Usage:\npbf2sqlite DATABASE [OPTION ...]\n"
  "\n"
  "Main options:\n"
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
  "  noindex       Do not create indexes (not recommended)\n"
  "\n"
  ;

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
  fprintf(stderr, "pbf2sqlite - (%i) %s - %s\n", rc, sqlite3_errstr(rc), sqlite3_errmsg(db));
  sqlite3_close(db);
  exit(EXIT_FAILURE);
}

/*
** Convert and check numeric inputs
*/
int64_t argv_to_int64(const char *str) {
  char *endptr;
  errno = 0; /* Reset errno before conversion */
  int64_t value = strtoll(str, &endptr, 10);
  /* Check for conversion errors */
  if( errno==ERANGE ) {
    printf("Invalid number: Overflow or underflow occurred\n");
    exit(EXIT_FAILURE);
  }
  if( endptr==str || *endptr!='\0' ) {
    printf("Invalid number: Not a valid integer string\n");
    exit(EXIT_FAILURE);
  }
  return value;
}

double argv_to_double(const char *str) {
  char *endptr;
  double value = strtod(str, &endptr);
  /* Check if the whole string was converted */
  if( *endptr!='\0' ) {
    printf("Invalid number: Non-numeric characters '%s'\n", endptr);
    exit(EXIT_FAILURE);
  }
  return value;
}

/*
** TODO dummy function
*/
void visualize_graph(sqlite3 *db, double lon1) {
  printf("Visualize Graph\nBoundingbox: %f \n", lon1);
}

/*
** Program start 
*/
int main(int argc, char **argv) {
  char *db_name;
  char *osm_file_name;
  int read = 0;
  int rtree = 0;
  int addr = 0;
  int graph = 0;
  int64_t node_id = 0;
  int64_t way_id = 0;
  int64_t relation_id = 0;
  int index = 1;
  int vgraph = 0;
  double lon1 = 0;
  int i;
  /* Parse parameter */
  if( argc==1 ){
    printf("%s", help);
    printf("(SQLite %s and readosm %s is used)\n\n",
             sqlite3_libversion(), readosm_version());
    return EXIT_FAILURE;
  }
  db_name = argv[1];
  i = 2;
  while( i<argc ){
    if( strcmp("read", argv[i])==0 && argc>=i+2 ){
      read = 1;
      osm_file_name = argv[i+1];
      i++;
    } 
    else if( strcmp("rtree", argv[i])==0 ) rtree = 1;
    else if( strcmp("addr", argv[i])==0 ) addr = 1;
    else if( strcmp("graph", argv[i])==0 ) graph = 1;
    else if( strcmp("node", argv[i])==0 && argc>=i+2 ){
      node_id = argv_to_int64(argv[i+1]);
      i++;
    }
    else if( strcmp("way", argv[i])==0 && argc>=i+2 ){
      way_id = argv_to_int64(argv[i+1]);
      i++;
    }
    else if( strcmp("relation", argv[i])==0 && argc>=i+2 ){
      relation_id = argv_to_int64(argv[i+1]);
      i++;
    }
    else if( strcmp("noindex", argv[i])==0 ) index = 0;
    else if( strcmp("vgraph", argv[i])==0 && argc>=i+2 ){
      vgraph = 1;
      lon1 = argv_to_double(argv[i+1]);
      i++;
    }
    else {
      printf("Invalid option: %s\n", argv[i]);
      return EXIT_FAILURE;
    };
    i++;
  }
  /* Open database connection */
  rc = sqlite3_open(db_name, &db);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* Execute options */
  if( read ){
    rc = sqlite3_exec(db, " PRAGMA journal_mode = OFF;"
                          " PRAGMA page_size = 65536;"
                          " BEGIN TRANSACTION;", NULL, NULL, NULL);
    if( rc!=SQLITE_OK ) abort_db_error(db, rc);
    add_tables(db);
    create_prep_stmt(db);
    read_osm_file(osm_file_name);
    destroy_prep_stmt();
    if( index ) add_index(db);
    rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
    if( rc!=SQLITE_OK ) abort_db_error(db, rc);
    rc = sqlite3_exec(db, "ANALYZE", NULL, NULL, NULL);
    if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  }
  if( rtree ) add_rtree(db);
  if( addr ) add_addr(db);
  if( graph ) add_graph(db);
  if( node_id ) show_node(db, node_id);
  if( way_id ) show_way(db, way_id);
  if( relation_id ) show_relation(db, relation_id);
  if( vgraph ) visualize_graph(db, lon1);
  /* Close database connection */
  rc = sqlite3_close(db);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  return EXIT_SUCCESS;
}
