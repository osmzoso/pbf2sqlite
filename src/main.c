/*
** pbf2sqlite
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sqlite3.h>
#include <readosm.h>

/* Mathematical Constants */
#ifndef M_PI
# define M_PI   3.141592653589793238462643383279502884
#endif

#define PBF2SQLITE_VERSION  "0.4.3 ALPHA"
#define PBF2SQLITE_MAX_POINTS 1000

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
  "Show data:\n"
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
sqlite3_stmt *stmt_insert_nodes, *stmt_insert_node_tags, *stmt_insert_way_nodes,
             *stmt_insert_way_tags, *stmt_insert_relation_members, *stmt_insert_relation_tags;

typedef struct {
  int no;
  double lon;
  double lat;
} point;

/*
** Convert and check numeric inputs
*/
int64_t get_arg_int64(char **argv, int i) {
  int64_t value;
  char *endptr;
  errno = 0; /* Reset errno before conversion */
  value = strtoll(argv[i], &endptr, 10);
  /* Check for conversion errors */
  if( errno==ERANGE ) {
    printf("Invalid number: Overflow or underflow occurred\n");
    exit(EXIT_FAILURE);
  }
  if( endptr==argv[i] || *endptr!='\0' ) {
    printf("Invalid number: Not a valid integer string\n");
    exit(EXIT_FAILURE);
  }
  return value;
}

double get_arg_double(char **argv, int i) {
  double value;
  char *endptr;
  value = strtod(argv[i], &endptr);
  /* Check if the whole string was converted */
  if( *endptr!='\0' ) {
    printf("Invalid number: Non-numeric characters '%s'\n", endptr);
    exit(EXIT_FAILURE);
  }
  return value;
}

#include "functions.c"
#include "leaflet.c"
#include "graph.c"
#include "read_osm.c"
#include "options.c"
#include "show_data.c"

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
      id = get_arg_int64(argv, i+1);
      if( exec ) show_node(db, id);
      i++;
    } 
    else if( strcmp("way", argv[i])==0 && argc>=i+2 ){
      id = get_arg_int64(argv, i+1);
      if( exec ) show_way(db, id);
      i++;
    } 
    else if( strcmp("relation", argv[i])==0 && argc>=i+2 ){
      id = get_arg_int64(argv, i+1);
      if( exec ) show_relation(db, id);
      i++;
    } 
    else if( strcmp("vaddr", argv[i])==0 && argc>=i+6 ){
      lon1 = get_arg_double(argv, i+1);
      lat1 = get_arg_double(argv, i+2);
      lon2 = get_arg_double(argv, i+3);
      lat2 = get_arg_double(argv, i+4);
      if( exec ) html_map_addr(db, lon1, lat1, lon2, lat2, argv[i+5]);
      i = i + 5;
    } 
    else if( strcmp("vgraph", argv[i])==0 && argc>=i+6 ){
      lon1 = get_arg_double(argv, i+1);
      lat1 = get_arg_double(argv, i+2);
      lon2 = get_arg_double(argv, i+3);
      lat2 = get_arg_double(argv, i+4);
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
  parse_args(db, argc, argv, 0);       /* Check args, no execution */
  rc = sqlite3_open(argv[1], &db);     /* Open database connection */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_exec(db,                /* Set PRAGMAs */
          " PRAGMA journal_mode = OFF;"
          " PRAGMA page_size = 65536;", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  register_functions(db);              /* Register custom functions */
  parse_args(db, argc, argv, 1);       /* Execute args */
  rc = sqlite3_close(db);              /* Close database connection */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  return EXIT_SUCCESS;
}
