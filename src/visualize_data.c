/*
** Visualize data
*/
#include "pbf2sqlite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
** Test leaflet.c
*/
int html_demo(){
  FILE *html;
  point *pointlist;
  html = fopen("demo.html", "w");
  if( html==NULL ) {
    printf("Error opening file: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  leaflet_html_header(html);
  fprintf(html,
    "<h2>Demo Map</h2>\n"
    "<div id=\"map1\" style=\"width:800px; height:800px;\"></div>\n");
  fprintf(html, "<script>\n");
  leaflet_init(html, "map1", 13.36, 52.51, 13.39, 52.53);
  leaflet_marker(html, "map1", 13.3757533, 52.518551, "Berlin Reichstag");
  leaflet_circle(html, "map1", 13.363, 52.514, 300, "A circle with default colors");
  leaflet_style(html, "#bb1122", 0.6, 2, "", "#ffffff", 0.6);
  leaflet_circle(html, "map1", 13.384, 52.517, 200, "A white circle");
  leaflet_style(html, "#992255", 1.0, 4, "4 7", "#ff0000", 0.3);
  leaflet_circle(html, "map1", 13.365, 52.518, 200, "A dotted circle");
  leaflet_style(html, "#ff0000", 0.6, 6, "", "#00ffff", 0.7);
  leaflet_line(html, "map1", 13.369, 52.513, 13.376, 52.514, "A line");

  pointlist = malloc(4 * sizeof(point));
  pointlist[0].lon = 13.368;
  pointlist[0].lat = 52.519;
  pointlist[1].lon = 13.372;
  pointlist[1].lat = 52.514;
  pointlist[2].lon = 13.382;
  pointlist[2].lat = 52.519;
  pointlist[3].lon = 13.378;
  pointlist[3].lat = 52.522;
  pointlist[0].no = 4;
  leaflet_polyline(html, "map1", pointlist, "A polyline");
  free(pointlist);

  pointlist = malloc(3 * sizeof(point));
  pointlist[0].lon = 13.367;
  pointlist[0].lat = 52.511;
  pointlist[1].lon = 13.373;
  pointlist[1].lat = 52.513;
  pointlist[2].lon = 13.376;
  pointlist[2].lat = 52.511;
  pointlist[0].no = 3;
  leaflet_polygon(html, "map1", pointlist, "A polygon");
  free(pointlist);

  leaflet_style(html, "#ff0000", 1.0, 2, "5 5", "none", 1.0);
  leaflet_rectangle(html, "map1", 13.36, 52.51, 13.39, 52.53, "A rectangle");
  leaflet_circlemarker(html, "map1", 13.3868086, 52.5203199, "A circlemarker");
  fprintf(html, "</script>\n");
  leaflet_html_footer(html);
  if( fclose(html)!=0 ) {
    printf("Error closing file: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf("File 'demo.html' written successfully.\n");
  return EXIT_SUCCESS;
}




/* TODO dummy function */
point* generate_pointlist(int n) {
  point *pointlist = malloc(n * sizeof(point));
  if( !pointlist ){
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }
  pointlist[0].lon = 7.8425217;
  pointlist[0].lat = 47.9857186;
  pointlist[1].lon = 7.8564262;
  pointlist[1].lat = 47.9892802;
  pointlist[2].lon = 7.8434658;
  pointlist[2].lat = 47.9933011;
  pointlist[3].lon = 7.8559113;
  pointlist[3].lat = 47.9961730;
  pointlist[0].no = 4;
  return pointlist;
}

/* TODO */
point* edge_points(
  sqlite3 *db,
  uint64_t way_id,
  uint64_t start_node_id,
  uint64_t end_node_id,
  int n
){
  int node_start_order, node_end_order;
  sqlite3_stmt *stmt_points, *stmt_order_pos;
  double lon, lat;
  point *pointlist = malloc(n * sizeof(point));
  if( !pointlist ){
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }
  /*  */
  rc = sqlite3_prepare_v2(db,
    " SELECT node_order FROM way_nodes WHERE way_id=? AND node_id=?",
     -1, &stmt_order_pos, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt_order_pos, 1, way_id);
  sqlite3_bind_int64(stmt_order_pos, 2, start_node_id);
  while( sqlite3_step(stmt_order_pos)==SQLITE_ROW ){
    node_start_order = (int)sqlite3_column_int(stmt_order_pos, 0);
  }

  rc = sqlite3_prepare_v2(db,
    " SELECT n.lon,n.lat"
    " FROM way_nodes AS wn"
    " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
    " WHERE wn.way_id=?"
    " ORDER BY wn.node_order",
     -1, &stmt_points, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt_points, 1, way_id);
  while( sqlite3_step(stmt_points)==SQLITE_ROW ){
    lon = (double)sqlite3_column_double(stmt_points, 0);
    lat = (double)sqlite3_column_double(stmt_points, 1);
  }
  return pointlist;
}

/*
** Creates visualization of the table graph TODO
*/
int html_graph(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
){
  FILE *html;
  sqlite3_stmt *stmt_edges;
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file: %s", strerror(errno));
    return EXIT_FAILURE;
  }
  leaflet_html_header(html);
  fprintf(html,
    "<h2>Map 1 - Graph complete</h2>\n"
    "<div id='map1' style='width:800px; height:500px;'></div>\n"
    "<h2>Map 2 - Graph foot</h2>\n"
    "<div id='map2' style='width:800px; height:500px;'></div>\n"
    "<h2>Map 3 - Graph bike</h2>\n"
    "<div id='map3' style='width:800px; height:500px;'></div>\n");
  fprintf(html, "<script>\n");
  /* init maps */
  leaflet_init(html, "map1", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map2", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map3", lon1, lat1, lon2, lat2);
  /* show boundingbox */
  leaflet_style(html, "#ff0000", 1.0, 1, "", "none", 0.3);
  leaflet_rectangle(html, "map1", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map2", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map3", lon1, lat1, lon2, lat2, "");
  /* show graph edges TODO... */
  int64_t start_node_id, end_node_id, way_id;
  int nodes, permit;
  rc = sqlite3_prepare_v2(db,
    " SELECT start_node_id,end_node_id,way_id,nodes,permit"
    " FROM graph"
    " WHERE way_id IN ("
    "     SELECT way_id FROM rtree_way"
    "     WHERE max_lon>=? AND min_lon<=?"
    "     AND max_lat>=? AND min_lat<=?"
    "    )",
     -1, &stmt_edges, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_double(stmt_edges, 1, lon1);
  sqlite3_bind_double(stmt_edges, 2, lon2);
  sqlite3_bind_double(stmt_edges, 3, lat1);
  sqlite3_bind_double(stmt_edges, 4, lat2);
  while( sqlite3_step(stmt_edges)==SQLITE_ROW ){
    start_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 0);
    end_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 1);
    way_id = (int64_t)sqlite3_column_int64(stmt_edges, 2);
    nodes = (int)sqlite3_column_int(stmt_edges, 3);
    permit = (int)sqlite3_column_int(stmt_edges, 4);
  }
  sqlite3_finalize(stmt_edges);
  /*  */
  leaflet_line(html, "map1", 7.835, 47.996, 7.863, 47.981, "Simple line");
  leaflet_style(html, "#0000ff", 0.9, 2, "", "none", 1.0);
  point *pointlist;
  pointlist = generate_pointlist(5);
  leaflet_polyline(html, "map1", pointlist, "");
  free(pointlist);
  /*  */
  fprintf(html, "</script>\n");
  leaflet_html_footer(html);
  /* Close the file */
  if( fclose(html)!=0 ) {
    printf("Error closing file: %s", strerror(errno));
    return EXIT_FAILURE;
  }
  printf("File %s written successfully.\n", html_file);
  return EXIT_SUCCESS;
}
