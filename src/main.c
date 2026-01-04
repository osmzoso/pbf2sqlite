/*
** pbf2sqlite
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

#define PBF2SQLITE_VERSION  "0.4.4 BETA"

/*
** Public variables
*/
sqlite3 *db;         /* SQLite Database connection */
int rc;              /* SQLite Result code */
sqlite3_stmt *stmt_insert_nodes, *stmt_insert_node_tags, *stmt_insert_way_nodes,
             *stmt_insert_way_tags, *stmt_insert_relation_members, *stmt_insert_relation_tags;
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
  "Shortest way:\n"
  "  route LON1 LAT1 LON2 LAT2 PERMIT FILE\n"
  "\n"
  "This is pbf2sqlite version " PBF2SQLITE_VERSION "\n"
  ;

#include "functions.c"
#include "nodelist.c"
#include "leaflet.c"
#include "dijkstra.c"
#include "graph.c"
#include "routing.c"
#include "read_osm.c"
#include "options.c"
#include "show_data.c"
#include "get_args.c"

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
