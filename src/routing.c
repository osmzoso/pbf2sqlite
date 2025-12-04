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
  printf("start: %f %f dest: %f %f\n", lon_start, lat_start, lon_dest, lat_dest);
  // TODO:
  // 1. Get boundingbox for the subgraph
  // 2. Get subgraph, fill adjacency list
  // 3. Find the nodes in the graph that are closest to the coordinates of the start point and end point
  // 4. Routing
  // 5. Output the coordinates of the path
}

