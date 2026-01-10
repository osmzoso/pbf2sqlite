/*
**
*/

typedef struct {
  double min_lon;
  double min_lat;
  double max_lon;
  double max_lat;
} bbox;

/*
** Create a boundingbox from two points.
** A magnification factor must be specified.
*/
bbox calc_boundingbox(
  double lon1,
  double lat1,
  double lon2,
  double lat2,
  double enlarge
){
  bbox b;
  if( lon1<=lon2 ) {
    b.min_lon = lon1;
    b.max_lon = lon2;
  } else {
    b.min_lon = lon2;
    b.max_lon = lon1;
  }
  if( lat1<=lat2 ) {
    b.min_lat = lat1;
    b.max_lat = lat2;
  } else {
    b.min_lat = lat2;
    b.max_lat = lat1;
  }
  double mp_lon = (b.min_lon + b.max_lon) / 2;
  double mp_lat = (b.min_lat + b.max_lat) / 2;
  double diff_mp_lon = mp_lon - b.min_lon;
  double diff_mp_lat = mp_lat - b.min_lat;
  double diff;
  if( diff_mp_lat > diff_mp_lon ) {
    diff = diff_mp_lat * enlarge;
  } else {
    diff = diff_mp_lon * enlarge;
  }
  b.min_lon = mp_lon - diff;
  b.min_lat = mp_lat - diff;
  b.max_lon = mp_lon + diff;
  b.max_lat = mp_lat + diff;
  return b;
}

/*
** Write the path coordinates to a CSV file
*/
void write_file_csv(
  const char *name,
  const NodeList *list
){
  FILE *csv;
  char *ext = ".csv";
  char *filename = malloc(strlen(name) + strlen(ext) + 1);
  if (!filename) abort_oom();
  strcpy(filename, name);
  strcat(filename, ext);
  csv = fopen(filename, "w");
  if( csv==NULL ) abort_fopen();
  /* Write the list in reverse order */
  for (int i=list->size-1; i>=0; i--) {
    fprintf(csv, "%f,%f,0,%" PRId64 "\n", list->node[i].lon, list->node[i].lat, list->node[i].node_id);
  }
  if( fclose(csv)!=0 ) abort_fclose();
  free(filename);
}

/*
** Calculate shortest way
** Output is a HTML file with a map of the route
*/
void shortest_way(
  sqlite3 *db,
  const double lon_start,
  const double lat_start,
  const double lon_dest,
  const double lat_dest,
  const char *permit,
  const char *name
){
  sqlite3_stmt *stmt;
  FILE *html;
  char *ext = ".html";
  char *filename = malloc(strlen(name) + strlen(ext) + 1);
  if (!filename) abort_oom();
  strcpy(filename, name);
  strcat(filename, ext);
  html = fopen(filename, "w");
  if( html==NULL ) abort_fopen();
  leaflet_html_header(html, "map route");
  fprintf(html, "<h3>Routing</h3>\n<pre>\n");
  fprintf(html, "# start: %f %f   dest: %f %f\n", lon_start, lat_start, lon_dest, lat_dest);
  /* 1. Get permit mask */
  int mask_permit;
  if     ( strcmp("foot", permit)==0 ) mask_permit = 1;
  else if( strcmp("bike", permit)==0 ) mask_permit = 2; 
  else if( strcmp("car",  permit)==0 ) mask_permit = 4; 
  else mask_permit = atoi(permit);
  fprintf(html, "# permit: %s -> mask_permit: %d\n", permit, mask_permit);
  /* 2. Get boundingbox for the subgraph */
  bbox b = calc_boundingbox(lon_start, lat_start, lon_dest, lat_dest, 2.0);
  fprintf(html, "# boundingbox: %f %f   %f %f\n", b.min_lon, b.min_lat, b.max_lon, b.max_lat);
  /* 3. Get subgraph */
  int number_nodes = create_subgraph_tables(db, b.min_lon, b.min_lat, b.max_lon, b.max_lat, mask_permit);
  fprintf(html, "# graph number nodes : %d\n", number_nodes);
  /* 4. fill adjacency list */
  struct Graph* graph = createGraph(number_nodes);
  rc = sqlite3_prepare_v2(db,
    " SELECT sns.no,sne.no,s.dist,s.edge_id,s.directed"
    " FROM subgraph AS s"
    " LEFT JOIN subgraph_nodes AS sns ON s.start_node_id=sns.node_id"
    " LEFT JOIN subgraph_nodes AS sne ON s.end_node_id=sne.node_id", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    addEdge(graph, sqlite3_column_int64(stmt, 0),
                   sqlite3_column_int64(stmt, 1),
                   sqlite3_column_int64(stmt, 2),
                   sqlite3_column_int64(stmt, 3),
                   sqlite3_column_int64(stmt, 4));
  }
  sqlite3_finalize(stmt);
  /* 5. Find the nodes in the graph that are closest to the coordinates of the start point and end point */
  double dist_node_start = DBL_MAX;
  int graph_node_start = -1;
  int64_t node_id_start = -1;
  double dist_node_end = DBL_MAX;
  int graph_node_end = -1;
  int64_t node_id_end = -1;
  int no;
  int64_t node_id;
  double lon, lat, dist;
  rc = sqlite3_prepare_v2(db, "SELECT no,node_id,lon,lat FROM subgraph_nodes", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    no = sqlite3_column_int64(stmt, 0);
    node_id = sqlite3_column_int64(stmt, 1);
    lon = sqlite3_column_double(stmt, 2);
    lat = sqlite3_column_double(stmt, 3);
    dist = sqrt(pow(lon_start-lon, 2) + pow(lat_start-lat, 2));
    if( dist < dist_node_start ){
      graph_node_start = no;
      node_id_start = node_id;
      dist_node_start = dist;
    }
    dist = sqrt(pow(lon_dest-lon, 2) + pow(lat_dest-lat, 2));
    if( dist < dist_node_end ){
      graph_node_end = no;
      node_id_end = node_id;
      dist_node_end = dist;
    }
  }
  sqlite3_finalize(stmt);
  fprintf(html, "# graph node_start   : %d (OSM node_id %" PRId64 ")\n", graph_node_start, node_id_start);
  fprintf(html, "# graph node_end     : %d (OSM node_id %" PRId64 ")\n", graph_node_end, node_id_end);
  /* 6. Routing */
  Dijkstra(graph, graph_node_start, graph_node_end);
  fprintf(html, "# distance: %d m\n", node[graph_node_end].d);
  fprintf(html, "</pre>\n");
  /* 7. Output the coordinates of the path */
  NodeList path;          /* contains all points of the path in reverse order */
  NodeList edge;          /* contains all points of an edge */
  nodelist_init(&path);
  nodelist_init(&edge);
  int64_t first_node_id = node_id_end;
  /* get all edges of the path */
  int edge_id;
  int v = graph_node_end;
  while ( node[v].v_edge != 0 ) {
    edge_id = node[v].v_edge;
    /* get all infos of the edge */
    int64_t way_id = 0;
    int64_t start_node_id = 0;
    int64_t end_node_id = 0;
    rc = sqlite3_prepare_v2(db,
      "SELECT way_id,start_node_id,end_node_id FROM graph WHERE edge_id=?", -1, &stmt, NULL);
    if( rc!=SQLITE_OK ) abort_db_error(db, rc);
    sqlite3_bind_int64(stmt, 1, edge_id);
    while( sqlite3_step(stmt)==SQLITE_ROW ){
      way_id = (int64_t)sqlite3_column_int64(stmt, 0);
      start_node_id = (int64_t)sqlite3_column_int64(stmt, 1);
      end_node_id = (int64_t)sqlite3_column_int64(stmt, 2);
    }
    sqlite3_finalize(stmt);
    /* Join edges together to form a continuous path */
    if( first_node_id==start_node_id ) {
      edge_points(db, way_id, start_node_id, end_node_id, &edge);
      first_node_id = end_node_id;
    }else{
      edge_points(db, way_id, end_node_id, start_node_id, &edge);
      first_node_id = start_node_id;
    }
    /* Add all edge points to the path, avoid the last node */
    for (int i=0; i<edge.size-1; i++) {
      nodelist_add(&path, edge.node[i].lon, edge.node[i].lat, edge.node[i].node_id);
    }
    /* get previous node of the path */
    v = node[v].v_node;
  }
  /* Add last point of the last edge to the path */
  nodelist_add(&path, edge.node[edge.size-1].lon, edge.node[edge.size-1].lat, edge.node[edge.size-1].node_id);
  /* Show map */
  fprintf(html, "<div id='map' style='width:100%%; height:500px;'></div>\n");
  fprintf(html, "<script>\n");
  leaflet_init(html, "map", b.min_lon, b.min_lat, b.max_lon, b.max_lat);
  /* boundingbox */
  leaflet_style(html, "#000000", 0.3, 2, "5 5", "none", 0.3, 5);
  leaflet_rectangle(html, "map", b.min_lon, b.min_lat, b.max_lon, b.max_lat, "");
  /* path */
  leaflet_style(html, "#ff0000", 0.5, 6, "", "none", 1.0, 5);
  leaflet_polyline(html, "map", &path, "Shortest way");
  /* start and dest points */
  leaflet_style(html, "none", 0.9, 2, "", "#ff0000", 1.0, 5);
  leaflet_circlemarker(html, "map", lon_start, lat_start, "Start");
  leaflet_style(html, "none", 0.9, 2, "", "#00ff00", 1.0, 5);
  leaflet_circlemarker(html, "map", lon_dest, lat_dest, "Dest");
  fprintf(html, "</script>\n");
  leaflet_html_footer(html);
  if( fclose(html)!=0 ) abort_fclose();
  /* Write path coordinates to a CSV file */
  write_file_csv(name, &path);
  /* 8. Cleanup */
  free(filename);
  nodelist_free(&path);
  nodelist_free(&edge);
  destroyGraph(graph);
  destroyDijkstra();
}

