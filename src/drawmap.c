/**
 * \file drawmap.c
 * \brief Contains functions for drawing a map
 */

/**
 * \brief Defines a point (x/y or lon/lat), coordinates are float
 */
typedef struct {
  union {
    double x;
    double lon;
  };
  union {
    double y;
    double lat;
  };
} Point;

typedef struct {
  int64_t x;
  int64_t y;
} IntPoint;

/**
 * \brief Calculates the size of a square world map in pixels for a given zoom level (tile: 256x256 pixel)
 */
int64_t size_world_map(const int zoomlevel){
  /* int64_t world_map_size = pow(2, zoomlevel) * 256; TODO */
  int64_t tiles = (int64_t)1 << zoomlevel;
  int64_t world_map_size = tiles * 256;
  return world_map_size;
}

/**
 * \brief Converts spherical coordinates into planar coordinates
 */
Point spherical_to_mercator(const Point spherical){
  Point mercator;
  const double r = 6378137.0;
  mercator.x = r * radians(spherical.lon);
  mercator.y = r * log(tan(M_PI / 4 + radians(spherical.lat) / 2));
  return mercator;
}

/**
 * \brief Converts planar coordinates into spherical coordinates
 */
Point mercator_to_spherical(const Point mercator){
  Point spherical;
  const double r = 6378137.0;
  spherical.lon = degrees(mercator.x / r);
  spherical.lat = degrees(2 * atan(exp(mercator.y / r)) - M_PI / 2.0);
  return spherical;
}


/**
 * Test
 */
void test_spherical_to_mercator(Point s){
  Point m = spherical_to_mercator(s);
  printf(" lon: %13.8f lat: %13.8f  ->  x: %19.9f  y: %19.9f\n", s.lon, s.lat, m.x, m.y);
}

void test_mercator_to_spherical(Point m){
  Point s = mercator_to_spherical(m);
  printf(" x: %19.9f y: %19.9f  ->  lon: %13.8f  lat: %13.8f\n", m.x, m.y, s.lon, s.lat);
}

void test_proj(){
  printf("Test size_world_map():\n");
  for(int i=0; i<=20; i++){
    int64_t world_map_size = size_world_map(i);
    printf("zoomlevel: %3d  world_map_size: %10" PRId64 "\n", i, world_map_size);
  }
  printf("Test spherical_to_mercator():\n");
  test_spherical_to_mercator( (Point){ {.lon =  180.00}, {.lat =  85.05112878}} );
  test_spherical_to_mercator( (Point){ {.lon = -180.00}, {.lat = -85.05112878}} );
  test_spherical_to_mercator( (Point){ {.lon =    0.00}, {.lat =   0.00}} );
  test_spherical_to_mercator( (Point){ {.lon =    7.81}, {.lat =  48.00}} );
  printf("Test mercator_to_spherical():\n");
  test_mercator_to_spherical( (Point){ {.x =  20037508.343}, {.y =  20037508.343} });
  test_mercator_to_spherical( (Point){ {.x = -20037508.343}, {.y = -20037508.343} });
  test_mercator_to_spherical( (Point){ {.x =         0.000}, {.y =         0.000} });
}

void test_matrix(){
  printf("\nTest matrix:\n");

  Matrix *z = matrix_zeros(3, 5);
  matrix_display(z, "Z");
  matrix_free(z);

  Matrix *o = matrix_ones(3, 5);
  matrix_display(o, "O");
  matrix_free(o);

  Matrix *e = matrix_identity(5);
  matrix_display(e, "E");
  matrix_free(e);

  const double values_a[] = {
    3, 2, 1,
    1, 0, 2 };
  Matrix *a = matrix_init(2, 3, values_a);
  matrix_display(a, "A");

  const double values_b[] = {
    1, 2,
    0, 1,
    4, 0 };
  Matrix *b = matrix_init(3, 2, values_b);
  matrix_display(b, "B");

  Matrix *c = matrix_multiply(a, b);
  matrix_display(c, "C = A ⋅ B");

  matrix_free(a);
  matrix_free(b);
  matrix_free(c);
}

/**
 * \brief Creates a map. The output format is SVG.
 */
void map(
  sqlite3 *db,
  const double lon,
  const double lat,
  const int zoomlevel,
  const int width,
  const int height,
  const char *svg_file
){
  printf("command 'map' not yet implemented...\n");
  printf(
   "lon       : %f\n"
   "lat       : %f\n"
   "zoomlevel : %d\n"
   "width     : %d\n"
   "height    : %d\n"
   "svg_file  : %s\n",
    lon, lat, zoomlevel, width, height, svg_file);
  test_proj();
  test_matrix();
  /* Test SVG */
  FILE *svg;
  svg = svgOpen(svg_file, width, height);
  svgLine(svg, 20, 20, 300, 300);
  svgClose(svg);
}
