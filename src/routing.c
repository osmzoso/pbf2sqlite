/*
** TODO: deprecated function, will be replaced by resize_boundingbox()
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

/**
 * \brief Resize boundingbox
 */
bbox resize_boundingbox(const bbox b, const double enlarge){
  bbox b_new;
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
  b_new.min_lon = mp_lon - diff;
  b_new.min_lat = mp_lat - diff;
  b_new.max_lon = mp_lon + diff;
  b_new.max_lat = mp_lat + diff;
  return b_new;
}


/*
** Write the path coordinates to CSV and GPX files
**
** https://en.wikipedia.org/wiki/Comma-separated_values
** https://en.wikipedia.org/wiki/GPS_Exchange_Format
**
*/
void write_file_csv(
  const char *name,
  const NodeList *list
){
  FILE *csv;
  char *ext = ".csv";
  char *filename = malloc(strlen(name) + strlen(ext) + 1);
  if (!filename) abort_msg("Out of memory\n");
  strcpy(filename, name);
  strcat(filename, ext);
  csv = fopen(filename, "w");
  if( csv==NULL ) abort_msg("Error opening file\n");
  fprintf(csv, "lon,lat,ele,node_id\r\n");
  /* Write the list in reverse order */
  for (int i=list->size-1; i>=0; i--) {
    fprintf(csv, "%.7f,%.7f,0,%" PRId64 "\r\n", list->node[i].lon, list->node[i].lat, list->node[i].node_id);
  }
  if( fclose(csv)!=0 ) abort_msg("Error closing file\n");
  free(filename);
}

void write_file_gpx(
  const char *name,
  const NodeList *list
){
  FILE *gpx;
  char *ext = ".gpx";
  char *filename = malloc(strlen(name) + strlen(ext) + 1);
  if (!filename) abort_msg("Out of memory\n");
  strcpy(filename, name);
  strcat(filename, ext);
  gpx = fopen(filename, "w");
  if( gpx==NULL ) abort_msg("Error opening file\n");
  fprintf(gpx,
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
    "<gpx version=\"1.1\" xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"pbf2sqlite\">\n"
    "  <metadata></metadata>\n"
    "  <trk>\n"
    "    <name>Track</name>\n"
    "    <extensions>\n"
    "    </extensions>\n"
    "    <trkseg>\n"
  );
  /* Write the list in reverse order */
  for (int i=list->size-1; i>=0; i--) {
    fprintf(gpx, "      <trkpt lat=\"%.7f\" lon=\"%.7f\">\n", list->node[i].lat, list->node[i].lon);
    fprintf(gpx, "        <ele>0.0</ele>\n      </trkpt>\n");
  }
  fprintf(gpx,
    "    </trkseg>\n"
    "  </trk>\n"
    "</gpx>"
  );
  if( fclose(gpx)!=0 ) abort_msg("Error closing file\n");
  free(filename);
}

/*
** Transforms a string into a permit mask
*/
int permit_mask(const char *permit){
  int mask_permit;
  if     ( strcmp("foot", permit)==0 ) mask_permit = 1;
  else if( strcmp("bike", permit)==0 ) mask_permit = 2;
  else if( strcmp("car",  permit)==0 ) mask_permit = 4;
  else mask_permit = atoi(permit);
  return mask_permit;
}

/*
** TODO: deprecated function, will be replaced by route()
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
  if (!filename) abort_msg("Out of memory\n");
  strcpy(filename, name);
  strcat(filename, ext);
  /* 1. Get permit mask */
  int mask_permit = permit_mask(permit);
  /* 2. Get boundingbox for the subgraph */
  bbox b = calc_boundingbox(lon_start, lat_start, lon_dest, lat_dest, 2.0);
  /* 3. Get subgraph */
  int number_nodes = create_subgraph_tables_v1(db, b.min_lon, b.min_lat, b.max_lon, b.max_lat, mask_permit);
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
  /* 6. Check whether valid start and end nodes were found */
  if( graph_node_start==-1 ) abort_msg("route - Start coordinates out of range\n");
  if( graph_node_end==-1 ) abort_msg("route - Destination coordinates out of range\n");
  /* 7. Open HTML file */
  html = fopen(filename, "w");
  if( html==NULL ) abort_msg("Error opening file\n");
  leaflet_html_header(html, "map route");
  fprintf(html, "<h3>Route</h3>\n<pre>\n");
  fprintf(html, "# start: %f %f   dest: %f %f\n", lon_start, lat_start, lon_dest, lat_dest);
  fprintf(html, "# permit: %s -> mask_permit: %d\n", permit, mask_permit);
  fprintf(html, "# boundingbox: %f %f   %f %f\n", b.min_lon, b.min_lat, b.max_lon, b.max_lat);
  fprintf(html, "# graph number nodes : %d\n", number_nodes);
  fprintf(html, "# graph node_start   : %d (OSM node_id %" PRId64 ")\n", graph_node_start, node_id_start);
  fprintf(html, "# graph node_end     : %d (OSM node_id %" PRId64 ")\n", graph_node_end, node_id_end);
  /* 8. Routing */
  Dijkstra(graph, graph_node_start, graph_node_end);
  fprintf(html, "# distance: %d m\n", node[graph_node_end].d);
  fprintf(html, "</pre>\n");
  /* 9. Output the coordinates of the path */
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
      "SELECT way_id,start_node_id,end_node_id FROM graph_edges WHERE edge_id=?", -1, &stmt, NULL);
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
  leaflet_style(html, "#0000ff", 0.5, 6, "", "none", 1.0, 5);
  leaflet_polyline(html, "map", &path, "Shortest way");
  /* start and dest points */
  leaflet_style(html, "none", 0.9, 2, "", "#ff0000", 1.0, 5);
  leaflet_circlemarker(html, "map", lon_start, lat_start, "Start");
  leaflet_style(html, "none", 0.9, 2, "", "#00ff00", 1.0, 5);
  leaflet_circlemarker(html, "map", lon_dest, lat_dest, "Dest");
  fprintf(html, "</script>\n");
  leaflet_html_footer(html);
  if( fclose(html)!=0 ) abort_msg("Error closing file\n");
  /* Write path coordinates to CSV and GPX files */
  write_file_csv(name, &path);
  write_file_gpx(name, &path);
  /* 10. Cleanup */
  free(filename);
  nodelist_free(&path);
  nodelist_free(&edge);
  destroyGraph(graph);
  destroyDijkstra();
}

/**
 * \brief Calculate shortest way
 * Output is a HTML file with a map of the route
 */
void route(
  sqlite3 *db,
  int argc,
  char *argv[]
){
  bbox bp, b;
  NodeList route_points;
  int i;
  sqlite3_stmt *stmt;
  FILE *html;
  char *ext = ".html";
  char buffer[30];
  char *filename;
  /* generate HTML filename */
  filename = malloc(strlen(argv[argc-1]) + strlen(ext) + 1);
  if (!filename) abort_msg("Out of memory\n");
  strcpy(filename, argv[argc-1]);
  strcat(filename, ext);
  /* Number of parameters must be even */
  if( argc % 2 == 0 ) abort_msg("Option route2: Incorrect number of parameters\n");
  /* 1. Read coordinates of the start, intermediate, and end points. Adjust boundingbox */
  nodelist_init(&route_points);
  bp.min_lon =  180;
  bp.min_lat =   90;
  bp.max_lon = -180;
  bp.max_lat =  -90;
  for (int i = 4; i < argc-1; i=i+2) {
    double lon = get_argv_double(argv, i);
    double lat = get_argv_double(argv, i+1);
    nodelist_add(&route_points, lon, lat, 0);
    if( bp.min_lon > lon ) bp.min_lon = lon;
    if( bp.min_lat > lat ) bp.min_lat = lat;
    if( bp.max_lon < lon ) bp.max_lon = lon;
    if( bp.max_lat < lat ) bp.max_lat = lat;
  }
  /* 2. Get permit mask */
  int mask_permit = permit_mask(argv[3]);
  /* 3. Resized boundingbox for the subgraph */
  b = resize_boundingbox(bp, 2.0);
  /* 4. Create subgraph tables */
  int number_nodes = create_subgraph_tables(db, b, mask_permit);
  /* 5. Get nearest node in the subgraph */
  for (i = 0; i < route_points.size; i++) {
    int64_t no = subgraph_nearest_node(db, route_points.node[i].lon, route_points.node[i].lat);
    if( no == -1 ) abort_msg("Option route2: Coordinates out of range\n");
    route_points.node[i].node_id = no;
  }
  /* 6. Fill adjacency list */
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
  /* 7. Routing */
  sqlite3_stmt *stmt_insert_path_edges;   /* TODO */
  rc = sqlite3_exec(db,
    " DROP TABLE IF EXISTS path_edges;"
    " CREATE TEMP TABLE path_edges ("
    "  section  INTEGER,"
    "  sequence INTEGER,"
    "  edge_id  INTEGER"
    " );",
     NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
         "INSERT INTO path_edges (section, sequence, edge_id) VALUES (?1,?2,?3)",
         -1, &stmt_insert_path_edges, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  NodeList xpath;                   /* contain all points of the result TODO */
  nodelist_init(&xpath);            /* TODO */
  int distance = 0;             /* TODO */
  for (i = 0; i < route_points.size-1; i++) {
    printf("dijkstra: subgraph_node %8" PRId64 " (node_id: %15" PRId64 ") -> subgraph_node %8" PRId64 " (node_id: %15" PRId64 ")\n",
        route_points.node[i].node_id, subgraph_node_id(db, route_points.node[i].node_id),
        route_points.node[i+1].node_id, subgraph_node_id(db, route_points.node[i+1].node_id) );
    Dijkstra(graph, route_points.node[i].node_id, route_points.node[i+1].node_id);
    distance = distance + node[route_points.node[i+1].node_id].d;
    /* Get the edges from the shortest path */
    int64_t v = route_points.node[i+1].node_id;
    int64_t edge_id;
    int sequence = 0;
    while ( node[v].v_edge != 0 ) {
      edge_id = node[v].v_edge;
      sqlite3_bind_int64(stmt_insert_path_edges, 1, i);
      sqlite3_bind_int64(stmt_insert_path_edges, 2, sequence);
      sqlite3_bind_int64(stmt_insert_path_edges, 3, edge_id);
      rc = sqlite3_step(stmt_insert_path_edges);
      if( rc==SQLITE_DONE ) {
        sqlite3_reset(stmt_insert_path_edges);
      } else {
        abort_db_error(db, rc);
      }
      sequence++;
      /* get previous node of the shortest way */
      v = node[v].v_node;
    }
    destroyDijkstra();
  }
  sqlite3_finalize(stmt_insert_path_edges);
  /* TEST */
  int64_t first_node_id;            /* TODO */
  first_node_id = subgraph_node_id(db, route_points.node[0].node_id);
  printf("first_node_id: %" PRId64 "\n", first_node_id);
  /* TEST */
  int64_t way_id, start_node_id, end_node_id;
  rc = sqlite3_prepare_v2(db,
    " SELECT pe.section,pe.sequence,pe.edge_id,ge.way_id,ge.start_node_id,ge.end_node_id,ge.dist"
    " FROM path_edges AS pe"
    " LEFT JOIN graph_edges AS ge ON pe.edge_id=ge.edge_id"
    " ORDER BY pe.section,pe.sequence DESC", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  printf("+---------+----------+-----------------+-----------------+-----------------+-----------------+---------+\n"
         "| section | sequence |     edge_id     |      way_id     |  start_node_id  |   end_node_id   |   dist  |\n"
         "+---------+----------+-----------------+-----------------+-----------------+-----------------+---------+\n");
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    printf("| %7" PRId64 " | %8" PRId64 " | %15" PRId64 " | %15" PRId64 " | %15" PRId64 " | %15" PRId64 " | %7" PRId64 " |\n",
             (int64_t)sqlite3_column_int64(stmt, 0),
             (int64_t)sqlite3_column_int64(stmt, 1),
             (int64_t)sqlite3_column_int64(stmt, 2),
             (int64_t)sqlite3_column_int64(stmt, 3),
             (int64_t)sqlite3_column_int64(stmt, 4),
             (int64_t)sqlite3_column_int64(stmt, 5),
             (int64_t)sqlite3_column_int64(stmt, 6)
          );
    way_id = (int64_t)sqlite3_column_int64(stmt, 3);
    start_node_id = (int64_t)sqlite3_column_int64(stmt, 4);
    end_node_id = (int64_t)sqlite3_column_int64(stmt, 5);
    /* Determination of the points on an edge, observing the direction */
    if( first_node_id==start_node_id ) {
      edge_points_v2(db, way_id, start_node_id, end_node_id, &xpath);
      first_node_id = end_node_id;
    }else{
      edge_points_v2(db, way_id, end_node_id, start_node_id, &xpath);
      first_node_id = start_node_id;
    }
  }
  printf("+---------+----------+-----------------+-----------------+-----------------+-----------------+---------+\n");
  nodelist_show(&xpath);
  sqlite3_finalize(stmt);
  /* 8. Open HTML file */
  html = fopen(filename, "w");
  if( html==NULL ) abort_msg("Error opening file\n");
  leaflet_html_header(html, "map route2");
  fprintf(html, "<h3>Route</h3>\n<pre>\n");
  fprintf(html, "# permit: %s -> mask_permit: %d\n", argv[3], mask_permit);
  fprintf(html, "# boundingbox: %f %f - %f %f\n", b.min_lon, b.min_lat, b.max_lon, b.max_lat);
  fprintf(html, "# graph number nodes: %d\n", number_nodes);
  for (i = 0; i < route_points.size; i++) {
    fprintf(html, "#  %f %f -> graph node %" PRId64 "\n", route_points.node[i].lon, route_points.node[i].lat, route_points.node[i].node_id);
  }
  fprintf(html, "# distance: %d m\n", distance);
  fprintf(html, "</pre>\n");
  fprintf(html, "<div id='map' style='width:100%%; height:500px;'></div>\n");            /* Show map */
  fprintf(html, "<script>\n");
  leaflet_init(html, "map", b.min_lon, b.min_lat, b.max_lon, b.max_lat);
  leaflet_style(html, "#000000", 0.3, 2, "5 5", "none", 0.3, 5);                         /* boundingbox */
  leaflet_rectangle(html, "map", b.min_lon, b.min_lat, b.max_lon, b.max_lat, "");
  for (i = 0; i < route_points.size; i++) {                                              /* marker route points */
    snprintf(buffer, sizeof(buffer), "Point %d", i+1);
    leaflet_marker(html, "map", route_points.node[i].lon, route_points.node[i].lat, buffer);
  }
  leaflet_style(html, "#0000ff", 0.5, 6, "", "none", 1.0, 5);                            /* path */
  leaflet_polyline(html, "map", &xpath, "Shortest way");
  fprintf(html, "</script>\n");
  leaflet_html_footer(html);
  if( fclose(html)!=0 ) abort_msg("Error closing file\n");

  /* TODO */

  /* 10. Cleanup */
  free(filename);
  nodelist_free(&xpath);
  nodelist_free(&route_points);
  destroyGraph(graph);
}
