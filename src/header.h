#ifndef HEADER_H
#define HEADER_H

#define BIN_NAME            "pbf2sqlite"
#define VERSION_NUMBER      "0.5.6"

#define QUICK_HELP \
"\n" \
"OpenStreetMap PBF/XML importer for SQLite\n" \
"\n" \
"`"BIN_NAME"` imports OpenStreetMap data from `.osm.pbf` or `.osm` files into an SQLite database.\n" \
"It also provides commands for creating spatial indexes, address and routing tables, inspecting OSM elements,\n" \
"executing SQL statements, visualizing geographic data, and calculating shortest paths.\n" \
"\n" \
"Usage:\n" \
BIN_NAME" <database> [COMMAND ...]\n" \
"\n" \
"+----------------------------------------------------+----------------------------------------------------+\n" \
"|                      Command                       |                    Description                     |\n" \
"+----------------------------------------------------+----------------------------------------------------+\n" \
"| read <file>                                        | Import data from an `.osm.pbf` or `.osm` file.     |\n" \
"| index                                              | Create basic database indexes.                     |\n" \
"| rtree                                              | Create R*Tree spatial indexes.                     |\n" \
"| addr                                               | Create address tables.                             |\n" \
"| graph                                              | Create graph tables for routing.                   |\n" \
"|                                                    |                                                    |\n" \
"| node <id>                                          | Display information about a node.                  |\n" \
"| way <id>                                           | Display information about a way.                   |\n" \
"| relation <id>                                      | Display information about a relation.              |\n" \
"| sql [<stmt>]                                       | Execute an SQL statement.                          |\n" \
"|                                                    |                                                    |\n" \
"| vaddr <lon> <lat> <length_m> <htmlfile>            | Create an HTML file that displays addresses.       |\n" \
"| vgraph <lon> <lat> <length_m> <mode> <htmlfile>    | Create an HTML file that displays the graph.       |\n" \
"|                                                    |                                                    |\n" \
"| route <mode> <lon1> <lat1> <lon2> <lat2> [<lon3> <lat3> ...] <file>                                     |\n" \
"|                                    Shortest-path routing: <mode> can be `foot` `bike` `roadbike` `car`. |\n" \
"|                                                    |                                                    |\n" \
"| map <lon> <lat> <zoom> <width> <height> <svgfile>  | Draw a map (not yet implemented...).               |\n" \
"|                                                    |                                                    |\n" \
"| mercator                                           | Add mercator projection in table `nodes`.          |\n" \
"| version                                            | Print version numbers.                             |\n" \
"+----------------------------------------------------+----------------------------------------------------+\n" \
"\n" \
"Examples:\n" \
"\n" \
"# Import an OpenStreetMap extract\n" \
BIN_NAME" map.db read germany.osm.pbf\n" \
"\n" \
"# Create normal indexes, spatial indexes and routing tables\n" \
BIN_NAME" map.db index rtree graph\n" \
"\n" \
"# Calculate a foot route\n" \
BIN_NAME" map.db route foot 13.3645 52.5108 13.4096 52.5182 my_route\n" \
"\n"

#ifndef M_PI
# define M_PI   3.141592653589793238462643383279502884
#endif

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define RESET   "\033[0m"

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

#endif  /* HEADER_H */
