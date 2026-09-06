/**
 * \file routing.c
 * \brief Contains functions for calculating shortest paths
 */

/**
 * \brief Determine intermediate nodes of a way
 */
void slice_way_nodes(
  sqlite3 *db,
  uint64_t way_id,
  uint64_t start_node_id,
  uint64_t end_node_id,
  NodeList *nl
){
  int n;
  sqlite3_stmt *stmt_points;
  n = 0;
  rc = sqlite3_prepare_v2(db,
    " SELECT n.lon,n.lat,n.node_id"
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
    addNodelist(nl, (double)sqlite3_column_double(stmt_points, 0),
                    (double)sqlite3_column_double(stmt_points, 1),
                    (double)sqlite3_column_int64(stmt_points, 2) );
    n++;
  }
  sqlite3_finalize(stmt_points);
  /*
  ** If no nodes were found then search in the opposite direction
  */
  if( n==0 ){
    rc = sqlite3_prepare_v2(db,
      " SELECT n.lon,n.lat,n.node_id"
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
      addNodelist(nl, (double)sqlite3_column_double(stmt_points, 0),
                      (double)sqlite3_column_double(stmt_points, 1),
                      (double)sqlite3_column_int64(stmt_points, 2) );
      n++;
    }
    sqlite3_finalize(stmt_points);
  }
}

/**
 * \brief Create subgraph
 *
 *  Creates from the table 'graph_edges' the temp. tables 'subgraph' and 'subgraph_nodes' in the database
 *
 * \param boundingbox
 * \param mask_permit
 */
int create_subgraph_tables(
  sqlite3 *db,
  const bbox b,
  const int mask_permit
){
  sqlite3_stmt *stmt_subgraph, *stmt_count;
  int number_of_nodes;
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
    " FROM graph_edges"
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
  sqlite3_bind_double(stmt_subgraph, 5, b.min_lon);
  sqlite3_bind_double(stmt_subgraph, 6, b.max_lon);
  sqlite3_bind_double(stmt_subgraph, 7, b.min_lat);
  sqlite3_bind_double(stmt_subgraph, 8, b.max_lat);
  rc = sqlite3_step(stmt_subgraph);
  if( rc==SQLITE_DONE ){
    sqlite3_reset(stmt_subgraph);
  }else{
    abort_db_error(db, rc);
  }
  sqlite3_finalize(stmt_subgraph);
  rc = sqlite3_exec(db,
    " DROP TABLE IF EXISTS subgraph_nodes;"
    " CREATE TEMP TABLE subgraph_nodes ("
    "  no      INTEGER PRIMARY KEY,"
    "  node_id INTEGER,"
    "  lon     REAL,"
    "  lat     REAL"
    " );"
    " INSERT INTO subgraph_nodes (node_id, lon, lat)"
    " SELECT s.node_id,n.lon,n.lat FROM"
    " ("
    "  SELECT start_node_id AS node_id FROM subgraph"
    "  UNION"
    "  SELECT end_node_id AS node_id FROM subgraph"
    " ) AS s"
    " LEFT JOIN nodes AS n ON s.node_id=n.node_id;",
     NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* count nodes */
  number_of_nodes = 0;
  rc = sqlite3_prepare_v2(db,
    "SELECT max(no) FROM subgraph_nodes", -1, &stmt_count, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_step(stmt_count);
  if( rc==SQLITE_ROW ) number_of_nodes = sqlite3_column_int(stmt_count, 0);
  sqlite3_finalize(stmt_count);
  return number_of_nodes;
}

/**
 * \brief Find the nearest node in the subgraph
 */
int64_t subgraph_nearest_node(
  sqlite3 *db,
  const double lon,
  const double lat
){
  sqlite3_stmt *stmt;
  double min_dist_node = DBL_MAX;
  int64_t graph_node_no, no;
  double graph_node_lon, graph_node_lat, dist;
  no = -1;
  rc = sqlite3_prepare_v2(db, "SELECT no,lon,lat FROM subgraph_nodes", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    graph_node_no = sqlite3_column_int64(stmt, 0);
    graph_node_lon = sqlite3_column_double(stmt, 1);
    graph_node_lat = sqlite3_column_double(stmt, 2);
    dist = sqrt(pow(lon-graph_node_lon, 2) + pow(lat-graph_node_lat, 2));
    if( dist < min_dist_node ){
      no = graph_node_no;
      min_dist_node = dist;
    }
  }
  sqlite3_finalize(stmt);
  return no;
}

/**
 * \brief Get node_id for a subgraph node no
 */
int64_t subgraph_node_id(
  sqlite3 *db,
  const int64_t no
){
  sqlite3_stmt *stmt;
  int64_t node_id = -1;
  rc = sqlite3_prepare_v2(db, "SELECT node_id FROM subgraph_nodes WHERE no=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, no);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    node_id = sqlite3_column_int64(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return node_id;
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

/**
 * \brief Writes a node list to a CSV file
 *
 * https://en.wikipedia.org/wiki/Comma-separated_values
 */
void write_file_csv(
  const char *name,
  const NodeList *list
){
  int i;
  FILE *csv;
  char *ext = ".csv";
  char *filename = malloc(strlen(name) + strlen(ext) + 1);
  if (!filename) abort_msg("Out of memory");
  strcpy(filename, name);
  strcat(filename, ext);
  csv = fopen(filename, "w");
  if( csv==NULL ) abort_msg("Error opening file");
  fprintf(csv, "lon,lat,ele,node_id\r\n");
  for(i=0; i<list->size; i++){
    fprintf(csv, "%.7f,%.7f,0,%" PRId64 "\r\n", list->node[i].lon, list->node[i].lat, list->node[i].node_id);
  }
  if( fclose(csv)!=0 ) abort_msg("Error closing file");
  free(filename);
}

/**
 * \brief Writes a node list to a GPX file
 *
 * https://en.wikipedia.org/wiki/GPS_Exchange_Format
 */
void write_file_gpx(
  const char *name,
  const NodeList *list
){
  int i;
  FILE *gpx;
  char *ext = ".gpx";
  char *filename = malloc(strlen(name) + strlen(ext) + 1);
  if (!filename) abort_msg("Out of memory");
  strcpy(filename, name);
  strcat(filename, ext);
  gpx = fopen(filename, "w");
  if( gpx==NULL ) abort_msg("Error opening file");
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
  for(i=0; i<list->size; i++){
    fprintf(gpx, "      <trkpt lat=\"%.7f\" lon=\"%.7f\">\n", list->node[i].lat, list->node[i].lon);
    fprintf(gpx, "        <ele>0.0</ele>\n      </trkpt>\n");
  }
  fprintf(gpx,
    "    </trkseg>\n"
    "  </trk>\n"
    "</gpx>"
  );
  if( fclose(gpx)!=0 ) abort_msg("Error closing file");
  free(filename);
}

/**
 * \brief Transforms a string into a permit mask
 */
int permit_mask(const char *permit){
  int mask_permit;
  if     ( strcmp("foot", permit)==0 ) mask_permit = 1;
  else if( strcmp("bike", permit)==0 ) mask_permit = 2;
  else if( strcmp("roadbike", permit)==0 ) mask_permit = 10;
  else if( strcmp("car", permit)==0 ) mask_permit = 4;
  else mask_permit = atoi(permit);
  return mask_permit;
}

/**
 * \brief Calculate the shortest path
 *
 * Creates three files
 *  - HTML file with a map of the route
 *  - CSV and GPX files
 *
 * \param ARGV
 */
void route(
  sqlite3 *db,
  int argc,
  char *argv[]
){
  int i;
  int mask_permit;                             /* Permit mask */
  NodeList route_points;                       /* List of all route points */
  char *name;                                  /* Filename without extension */
  double lon, lat;                             /* Coordinates of a route point */
  bbox bp;                                     /* Bounding box of the route points */
  bbox b;                                      /* Enlarged bounding box */
  int number_nodes;                            /* Number of nodes in the subgraph */
  int64_t no;                                  /* Node ID subgraph */
  struct Graph* graph;                         /* Adjacency list */
  sqlite3_stmt *stmt_insert_path_edges;        /* SQLite statement handler */
  NodeList path, path2;                        /* Contains all points of the shortest path */
  int distance;                                /* Distance of the shortest path in meters */
  int64_t v;                                   /* Previous node of the shortest way */
  int sequence;                                /* Contain the sequence of edges */
  int64_t edge_id, way_id, start_node_id, end_node_id;  /* Cache ID */
  sqlite3_stmt *stmt;                          /* SQLite statement handler */
  int64_t first_node_id;                       /* Node ID of the first node in the path */
  FILE *html;                                  /* File pointer HTML file */
  char *ext = ".html";                         /* File extension */
  char buffer[30];                             /* Buffer */
  char *filename;                              /* File name HTML file */
  /* Number of parameters must be even */
  if( argc % 2 == 0 ) abort_msg("Option route: Incorrect number of parameters");
  /* Get permit mask, coordinates of all route points and filename without extension */
  mask_permit = permit_mask(argv[3]);
  initNodelist(&route_points);
  bp.min_lon =  180;    /* start values bounding box */
  bp.min_lat =   90;
  bp.max_lon = -180;
  bp.max_lat =  -90;
  for (i = 4; i < argc-1; i=i+2) {
    lon = get_argv_double(argv, i);
    lat = get_argv_double(argv, i+1);
    addNodelist(&route_points, lon, lat, 0);
    if( bp.min_lon > lon ) bp.min_lon = lon;    /* adjust bounding box */
    if( bp.min_lat > lat ) bp.min_lat = lat;
    if( bp.max_lon < lon ) bp.max_lon = lon;
    if( bp.max_lat < lat ) bp.max_lat = lat;
  }
  name = argv[argc-1];
  /* Enlarge boundingbox for the subgraph, create subgraph tables */
  b = resize_boundingbox(bp, 2.0);
  number_nodes = create_subgraph_tables(db, b, mask_permit);
  /* For all route points get nearest node in the subgraph */
  for (i = 0; i < route_points.size; i++) {
    no = subgraph_nearest_node(db, route_points.node[i].lon, route_points.node[i].lat);
    if( no == -1 ) abort_msg("Option route: Coordinates out of range");
    route_points.node[i].node_id = no;  /* Attention: Inserts Node ID subgraph, not OSM Node ID */
  }
  /* Fill adjacency list */
  graph = createGraph(number_nodes);
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
  /* Routing */
  initNodelist(&path);
  distance = 0;
  rc = sqlite3_exec(db,
    " DROP TABLE IF EXISTS path_edges;"
    " CREATE TEMP TABLE path_edges ("
    "  section  INTEGER,"
    "  sequence INTEGER,"
    "  edge_id  INTEGER"
    " );",
     NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db, "INSERT INTO path_edges (section, sequence, edge_id) VALUES (?1,?2,?3)",
         -1, &stmt_insert_path_edges, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  for (i = 0; i < route_points.size-1; i++) {
#ifdef DEBUG
    printf("dijkstra: %8" PRId64 " -> %8" PRId64 "         (node_id: %15" PRId64 " -> %15" PRId64 ")\n",
        route_points.node[i].node_id, route_points.node[i+1].node_id,
        subgraph_node_id(db, route_points.node[i].node_id), subgraph_node_id(db, route_points.node[i+1].node_id) );
#endif
    Dijkstra(graph, route_points.node[i].node_id, route_points.node[i+1].node_id);
    distance = distance + node[route_points.node[i+1].node_id].d;
    /* Get the edges from the shortest path and store them in table 'path_edges' */
    v = route_points.node[i+1].node_id;
    sequence = 0;
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
    freeDijkstra();
  }
  sqlite3_finalize(stmt_insert_path_edges);
  /* Get all edges in the right order */
  first_node_id = subgraph_node_id(db, route_points.node[0].node_id);
  rc = sqlite3_prepare_v2(db,
    " SELECT pe.section,pe.sequence,pe.edge_id,ge.way_id,ge.start_node_id,ge.end_node_id,ge.dist"
    " FROM path_edges AS pe"
    " LEFT JOIN graph_edges AS ge ON pe.edge_id=ge.edge_id"
    " ORDER BY pe.section,pe.sequence DESC", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
#ifdef DEBUG
  printf("first_node_id: %" PRId64 "\n", first_node_id);
  printf(" section | sequence |     edge_id     |      way_id     |  start_node_id  |   end_node_id   |   dist  \n"
         "---------+----------+-----------------+-----------------+-----------------+-----------------+---------\n");
#endif
  while( sqlite3_step(stmt)==SQLITE_ROW ) {
    way_id = (int64_t)sqlite3_column_int64(stmt, 3);
    start_node_id = (int64_t)sqlite3_column_int64(stmt, 4);
    end_node_id = (int64_t)sqlite3_column_int64(stmt, 5);
    /* Determination of the points on an edge, observing the direction */
    if( first_node_id==start_node_id ) {
      slice_way_nodes(db, way_id, start_node_id, end_node_id, &path);
      first_node_id = end_node_id;
    }else{
      slice_way_nodes(db, way_id, end_node_id, start_node_id, &path);
      first_node_id = start_node_id;
    }
#ifdef DEBUG
    printf(" %7" PRId64 " | %8" PRId64 " | %15" PRId64 " | %15" PRId64 " | %15" PRId64 " | %15" PRId64 " | %7" PRId64 " \n",
             (int64_t)sqlite3_column_int64(stmt, 0),
             (int64_t)sqlite3_column_int64(stmt, 1),
             (int64_t)sqlite3_column_int64(stmt, 2),
             (int64_t)sqlite3_column_int64(stmt, 3),
             (int64_t)sqlite3_column_int64(stmt, 4),
             (int64_t)sqlite3_column_int64(stmt, 5),
             (int64_t)sqlite3_column_int64(stmt, 6)
          );
#endif
  }
  sqlite3_finalize(stmt);
  /*
   * Dirty Hack
   * remove double points in the nodelist
   */
  initNodelist(&path2);
  v = -1;
  for (i = 0; i < path.size; i++) {
    if( v!=path.node[i].node_id ){
      addNodelist(&path2, path.node[i].lon, path.node[i].lat, path.node[i].node_id);
    }
    v = path.node[i].node_id;
  }
#ifdef DEBUG
  printNodelist(&path);
  printNodelist(&path2);
#endif
  /* Create CSV and GPX files with the path coordinates */
  write_file_csv(name, &path2);
  write_file_gpx(name, &path2);
  /* Create HTML file */
  filename = malloc(strlen(argv[argc-1]) + strlen(ext) + 1);
  if (!filename) abort_msg("Out of memory");
  strcpy(filename, argv[argc-1]);
  strcat(filename, ext);
  html = fopen(filename, "w");
  if( html==NULL ) abort_msg("Error opening file");
  leaflet_html_header(html, "map route");
  fprintf(html, "<h1>Route</h1>\n<pre>\n");
  fprintf(html, "# permit: %s (mask_permit: %d)\n", argv[3], mask_permit);
  for (i = 0; i < route_points.size; i++) {
    fprintf(html, "# %d.  %f %f (OSM Node %" PRId64 ")\n",
       i+1, route_points.node[i].lon, route_points.node[i].lat,
       subgraph_node_id(db, route_points.node[i].node_id) );
  }
  fprintf(html, "# route distance: %d m\n", distance);
  fprintf(html, "#\n# boundingbox: %f %f - %f %f\n", b.min_lon, b.min_lat, b.max_lon, b.max_lat);
  fprintf(html, "# graph number nodes: %d\n", number_nodes);
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
  leaflet_polyline(html, "map", &path2, "Shortest way");
  fprintf(html, "</script>\n");
  leaflet_html_footer(html);
  if( fclose(html)!=0 ) abort_msg("Error closing file");
  /* Cleanup */
  free(filename);
  freeNodelist(&path);
  freeNodelist(&path2);
  freeNodelist(&route_points);
  freeGraph(graph);
}
