/**
 * \file main.c
 * \brief Start program
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <float.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <sqlite3.h>
#include <readosm.h>

#ifndef M_PI
# define M_PI   3.141592653589793238462643383279502884
#endif

#define PBF2SQLITE_VERSION  "0.5.5"

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define RESET   "\033[0m"

static char *built_in_help =
#ifdef DEBUG
  RED "\n!!!!! Warning: This is a DEBUG build. "__DATE__" "__TIME__" !!!!!\n" RESET
#endif
  "\n"
  "Imports PBF or XML OpenStreetMap data into an SQLite database.\n"
  "\n"
  "Usage:\npbf2sqlite <database> [OPTION ...]\n"
  "\n"
  "Main options:\n"
  "  read <file>                                         Reads an .osm.pbf or .osm file\n"
  "  index                                               Add basic indexes\n"
  "  rtree                                               Add R*Tree indexes\n"
  "  addr                                                Add address tables\n"
  "  graph                                               Add graph tables\n"
  "\n"
  "Options for displaying data:\n"
  "  node <id>                                           Show data of a node\n"
  "  way <id>                                            Show data of a way\n"
  "  relation <id>                                       Show data of a relation\n"
  "  vaddr <lon1> <lat1> <lon2> <lat2> <htmlfile>        Generates a map of the addresses\n"
  "  vgraph <lon1> <lat1> <lon2> <lat2> <htmlfile>       Generates a map of the graph\n"
  "  sql [<stmt>]                                        Executes an SQL statement\n"
  "\n"
  "Calculate shortest path (<permit> can be: foot bike roadbike car):\n"
  "  route <permit> <lon1> <lat1> <lon2> <lat2> [<lon3> <lat3> ...] <file>\n"
  "\n"
  ;

/**
 * \brief Defines a bounding box (rectangular boundary)
 */
typedef struct {
  double min_lon;
  double min_lat;
  double max_lon;
  double max_lat;
} bbox;

/* Public variables */
sqlite3 *db;                                 /**< SQLite Database connection */
int rc;                                      /**< SQLite Result code */
sqlite3_stmt *stmt_insert_nodes;             /**< SQlite Prepared Statement Object insert nodes */
sqlite3_stmt *stmt_insert_node_tags;         /**< SQlite Prepared Statement Object insert node_tags */
sqlite3_stmt *stmt_insert_way_nodes;         /**< SQlite Prepared Statement Object insert way_nodes */
sqlite3_stmt *stmt_insert_way_tags;          /**< SQlite Prepared Statement Object insert way_tags */
sqlite3_stmt *stmt_insert_relation_members;  /**< SQlite Prepared Statement Object insert relation_members */
sqlite3_stmt *stmt_insert_relation_tags;     /**< SQlite Prepared Statement Object insert relation_tags */
int duplicate_nodes;                         /**< Number of nodes that could not be inserted */

#include "functions.c"
#include "nodelist.c"
#include "leaflet.c"
#include "dijkstra.c"
#include "routing.c"
#include "read_osm.c"
#include "add_data.c"
#include "show_data.c"

/**
 * \brief Parses the arguments and calls the functions if exec is true
 */
void parse_args(sqlite3 *db, int argc, char **argv, int exec) {
  int i;
  int64_t id;
  bbox b;
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
    else if( strcmp("node", argv[2])==0 && argc==4 ){
      id = get_argv_int64(argv, 3);
      if( exec ) show_node(db, id);
      break;
    } 
    else if( strcmp("way", argv[2])==0 && argc==4 ){
      id = get_argv_int64(argv, 3);
      if( exec ) show_way(db, id);
      break;
    } 
    else if( strcmp("relation", argv[2])==0 && argc==4 ){
      id = get_argv_int64(argv, 3);
      if( exec ) show_relation(db, id);
      break;
    } 
    else if( strcmp("vaddr", argv[2])==0 && argc==8 ){
      b.min_lon = get_argv_double(argv, 3);
      b.min_lat = get_argv_double(argv, 4);
      b.max_lon = get_argv_double(argv, 5);
      b.max_lat = get_argv_double(argv, 6);
      if( exec ) html_map_addr(db, b, argv[7]);
      break;
    } 
    else if( strcmp("vgraph", argv[2])==0 && argc==8 ){
      b.min_lon = get_argv_double(argv, 3);
      b.min_lat = get_argv_double(argv, 4);
      b.max_lon = get_argv_double(argv, 5);
      b.max_lat = get_argv_double(argv, 6);
      if( exec ) html_map_graph(db, b, argv[7]);
      break;
    } 
    else if( strcmp("sql", argv[2])==0 && argc==4 ){
      if( exec ) sql_exec_stmt(db, argv[3]);
      break;
    } 
    else if( strcmp("sql", argv[2])==0 && argc==3 ){
      if( exec ) sql_read_stdin(db);
      break;
    } 
    else if( strcmp("route", argv[2])==0 && argc>=9 ){
      if( exec ) route(db, argc, argv);
      break;
    } 
    else {
      printf("Incorrect option '%s', type 'pbf2sqlite' without parameters to display help.\n", argv[i]);
      exit(EXIT_FAILURE);
    };
    i++;
  }
}

/**
 * \brief Program start 
 */
int main(int argc, char **argv) {
  if( argc==1 ){
    printf("\npbf2sqlite %s (with SQLite %s and readosm %s)\n%s",
             PBF2SQLITE_VERSION, sqlite3_libversion(), readosm_version(), built_in_help);
    return EXIT_FAILURE;
  }
  parse_args(db, argc, argv, 0);       /* Check args, no execution */
  rc = sqlite3_initialize();           /* Initialize the SQLite library */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
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
