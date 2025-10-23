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
  "Reads OpenStreetMap data into a SQLite database.\n"
  "\n"
  "Usage:\npbf2sqlite DATABASE [OPTION ...]\n"
  "\n"
  "Main options:\n"
  "  read FILE     Reads FILE (.osm.pbf or .osm) into the database\n"
  "  index         Add basic indexes\n"
  "  rtree         Add R*Tree indexes\n"
  "  addr          Add address tables\n"
  "  graph         Add graph table\n"
  "\n"
  "Other options:\n"
  "  node ID                                 Show node data\n"
  "  way ID                                  Show way data\n"
  "  relation ID                             Show relation data\n"
  "  vaddr  LON1 LAT1 LON2 LAT2 HTMLFILE     Visualize address data\n"
  "  vgraph LON1 LAT1 LON2 LAT2 HTMLFILE     Visualize graph data\n"
  "  sql STATEMENT                           Execute SQL statement\n"
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
  int64_t id;
  double lon1, lat1, lon2, lat2;
  i = 2;
  while( i<argc ){
    if( strcmp("read", argv[i])==0 && argc>=i+2 ){
      if( exec ) read_osm_file(db, argv[i+1]);
      i++;
    } 
    else if( strcmp("index", argv[i])==0 ){
      if( exec ) add_index(db);
    }
    else if( strcmp("rtree", argv[i])==0 ){
      if( exec ) add_rtree(db);
    }
    else if( strcmp("addr", argv[i])==0 ){
      if( exec ) add_addr(db);
    }
    else if( strcmp("graph", argv[i])==0 ){
      if( exec ) add_graph(db);
    }
    else if( strcmp("node", argv[i])==0 && argc>=i+2 ){
      id = argv_to_int64(argv[i+1]);
      if( exec ) show_node(db, id);
      i++;
    } 
    else if( strcmp("way", argv[i])==0 && argc>=i+2 ){
      id = argv_to_int64(argv[i+1]);
      if( exec ) show_way(db, id);
      i++;
    } 
    else if( strcmp("relation", argv[i])==0 && argc>=i+2 ){
      id = argv_to_int64(argv[i+1]);
      if( exec ) show_relation(db, id);
      i++;
    } 
    else if( strcmp("vaddr", argv[i])==0 && argc>=i+6 ){
      lon1 = argv_to_double(argv[i+1]);
      lat1 = argv_to_double(argv[i+2]);
      lon2 = argv_to_double(argv[i+3]);
      lat2 = argv_to_double(argv[i+4]);
      if( exec ) html_map_addr(db, lon1, lat1, lon2, lat2, argv[i+5]);
      i = i + 5;
    } 
    else if( strcmp("vgraph", argv[i])==0 && argc>=i+6 ){
      lon1 = argv_to_double(argv[i+1]);
      lat1 = argv_to_double(argv[i+2]);
      lon2 = argv_to_double(argv[i+3]);
      lat2 = argv_to_double(argv[i+4]);
      if( exec ) html_map_graph(db, lon1, lat1, lon2, lat2, argv[i+5]);
      i = i + 5;
    } 
    else if( strcmp("sql", argv[i])==0 && argc>=i+2 ){
      if( exec ) exec_sql_stmt(db, argv[i+1]);
      i++;
    } 
    else {
      printf("Incorrect option '%s'\n", argv[i]);
      exit(EXIT_FAILURE);
    };
    i++;
  }
}

/*
** Program start 
*/
int main(int argc, char **argv) {
  if( argc==1 ){
    printf("%s", help);
    printf("SQLite %s and readosm %s are used.\n\n",
             sqlite3_libversion(), readosm_version());
    return EXIT_FAILURE;
  }
  parse_args(db, argc, argv, 0);       /* Check args */
  rc = sqlite3_open(argv[1], &db);     /* Open database connection */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  register_functions(db);
  parse_args(db, argc, argv, 1);       /* Execute args */
  rc = sqlite3_close(db);              /* Close database connection */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  return EXIT_SUCCESS;
}
