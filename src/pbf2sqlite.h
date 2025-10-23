#ifndef PBF2SQLITE_H
#define PBF2SQLITE_H

#include <stdint.h>
#include <stdio.h>
#include <sqlite3.h>

#define PBF2SQLITE_VERSION  "0.4.2"
#define PBF2SQLITE_MAX_POINTS 1000

typedef struct {
  int no;
  double lon;
  double lat;
} point;

/* Mathematical Constants */
#ifndef M_PI
# define M_PI   3.141592653589793238462643383279502884
#endif

/*
** Public variables
*/
extern sqlite3 *db;         /* SQLite Database connection */
extern int rc;              /* SQLite Result code */
extern sqlite3_stmt *stmt_insert_nodes, *stmt_insert_node_tags, *stmt_insert_way_nodes,
             *stmt_insert_way_tags, *stmt_insert_relation_members, *stmt_insert_relation_tags;

/*
** Function prototypes
*/
/* main.c */
void abort_db_error(sqlite3 *db, int rc);
/* read_data.c */
void add_index(sqlite3 *db);
int read_osm_file(sqlite3 *db, char *filename);
/* options.c */
void add_rtree(sqlite3 *db);
void add_addr(sqlite3 *db);
void add_graph(sqlite3 *db);
/* show_data.c */
void show_node(sqlite3 *db, const int64_t node_id);
void show_way(sqlite3 *db, const int64_t way_id);
void show_relation(sqlite3 *db, const int64_t relation_id);
/* sql_stmt.c */
void exec_sql_stmt(sqlite3 *db, const char *sql_stmt);
/* functions.c */
double radians(double deg);
double degrees(double rad);
double distance(double lon1, double lat1, double lon2, double lat2);
double mercator_x(double lon);
double mercator_y(double lat);
void register_functions(sqlite3 *db);
/* graph.c */
void edge_points(
  sqlite3 *db,
  uint64_t way_id,
  uint64_t start_node_id,
  uint64_t end_node_id,
  point *pointlist
);
int create_subgraph_tables(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const int mask_permit
);
/* leaflet.c */
void leaflet_html_header(FILE *html, const char *title);
void leaflet_html_footer(FILE *html);
void leaflet_init(
  FILE *html,
  const char *mapid,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2
);
void leaflet_marker(
  FILE *html,
  const char *mapid,
  const double lon,
  const double lat,
  const char *text
);
void leaflet_polyline(
  FILE *html,
  const char *mapid,
  point *pointlist,
  const char *text
);
void leaflet_line(
  FILE *html,
  const char *mapid,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *text
);
void leaflet_polygon(
  FILE *html,
  const char *mapid,
  point *pointlist,
  const char *text
);
void leaflet_circle(
  FILE *html,
  const char *mapid,
  const double lon,
  const double lat,
  const int radius,
  const char *text
);
void leaflet_circlemarker(
  FILE *html,
  const char *mapid,
  const double lon,
  const double lat,
  const char *text
);
void leaflet_rectangle(
  FILE *html,
  const char *mapid,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *text
);
void leaflet_style(
  FILE *html,
  const char *color,
  const double opacity,
  const int weight,
  const char *dasharray,
  const char *fillcolor,
  const double fillopacity,
  const int radius
);
/* visualize_data.c */
void html_map_graph(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
);
void html_map_addr(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
);

#endif /* PBF2SQLITE_H */
