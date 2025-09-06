/*
** Visualize data
*/
#include "pbf2sqlite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

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
  leaflet_html_header(html, "map demo");
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

void edge_points(
  sqlite3 *db,
  uint64_t way_id,
  uint64_t start_node_id,
  uint64_t end_node_id,
  point *pointlist
){
  int n;
  sqlite3_stmt *stmt_points;
  n = 0;
  rc = sqlite3_prepare_v2(db,
    " SELECT n.lon,n.lat"
    " FROM way_nodes AS wn"
    " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
    " WHERE wn.way_id=?1"
    "   AND wn.node_order>=(SELECT node_order FROM way_nodes"
    "                       WHERE way_id=?2 AND node_id=?3)"
    "   AND wn.node_order<=(SELECT node_order FROM way_nodes"
    "                       WHERE way_id=?4 AND node_id=?5)"
    " ORDER BY wn.node_order",
     -1, &stmt_points, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt_points, 1, way_id);
  sqlite3_bind_int64(stmt_points, 2, way_id);
  sqlite3_bind_int64(stmt_points, 3, start_node_id);
  sqlite3_bind_int64(stmt_points, 4, way_id);
  sqlite3_bind_int64(stmt_points, 5, end_node_id);
  while( sqlite3_step(stmt_points)==SQLITE_ROW ){
    pointlist[n].lon = (double)sqlite3_column_double(stmt_points, 0);
    pointlist[n].lat = (double)sqlite3_column_double(stmt_points, 1);
    n++;
    if( n >= PBF2SQLITE_MAX_POINTS ){
      printf("More than %d edge points in way %" PRId64 " "
             "(start_node %" PRId64 ", end_node %" PRId64 ")\n",
               PBF2SQLITE_MAX_POINTS, way_id, start_node_id, end_node_id);
      break;
    }
  }
  pointlist[0].no = n;
  sqlite3_finalize(stmt_points);
  /*
  ** If no nodes were found then search in the opposite direction
  */
  if( n==0 ){
    rc = sqlite3_prepare_v2(db,
      " SELECT n.lon,n.lat"
      " FROM way_nodes AS wn"
      " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
      " WHERE wn.way_id=?1"
      "   AND wn.node_order>=(SELECT node_order FROM way_nodes"
      "                       WHERE way_id=?2 AND node_id=?3)"
      "   AND wn.node_order<=(SELECT node_order FROM way_nodes"
      "                       WHERE way_id=?4 AND node_id=?5)"
      " ORDER BY wn.node_order DESC  -- nodes in reverse order",
     -1, &stmt_points, NULL);
    if( rc!=SQLITE_OK ) abort_db_error(db, rc);
    sqlite3_bind_int64(stmt_points, 1, way_id);
    sqlite3_bind_int64(stmt_points, 2, way_id);
    sqlite3_bind_int64(stmt_points, 3, end_node_id);
    sqlite3_bind_int64(stmt_points, 4, way_id);
    sqlite3_bind_int64(stmt_points, 5, start_node_id);
    while( sqlite3_step(stmt_points)==SQLITE_ROW ){
      pointlist[n].lon = (double)sqlite3_column_double(stmt_points, 0);
      pointlist[n].lat = (double)sqlite3_column_double(stmt_points, 1);
      n++;
      if( n >= PBF2SQLITE_MAX_POINTS ){
        printf("More than %d edge points in way %" PRId64 " "
               "(start_node %" PRId64 ", end_node %" PRId64 ")\n",
                 PBF2SQLITE_MAX_POINTS, way_id, start_node_id, end_node_id);
        break;
      }
    }
    pointlist[0].no = n;
    sqlite3_finalize(stmt_points);
  }
}

/*
** Creates subgraph for a given boundingbox.
** The result is stored in the temp. table 'subgraph'.
*/
void create_subgraph_tables(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const int mask_permit
){
  sqlite3_stmt *stmt_subgraph;
  rc = sqlite3_exec(db, "DROP TABLE IF EXISTS subgraph", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
    " CREATE TEMP TABLE subgraph AS"
    " SELECT edge_id,start_node_id,end_node_id,dist,way_id,"
    "        CASE"
    "          WHEN (?1&2=2 AND permit&16=16) OR"
    "               (?2&4=4 AND permit&32=32) THEN 1"
    "          ELSE 0"
    "        END AS directed"
    " FROM graph"
    " WHERE permit & ?3 = ?4 AND"
    "       way_id IN ("
    "                  SELECT way_id FROM rtree_way"
    "                  WHERE max_lon>=?5 AND min_lon<=?6"
    "                    AND max_lat>=?7 AND min_lat<=?8"
    "                 )",
     -1, &stmt_subgraph, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int(stmt_subgraph, 1, mask_permit);
  sqlite3_bind_int(stmt_subgraph, 2, mask_permit);
  sqlite3_bind_int(stmt_subgraph, 3, mask_permit);
  sqlite3_bind_int(stmt_subgraph, 4, mask_permit);
  sqlite3_bind_double(stmt_subgraph, 5, lon1);
  sqlite3_bind_double(stmt_subgraph, 6, lon2);
  sqlite3_bind_double(stmt_subgraph, 7, lat1);
  sqlite3_bind_double(stmt_subgraph, 8, lat2);
  rc = sqlite3_step(stmt_subgraph);
  if( rc==SQLITE_DONE ){
    sqlite3_reset(stmt_subgraph);
  }else{
    abort_db_error(db, rc);
  }
  sqlite3_finalize(stmt_subgraph);
}

/*
** Creates visualization of the table graph
*/
void html_map_graph(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
){
  FILE *html;
  sqlite3_stmt *stmt_edges;
  int permit;
  char popuptext[200];
  int64_t way_id, start_node_id, end_node_id;
  point *pointlist = malloc(PBF2SQLITE_MAX_POINTS * sizeof(point));
  if( !pointlist ){
    fprintf(stderr, "malloc failed");
    return;
  }
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file %s: %s", html_file, strerror(errno));
    return;
  }
  leaflet_html_header(html, "map graph");
  fprintf(html,
    "<h3>Map 1 - Visualization 'graph'</h3>\n"
    "<div id='map1' style='width:850px; height:500px;'></div>\n"
    "<h3>Map 2 - Graph foot</h3>\n"
    "<div id='map2' style='width:850px; height:500px;'></div>\n"
    "<h3>Map 3 - Graph bike</h3>\n"
    "<div id='map3' style='width:850px; height:500px;'></div>\n"
    "<h3>Map 4 - Graph car</h3>\n"
    "<div id='map4' style='width:850px; height:500px;'></div>\n");
  fprintf(html, "<script>\n");
  /* init maps */
  leaflet_init(html, "map1", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map2", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map3", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map4", lon1, lat1, lon2, lat2);
  /* show boundingbox */
  leaflet_style(html, "#ff0000", 1.0, 1, "", "none", 0.3);
  leaflet_rectangle(html, "map1", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map2", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map3", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map4", lon1, lat1, lon2, lat2, "");
  /* show graph edges */
  create_subgraph_tables(db, lon1, lat1, lon2, lat2, 1);
  leaflet_style(html, "#0000ff", 0.9, 2, "", "none", 1.0);
  rc = sqlite3_prepare_v2(db,
    " SELECT permit,way_id,start_node_id,end_node_id"
    " FROM graph"
    " WHERE way_id IN ("
    "     SELECT way_id FROM rtree_way"
    "     WHERE max_lon>=?1 AND min_lon<=?2"
    "     AND max_lat>=?3 AND min_lat<=?4"
    "    )",
     -1, &stmt_edges, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_double(stmt_edges, 1, lon1);
  sqlite3_bind_double(stmt_edges, 2, lon2);
  sqlite3_bind_double(stmt_edges, 3, lat1);
  sqlite3_bind_double(stmt_edges, 4, lat2);
  while( sqlite3_step(stmt_edges)==SQLITE_ROW ){
    permit = (int)sqlite3_column_int(stmt_edges, 0);
    way_id = (int64_t)sqlite3_column_int64(stmt_edges, 1);
    start_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 2);
    end_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 3);
    edge_points(db, way_id, start_node_id, end_node_id, pointlist);
    snprintf(popuptext, sizeof(popuptext), "way_id %" PRId64, way_id);
    leaflet_polyline(html, "map1", pointlist, popuptext);
    if( (permit&1)==1 ){                        /* foot */
      leaflet_polyline(html, "map2", pointlist, popuptext);
    }
    if( (permit&2)==2 ){                        /* bike, draw onway dotted */
      if( (permit&16)==16 ){
        leaflet_style(html, "#0000ff", 0.9, 2, "5 5", "none", 1.0);
        leaflet_polyline(html, "map3", pointlist, popuptext);
        leaflet_style(html, "#0000ff", 0.9, 2, "", "none", 1.0);
      }else{
        leaflet_polyline(html, "map3", pointlist, popuptext);
      }
    }
    if( (permit&4)==4 ){                        /* car, draw onway dotted */
      if( (permit&32)==32 ){
        leaflet_style(html, "#0000ff", 0.9, 2, "5 5", "none", 1.0);
        leaflet_polyline(html, "map4", pointlist, popuptext);
        leaflet_style(html, "#0000ff", 0.9, 2, "", "none", 1.0);
      }else{
        leaflet_polyline(html, "map4", pointlist, popuptext);
      }
    }
  }
  fprintf(html,
      "</script>\n"
      "<hr>\n"
      "<p>Boundingbox: %f %f - %f %f</p>\n", lon1, lat1, lon2, lat2);
  leaflet_html_footer(html);
  /* Close the file */
  if( fclose(html)!=0 ) {
    printf("Error closing file %s: %s", html_file, strerror(errno));
  }
  free(pointlist);
  sqlite3_finalize(stmt_edges);
}

/*
** Creates visualization of the table graph
*/
void html_map_graph_v1(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
){
  FILE *html;
  sqlite3_stmt *stmt_edges;
  int permit;
  char popuptext[200];
  int64_t way_id, start_node_id, end_node_id;
  point *pointlist = malloc(PBF2SQLITE_MAX_POINTS * sizeof(point));
  if( !pointlist ){
    fprintf(stderr, "malloc failed");
    return;
  }
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file %s: %s", html_file, strerror(errno));
    return;
  }
  leaflet_html_header(html, "map graph");
  fprintf(html,
    "<h3>Map 1 - Visualization 'graph'</h3>\n"
    "<div id='map1' style='width:850px; height:500px;'></div>\n"
    "<h3>Map 2 - Graph foot</h3>\n"
    "<div id='map2' style='width:850px; height:500px;'></div>\n"
    "<h3>Map 3 - Graph bike</h3>\n"
    "<div id='map3' style='width:850px; height:500px;'></div>\n"
    "<h3>Map 4 - Graph car</h3>\n"
    "<div id='map4' style='width:850px; height:500px;'></div>\n");
  fprintf(html, "<script>\n");
  /* init maps */
  leaflet_init(html, "map1", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map2", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map3", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map4", lon1, lat1, lon2, lat2);
  /* show boundingbox */
  leaflet_style(html, "#ff0000", 1.0, 1, "", "none", 0.3);
  leaflet_rectangle(html, "map1", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map2", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map3", lon1, lat1, lon2, lat2, "");
  leaflet_rectangle(html, "map4", lon1, lat1, lon2, lat2, "");
  /* show graph edges */
  leaflet_style(html, "#0000ff", 0.9, 2, "", "none", 1.0);
  rc = sqlite3_prepare_v2(db,
    " SELECT permit,way_id,start_node_id,end_node_id"
    " FROM graph"
    " WHERE way_id IN ("
    "     SELECT way_id FROM rtree_way"
    "     WHERE max_lon>=?1 AND min_lon<=?2"
    "     AND max_lat>=?3 AND min_lat<=?4"
    "    )",
     -1, &stmt_edges, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_double(stmt_edges, 1, lon1);
  sqlite3_bind_double(stmt_edges, 2, lon2);
  sqlite3_bind_double(stmt_edges, 3, lat1);
  sqlite3_bind_double(stmt_edges, 4, lat2);
  while( sqlite3_step(stmt_edges)==SQLITE_ROW ){
    permit = (int)sqlite3_column_int(stmt_edges, 0);
    way_id = (int64_t)sqlite3_column_int64(stmt_edges, 1);
    start_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 2);
    end_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 3);
    edge_points(db, way_id, start_node_id, end_node_id, pointlist);
    snprintf(popuptext, sizeof(popuptext), "way_id %" PRId64, way_id);
    leaflet_polyline(html, "map1", pointlist, popuptext);
    if( (permit&1)==1 ){                        /* foot */
      leaflet_polyline(html, "map2", pointlist, popuptext);
    }
    if( (permit&2)==2 ){                        /* bike, draw onway dotted */
      if( (permit&16)==16 ){
        leaflet_style(html, "#0000ff", 0.9, 2, "5 5", "none", 1.0);
        leaflet_polyline(html, "map3", pointlist, popuptext);
        leaflet_style(html, "#0000ff", 0.9, 2, "", "none", 1.0);
      }else{
        leaflet_polyline(html, "map3", pointlist, popuptext);
      }
    }
    if( (permit&4)==4 ){                        /* car, draw onway dotted */
      if( (permit&32)==32 ){
        leaflet_style(html, "#0000ff", 0.9, 2, "5 5", "none", 1.0);
        leaflet_polyline(html, "map4", pointlist, popuptext);
        leaflet_style(html, "#0000ff", 0.9, 2, "", "none", 1.0);
      }else{
        leaflet_polyline(html, "map4", pointlist, popuptext);
      }
    }
  }
  fprintf(html,
      "</script>\n"
      "<hr>\n"
      "<p>Boundingbox: %f %f - %f %f</p>\n", lon1, lat1, lon2, lat2);
  leaflet_html_footer(html);
  /* Close the file */
  if( fclose(html)!=0 ) {
    printf("Error closing file %s: %s", html_file, strerror(errno));
  }
  free(pointlist);
  sqlite3_finalize(stmt_edges);
}

/*
** Creates visualization of the table addr
*/
void html_map_addr(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
){
  FILE *html;
  sqlite3_stmt *stmt_addr;
  char popup_text[1000];
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file %s: %s", html_file, strerror(errno));
    return;
  }
  leaflet_html_header(html, "map addr");
  fprintf(html,
    "<h3>Map - Visualization 'addr'</h3>\n"
    "<div id='map' style='width:100%%; height:500px;'></div>\n");
  fprintf(html, "<script>\n");
  leaflet_init(html, "map", lon1, lat1, lon2, lat2);
  leaflet_style(html, "#ff0000", 1.0, 1, "", "none", 0.3);
  leaflet_rectangle(html, "map", lon1, lat1, lon2, lat2, "");
  const char *query = 
    " SELECT way_id,node_id,postcode,city,street,housenumber,lon,lat"
    " FROM addr_view"
    " WHERE lon>=?1 AND lat>=?2 AND lon<=?3 AND lat<=?4"
    " ORDER BY postcode,street,abs(housenumber)";
  /* 1. Map Marker */
  rc = sqlite3_prepare_v2(db, query, -1, &stmt_addr, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_double(stmt_addr, 1, lon1);
  sqlite3_bind_double(stmt_addr, 2, lat1);
  sqlite3_bind_double(stmt_addr, 3, lon2);
  sqlite3_bind_double(stmt_addr, 4, lat2);
  while( sqlite3_step(stmt_addr)==SQLITE_ROW ){
    snprintf(popup_text, sizeof(popup_text),
        "<pre>"
        "addr:postcode    : %s<br>"
        "addr:city        : %s<br>"
        "addr:street      : %s<br>"
        "addr:housenumber : %s<br>"
        "</pre>",
        (char *)sqlite3_column_text(stmt_addr, 2),
        (char *)sqlite3_column_text(stmt_addr, 3),
        (char *)sqlite3_column_text(stmt_addr, 4),
        (char *)sqlite3_column_text(stmt_addr, 5)
    );
    leaflet_marker(html, "map",
        (double)sqlite3_column_double(stmt_addr, 6),
        (double)sqlite3_column_double(stmt_addr, 7),
        popup_text);
  }
  fprintf(html, "</script>\n");
  /* 2. Table of addresses */
  fprintf(html,
      "<table border=1>\n"
      "<tr><th>way_id</th><th>node_id</th><th>addr:postcode</th><th>addr:city</th>"
      "<th>addr:street</th><th>addr:housenumber</th><th>lon</th><th>lat</th></tr>\n"
  );
  sqlite3_reset(stmt_addr);
  sqlite3_clear_bindings(stmt_addr);
  sqlite3_bind_double(stmt_addr, 1, lon1);
  sqlite3_bind_double(stmt_addr, 2, lat1);
  sqlite3_bind_double(stmt_addr, 3, lon2);
  sqlite3_bind_double(stmt_addr, 4, lat2);
  while( sqlite3_step(stmt_addr)==SQLITE_ROW ){
    fprintf(html,
        "<tr>"
        "<td>%" PRId64 "</td><td>%" PRId64 "</td><td>%s</td><td>%s</td>"
        "<td>%s</td><td>%s</td><td>%.7f</td><td>%.7f</td>"
        "</tr>\n",
        (int64_t)sqlite3_column_int64(stmt_addr, 0),
        (int64_t)sqlite3_column_int64(stmt_addr, 1),
        (char *)sqlite3_column_text(stmt_addr, 2),
        (char *)sqlite3_column_text(stmt_addr, 3),
        (char *)sqlite3_column_text(stmt_addr, 4),
        (char *)sqlite3_column_text(stmt_addr, 5),
        (double)sqlite3_column_double(stmt_addr, 6),
        (double)sqlite3_column_double(stmt_addr, 7)
    );
  }
  fprintf(html,
      "</table>\n"
      "<hr>\n"
      "<p>Boundingbox: %f %f - %f %f</p>\n", lon1, lat1, lon2, lat2);
  leaflet_html_footer(html);
  /* Close the file */
  if( fclose(html)!=0 ) {
    printf("Error closing file %s: %s", html_file, strerror(errno));
  }
  sqlite3_finalize(stmt_addr);
}
