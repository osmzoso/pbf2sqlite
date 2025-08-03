#ifndef PBF2SQLITE_H
#define PBF2SQLITE_H

#include <stdint.h>
#include <sqlite3.h>

#define PBF2SQLITE_VERSION  "0.3"

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
**  Function prototypes
*/
/* main.c */
void abort_db_error(sqlite3 *db, int rc);
int64_t str_to_int64(const char *str);
/* read_data.c */
void add_tables(sqlite3 *db);
void add_index(sqlite3 *db);
void create_prep_stmt(sqlite3 *db);
void destroy_prep_stmt();
int read_osm_file(char *filename);
/* add_data.c */
void add_rtree(sqlite3 *db);
void add_addr(sqlite3 *db);
void add_graph(sqlite3 *db);
double distance(double lon1, double lat1, double lon2, double lat2);
/* show_data.c */
void show_node(sqlite3 *db, const int64_t node_id);
void show_way(sqlite3 *db, const int64_t way_id);
void show_relation(sqlite3 *db, const int64_t relation_id);

#endif /* PBF2SQLITE_H */
