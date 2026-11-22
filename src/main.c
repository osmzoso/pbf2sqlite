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

/* Include all source files to avoid declaring prototypes */
#include "header.h"
#include "functions.c"
#include "nodelist.c"
#include "leaflet.c"
#include "matrix.c"
#include "svg.c"
#include "dijkstra.c"
#include "routing.c"
#include "read_osm.c"
#include "add_data.c"
#include "show_data.c"
#include "drawmap.c"

/**
 * \brief Parses the arguments and calls the functions if exec is true
 */
void parse_args(sqlite3 *db, int argc, char **argv, int exec){
  int i, zoomlevel, width, height;
  int64_t id;
  double lon, lat, side_length_m;
  i = 2;
  while( i<argc ){
    if( strcmp("read", argv[i])==0 && argc>=i+2 ){
      if( exec ) command_read(db, argv[i+1]);
      i = i + 1;
    } 
    else if( strcmp("index", argv[i])==0 ){
      if( exec ) command_index(db);
    }
    else if( strcmp("rtree", argv[i])==0 ){
      if( exec ) command_rtree(db);
    }
    else if( strcmp("addr", argv[i])==0 ){
      if( exec ) command_addr(db);
    }
    else if( strcmp("graph", argv[i])==0 ){
      if( exec ) command_graph(db);
    }
    else if( strcmp("node", argv[i])==0 && argc>=i+2 ){
      id = get_argv_int64(argv, i+1);
      if( exec ) command_node(db, id);
      i = i + 1;
    } 
    else if( strcmp("way", argv[i])==0 && argc>=i+2 ){
      id = get_argv_int64(argv, i+1);
      if( exec ) command_way(db, id);
      i = i + 1;
    } 
    else if( strcmp("relation", argv[i])==0 && argc>=i+2 ){
      id = get_argv_int64(argv, i+1);
      if( exec ) command_relation(db, id);
      i = i + 1;
    } 
    else if( strcmp("vaddr", argv[i])==0 && argc>=i+5 ){
      lon = get_argv_double(argv, i+1);
      lat = get_argv_double(argv, i+2);
      side_length_m = get_argv_double(argv, i+3);
      if( exec ) command_vaddr(db, lon, lat, side_length_m, argv[i+4]);
      i = i + 4;
    } 
    else if( strcmp("vgraph", argv[i])==0 && argc>=i+6 ){
      lon = get_argv_double(argv, i+1);
      lat = get_argv_double(argv, i+2);
      side_length_m = get_argv_double(argv, i+3);
      if( exec ) command_vgraph(db, lon, lat, side_length_m, argv[i+4], argv[i+5]);
      i = i + 5;
    } 
    else if( strcmp("sql", argv[i])==0 && argc>=i+2 ){
      if( exec ) command_sql(db, argv[i+1]);
      i = i + 1;
    } 
    else if( strcmp("sql", argv[2])==0 && argc==3 ){
      if( exec ) command_sql_stdin(db);
      break;
    }
    else if( strcmp("route", argv[2])==0 && argc>=9 ){
      if( exec ) command_route(db, argc, argv);
      break;
    } 
    else if( strcmp("mercator", argv[i])==0 ){
      if( exec ) command_mercator(db);
    }
    else if( strcmp("map", argv[i])==0 && argc>=i+7 ){
      lon = get_argv_double(argv, 3);
      lat = get_argv_double(argv, 4);
      zoomlevel = (int)get_argv_int64(argv, 5);
      width = (int)get_argv_int64(argv, 6);
      height = (int)get_argv_int64(argv, 7);
      if( exec ) command_map(db, lon, lat, zoomlevel, width, height, argv[8]);
      i = i + 6;
    } 
    else if( strcmp("version", argv[i])==0 ){
      if( exec ) command_version(db);
    }
    else {
      printf("Incorrect command '%s', type '"BIN_NAME"' without parameters to display help.\n", argv[i]);
      exit(EXIT_FAILURE);
    };
    i++;
  }
}

/**
 * \brief Program start 
 */
int main(int argc, char **argv) {
  if( argc<=2 ){
#ifdef DEBUG
    printf("%s",  RED "\n!!!!! Warning: This is a DEBUG build (compiled on "__DATE__" "__TIME__"). !!!!!\n" RESET);
#endif
    printf("%s", QUICK_HELP);
    return EXIT_FAILURE;
  }
  parse_args(db, argc, argv, 0);       /* Check args, no execution */
  rc = sqlite3_initialize();           /* Initialize the SQLite library */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_open(argv[1], &db);     /* Open database connection */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_exec(db, "PRAGMA journal_mode = OFF;", NULL, NULL, NULL);  /* Set PRAGMAs */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  register_functions(db);              /* Register custom functions */
  parse_args(db, argc, argv, 1);       /* Execute args */
  rc = sqlite3_close(db);              /* Close database connection */
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  return EXIT_SUCCESS;
}
