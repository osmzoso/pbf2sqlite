/*
** Show data
*/


/*
** Execute SQL statement
*/
static int print_row(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    if( i>0 ) printf("|");
    printf("%s", argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void exec_sql_stmt(sqlite3 *db, const char *sql_stmt){
  char *zErrMsg = 0;
  rc = sqlite3_exec(db, sql_stmt, print_row, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
}

/*
** Show data for node, way or relation
*/
void show_node(sqlite3 *db, const int64_t node_id) {
  sqlite3_stmt *stmt;
  /* Location */
  rc = sqlite3_prepare_v2(db,
    "SELECT lon,lat FROM nodes WHERE node_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, node_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    printf("node %" PRId64 " location %.7f %.7f\n", node_id,
             (double)sqlite3_column_double(stmt, 0),
             (double)sqlite3_column_double(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Tags */
  rc = sqlite3_prepare_v2(db,
    "SELECT key,value FROM node_tags WHERE node_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, node_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("node %" PRId64 " tag \"%s\":\"%s\"\n", node_id,
             (char *)sqlite3_column_text(stmt, 0),
             (char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Part of relation */
  rc = sqlite3_prepare_v2(db,
    " SELECT relation_id,role"
    " FROM relation_members"
    " WHERE ref_id=? AND ref='node'", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, node_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("node %" PRId64 " part_of_relation %15" PRId64 " %s\n", node_id,
             (int64_t)sqlite3_column_int64(stmt, 0),
             (char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
}

void show_way(sqlite3 *db, const int64_t way_id) {
  sqlite3_stmt *stmt;
  /* Tags */
  rc = sqlite3_prepare_v2(db,
    "SELECT key,value FROM way_tags WHERE way_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, way_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("way %" PRId64 " tag \"%s\":\"%s\"\n", way_id,
             (char *)sqlite3_column_text(stmt, 0),
             (char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Part of relation */
  rc = sqlite3_prepare_v2(db,
    " SELECT relation_id,role"
    " FROM relation_members"
    " WHERE ref_id=? AND ref='way'", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, way_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("way %" PRId64 " part_of_relation %15" PRId64 " %s\n", way_id,
             (int64_t)sqlite3_column_int64(stmt, 0),
             (char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Nodes */
  rc = sqlite3_prepare_v2(db,
    " SELECT wn.node_id,n.lat,n.lon"
    " FROM way_nodes AS wn"
    " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
    " WHERE wn.way_id=?"
    " ORDER BY wn.node_order", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, way_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("way %" PRId64 " node %15" PRId64 " %.7f %.7f\n", way_id,
             (int64_t)sqlite3_column_int64(stmt, 0),
             (double)sqlite3_column_double(stmt, 1),
             (double)sqlite3_column_double(stmt, 2) );
  }
  sqlite3_finalize(stmt);
}

void show_relation(sqlite3 *db, const int64_t relation_id) {
  sqlite3_stmt *stmt;
  /* Tags */
  rc = sqlite3_prepare_v2(db,
    "SELECT key,value FROM relation_tags WHERE relation_id=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, relation_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("relation %" PRId64 " tag \"%s\":\"%s\"\n", relation_id,
             (char *)sqlite3_column_text(stmt, 0),
             (char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Part of relation */
  rc = sqlite3_prepare_v2(db,
    " SELECT relation_id,role"
    " FROM relation_members"
    " WHERE ref_id=? AND ref='relation'", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, relation_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("relation %" PRId64 " part_of_relation %15" PRId64 " %s\n", relation_id,
             (int64_t)sqlite3_column_int64(stmt, 0),
             (char *)sqlite3_column_text(stmt, 1) );
  }
  sqlite3_finalize(stmt);
  /* Members */
  rc = sqlite3_prepare_v2(db,
    " SELECT ref,ref_id,role"
    " FROM relation_members"
    " WHERE relation_id=?"
    " ORDER BY member_order", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, relation_id);
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("relation %" PRId64 " member %s %15" PRId64 " %s\n", relation_id,
             (char *)sqlite3_column_text(stmt, 0), 
             (int64_t)sqlite3_column_int64(stmt, 1),
             (char *)sqlite3_column_text(stmt, 2) );
  }
  sqlite3_finalize(stmt);
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
  int64_t way_id, node_id;
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file %s: %s", html_file, strerror(errno));
    return;
  }
  leaflet_html_header(html, "map addr");
  fprintf(html,
    "<h3>Map 1 - Address</h3>\n"
    "<div id='map' style='width:100%%; height:500px;'></div>\n");
  fprintf(html, "<script>\n");
  leaflet_init(html, "map", lon1, lat1, lon2, lat2);
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
  /* show boundingbox */
  leaflet_style(html, "#000000", 0.3, 2, "5 5", "none", 0.3, 5);
  leaflet_rectangle(html, "map", lon1, lat1, lon2, lat2, "");
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
    fprintf(html, "<tr>");
    way_id = (int64_t)sqlite3_column_int64(stmt_addr, 0);
    if( way_id!=-1 ){
      fprintf(html, "<td><a href='https://www.openstreetmap.org/way/%" PRId64 "'"
                    " target='_blank'>%" PRId64 "</a></td>", way_id, way_id);
    } else {
      fprintf(html, "<td>%" PRId64 "</td>", way_id);
    }
    node_id = (int64_t)sqlite3_column_int64(stmt_addr, 1);
    if( node_id!=-1 ){
      fprintf(html, "<td><a href='https://www.openstreetmap.org/node/%" PRId64 "'"
                    " target='_blank'>%" PRId64 "</a></td>", node_id, node_id);
    } else {
      fprintf(html, "<td>%" PRId64 "</td>", node_id);
    }
    fprintf(html,
        "<td>%s</td><td>%s</td><td>%s</td><td>%s</td>"
        "<td>%.7f</td><td>%.7f</td></tr>\n",
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

/*
** Creates visualization of the table graph
*/
void write_graph(
  sqlite3 *db,
  FILE *html,
  const char *mapid,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const int mask_permit
){
  sqlite3_stmt *stmt_nodes, *stmt_edges;
  int directed;
  char popuptext[200];
  int64_t node_id, way_id, start_node_id, end_node_id;
  double lon, lat;
  /*  */
  create_subgraph_tables(db, lon1, lat1, lon2, lat2, mask_permit);
  /* show graph nodes */
  leaflet_style(html, "none", 0.9, 2, "", "#ff5348", 0.5, 5);
  rc = sqlite3_prepare_v2(db,
    "SELECT node_id,lon,lat FROM subgraph_nodes",
     -1, &stmt_nodes, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  while( sqlite3_step(stmt_nodes)==SQLITE_ROW ){
    node_id = (int64_t)sqlite3_column_int64(stmt_nodes, 0);
    lon = (double)sqlite3_column_double(stmt_nodes, 1);
    lat = (double)sqlite3_column_double(stmt_nodes, 2);
    snprintf(popuptext, sizeof(popuptext), "node_id %" PRId64, node_id);
    leaflet_circlemarker(html, mapid, lon, lat, popuptext);
  }
  /* show graph edges */
  NodeList nodelist;
  nodelist_init(&nodelist);
  leaflet_style(html, "#0000ff", 0.5, 3, "", "none", 1.0, 5);
  rc = sqlite3_prepare_v2(db,
    "SELECT start_node_id,end_node_id,way_id,directed FROM subgraph",
     -1, &stmt_edges, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  while( sqlite3_step(stmt_edges)==SQLITE_ROW ){
    start_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 0);
    end_node_id = (int64_t)sqlite3_column_int64(stmt_edges, 1);
    way_id = (int64_t)sqlite3_column_int64(stmt_edges, 2);
    directed = (int)sqlite3_column_int(stmt_edges, 3);
    edge_points(db, way_id, start_node_id, end_node_id, &nodelist);
    snprintf(popuptext, sizeof(popuptext), "way_id %" PRId64, way_id);
    if( directed ){
      leaflet_style(html, "#0000ff", 0.5, 3, "5 5", "none", 1.0, 5);
      leaflet_polyline(html, mapid, &nodelist, popuptext);
      leaflet_style(html, "#0000ff", 0.5, 3, "", "none", 1.0, 5);
    }else{
      leaflet_polyline(html, mapid, &nodelist, popuptext);
    }
  }
  /* show boundingbox */
  leaflet_style(html, "#000000", 0.3, 2, "5 5", "none", 0.3, 5);
  leaflet_rectangle(html, mapid, lon1, lat1, lon2, lat2, "");
  /*  */
  nodelist_free(&nodelist);
  sqlite3_finalize(stmt_nodes);
  sqlite3_finalize(stmt_edges);
}

void html_map_graph(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
){
  FILE *html;
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file %s: %s", html_file, strerror(errno));
    return;
  }
  leaflet_html_header(html, "map graph");
  fprintf(html,
    "<h3>Map 1 - Graph complete</h3>\n"
    "<div id='map1' style='width:100%%; height:500px;'></div>\n"
    "<h3>Map 2 - Graph foot</h3>\n"
    "<div id='map2' style='width:100%%; height:500px;'></div>\n"
    "<h3>Map 3 - Graph bike</h3>\n"
    "<div id='map3' style='width:100%%; height:500px;'></div>\n"
    "<h3>Map 4 - Graph car</h3>\n"
    "<div id='map4' style='width:100%%; height:500px;'></div>\n");
  fprintf(html, "<script>\n");
  leaflet_init(html, "map1", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map2", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map3", lon1, lat1, lon2, lat2);
  leaflet_init(html, "map4", lon1, lat1, lon2, lat2);
  write_graph(db, html, "map1", lon1, lat1, lon2, lat2, 0);  /* graph complete */
  write_graph(db, html, "map2", lon1, lat1, lon2, lat2, 1);  /* graph foot */
  write_graph(db, html, "map3", lon1, lat1, lon2, lat2, 2);  /* graph bike */
  write_graph(db, html, "map4", lon1, lat1, lon2, lat2, 4);  /* graph car */
  fprintf(html,
      "</script>\n"
      "<p>dashed line -> one way</p>\n"
      "<hr>\n"
      "<p>Boundingbox: %f %f - %f %f</p>\n", lon1, lat1, lon2, lat2);
  leaflet_html_footer(html);
  if( fclose(html)!=0 ) {
    printf("Error closing file %s: %s", html_file, strerror(errno));
  }
}
