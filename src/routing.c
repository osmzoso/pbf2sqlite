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
**
*/
void shortest_way(
  sqlite3 *db,
  const double lon_start,
  const double lat_start,
  const double lon_dest,
  const double lat_dest,
  const char *permit,
  const char *filename
){
  sqlite3_stmt *stmt;
  printf("start: %f %f dest: %f %f\n", lon_start, lat_start, lon_dest, lat_dest);
  /* 1. Get permit mask */
  int mask_permit;
  if     ( strcmp("foot", permit)==0 ) mask_permit = 1;
  else if( strcmp("bike", permit)==0 ) mask_permit = 2; 
  else if( strcmp("car",  permit)==0 ) mask_permit = 4; 
  else mask_permit = atoi(permit);
  printf("permit: %s -> mask_permit: %d\n", permit, mask_permit);
  /* 2. Get boundingbox for the subgraph */
  bbox b = calc_boundingbox(lon_start, lat_start, lon_dest, lat_dest, 2.0);
  printf("bbox: %f %f %f %f\n", b.min_lon, b.min_lat, b.max_lon, b.max_lat);
  /* 3. Get subgraph */
  int number_nodes = create_subgraph_tables(db, b.min_lon, b.min_lat, b.max_lon, b.max_lat, mask_permit);
  printf("number nodes: %d\n", number_nodes);
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
  //printGraph(graph);
  /* 5. Find the nodes in the graph that are closest to the coordinates of the start point and end point */
  double dist_node_start = DBL_MAX;
  int graph_node_start = -1;
  double dist_node_end = DBL_MAX;
  int graph_node_end = -1;
  int no;
  double lon, lat, dist;
  rc = sqlite3_prepare_v2(db, "SELECT no,lon,lat FROM subgraph_nodes", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    no = sqlite3_column_int64(stmt, 0);
    lon = sqlite3_column_double(stmt, 1);
    lat = sqlite3_column_double(stmt, 2);
    dist = sqrt(pow(lon_start-lon, 2) + pow(lat_start-lat, 2));
    if( dist < dist_node_start ){
      graph_node_start = no;
      dist_node_start = dist;
    }
    dist = sqrt(pow(lon_dest-lon, 2) + pow(lat_dest-lat, 2));
    if( dist < dist_node_end ){
      graph_node_end = no;
      dist_node_end = dist;
    }
  }
  sqlite3_finalize(stmt);
  printf("node_start: %d    node_end: %d\n", graph_node_start, graph_node_end);
  /* 6. Routing */
  Dijkstra(graph, graph_node_start, graph_node_end);
  // TODO:
  // 7. Output the coordinates of the path
  // 8. Cleanup
  destroyGraph(graph);
  destroyDijkstra();
}

