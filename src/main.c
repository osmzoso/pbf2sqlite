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
  "\n"
  "Reads OpenStreetMap PBF data into a SQLite database.\n"
  "\n"
  "Usage:\npbf2sqlite DATABASE [OPTION ...]\n"
  "\n"
  "Main options:\n"
  "  read FILE     Reads FILE (.osm.pbf or .osm) into the database\n"
  "  rtree         Add R*Tree indexes\n"
  "  addr          Add address tables\n"
  "  graph         Add graph table\n"
  "  noindex       Do not create indexes (not recommended)\n"
  "\n"
  "Other options:\n"
  "  node ID                                 Show node data\n"
  "  way ID                                  Show way data\n"
  "  relation ID                             Show relation data\n"
  "  vgraph LON1 LAT1 LON2 LAT2 HTMLFILE     Visualize graph data\n"
  "  vaddr  LON1 LAT1 LON2 LAT2 HTMLFILE     Visualize address data\n"
  "\n"
  "This is pbf2sqlite version " PBF2SQLITE_VERSION "\n"
  ;

/*
** Public variables
*/
sqlite3 *db;         /* SQLite Database connection */
int rc;              /* SQLite Result code */

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

void parse_args(sqlite3 *db, int argc, char **argv, int exec) {
  int i;
  i = 2;
  while( i<argc ){
    if( strcmp("read", argv[i])==0 && argc>=i+2 ){
      printf("-> read %s\n", argv[i+1]);
      i++;
    } 
    else if( strcmp("index", argv[i])==0 ){
      printf("-> index\n");
    }
    else {
      printf("Invalid option: %s\n", argv[i]);
      exit(EXIT_FAILURE);
    };
    i++;
  }
}

/*
** Program start 
*/
int main(int argc, char **argv) {
  char *db_file;
  char *osm_file;
  int read = 0;
  int rtree = 0;
  int addr = 0;
  int graph = 0;
  int64_t node_id = 0;
  int64_t way_id = 0;
  int64_t relation_id = 0;
  int index = 1;
  int vgraph = 0;
  int vaddr = 0;
  double lon1 = 0;
  double lat1 = 0;
  double lon2 = 0;
  double lat2 = 0;
  char *html_file;
  int i;
  /* Parse args test TODO */
  //parse_args(db, argc, argv, 0);
  /* Parse parameter */
  if( argc==1 ){
    printf("%s", help);
    printf("SQLite %s and readosm %s are used.\n\n",
             sqlite3_libversion(), readosm_version());
    return EXIT_FAILURE;
  }
  db_file = argv[1];
  i = 2;
  while( i<argc ){
    if( strcmp("read", argv[i])==0 && argc>=i+2 ){
      read = 1;
      osm_file = argv[i+1];
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
    else if( strcmp("vgraph", argv[i])==0 && argc>=i+6 ){
      vgraph = 1;
      lon1 = argv_to_double(argv[i+1]);
      lat1 = argv_to_double(argv[i+2]);
      lon2 = argv_to_double(argv[i+3]);
      lat2 = argv_to_double(argv[i+4]);
      html_file = argv[i+5];
      i = i + 5;
    }
    else if( strcmp("vaddr", argv[i])==0 && argc>=i+6 ){
      vaddr = 1;
      lon1 = argv_to_double(argv[i+1]);
      lat1 = argv_to_double(argv[i+2]);
      lon2 = argv_to_double(argv[i+3]);
      lat2 = argv_to_double(argv[i+4]);
      html_file = argv[i+5];
      i = i + 5;
    }
    else {
      printf("Invalid option: %s\n", argv[i]);
      return EXIT_FAILURE;
    };
    i++;
  }
  /* Open database connection */
  rc = sqlite3_open(db_file, &db);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* Execute options */
  if( read ){
    rc = sqlite3_exec(db, " PRAGMA journal_mode = OFF;"
                          " PRAGMA page_size = 65536;"
                          " BEGIN TRANSACTION;", NULL, NULL, NULL);
    if( rc!=SQLITE_OK ) abort_db_error(db, rc);
    read_osm_file(db, osm_file);
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
  if( vgraph ) html_map_graph(db, lon1, lat1, lon2, lat2, html_file);
  if( vaddr ) html_map_addr(db, lon1, lat1, lon2, lat2, html_file);
  /* Close database connection */
  rc = sqlite3_close(db);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  return EXIT_SUCCESS;
}
