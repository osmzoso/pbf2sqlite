#ifndef PBF2SQLITE_H
#define PBF2SQLITE_H

#include <stdint.h>
#include <sqlite3.h>

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
void abort_db_error();
int64_t str_to_int64(const char *str);
void show_node(sqlite3 *db, const int64_t node_id);
void show_way(sqlite3 *db, const int64_t way_id);
void show_relation(sqlite3 *db, const int64_t relation_id);

#endif /* PBF2SQLITE_H */
