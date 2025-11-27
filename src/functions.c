/*
**
*/


/* Conversion degree to radians */
double radians(double deg) {
  return deg * (M_PI / 180.0);
}

double degrees(double rad) {
  return rad * (180.0 / M_PI);
}

/* Calculates great circle distance between two coordinates in degrees */
double distance(double lon1, double lat1, double lon2, double lat2) {
  /* Avoid a acos error if the two points are identical */
  if( lon1 == lon2 && lat1 == lat2 ) return 0;
  lon1 = radians(lon1);
  lat1 = radians(lat1);
  lon2 = radians(lon2);
  lat2 = radians(lat2);
  /* Use earth radius Europe 6371 km (alternatively radius equator 6378 km) */
  double dist = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon2 - lon1)) * 6371000;
  return dist;    /* distance in meters */
}

double mercator_x(double lon) {
  const double r = 6378137.0;
  return r * radians(lon);
}

double mercator_y(double lat) {
  const double r = 6378137.0;
  return r * log(tan(M_PI / 4 + radians(lat) / 2));
}

/*
** Register the functions in SQLite
*/
static void radians_func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc == 1 && sqlite3_value_type(argv[0]) == SQLITE_FLOAT) {
    double phi = sqlite3_value_double(argv[0]);
    phi = radians(phi);
    sqlite3_result_double(context, phi);
  } else {
    sqlite3_result_null(context);
  }
}
static void degrees_func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc == 1 && sqlite3_value_type(argv[0]) == SQLITE_FLOAT) {
    double phi = sqlite3_value_double(argv[0]);
    phi = degrees(phi);
    sqlite3_result_double(context, phi);
  } else {
    sqlite3_result_null(context);
  }
}
static void distance_func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc == 4 && sqlite3_value_type(argv[0]) == SQLITE_FLOAT
                && sqlite3_value_type(argv[1]) == SQLITE_FLOAT
                && sqlite3_value_type(argv[2]) == SQLITE_FLOAT
                && sqlite3_value_type(argv[3]) == SQLITE_FLOAT) {
    double lon1 = sqlite3_value_double(argv[0]);
    double lat1 = sqlite3_value_double(argv[1]);
    double lon2 = sqlite3_value_double(argv[2]);
    double lat2 = sqlite3_value_double(argv[3]);
    double dist = distance(lon1, lat1, lon2, lat2);
    sqlite3_result_double(context, dist);
  } else {
    sqlite3_result_null(context);
  }
}
static void mercator_x_func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc == 1 && sqlite3_value_type(argv[0]) == SQLITE_FLOAT) {
    double lon = sqlite3_value_double(argv[0]);
    double x = mercator_x(lon);
    sqlite3_result_double(context, x);
  } else {
    sqlite3_result_null(context);
  }
}
static void mercator_y_func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  if (argc == 1 && sqlite3_value_type(argv[0]) == SQLITE_FLOAT) {
    double lat = sqlite3_value_double(argv[0]);
    double y = mercator_y(lat);
    sqlite3_result_double(context, y);
  } else {
    sqlite3_result_null(context);
  }
}


void register_functions(sqlite3 *db) {
  sqlite3_create_function(db, "radians", 1, SQLITE_UTF8, NULL, radians_func, NULL, NULL);
  sqlite3_create_function(db, "degrees", 1, SQLITE_UTF8, NULL, degrees_func, NULL, NULL);
  sqlite3_create_function(db, "distance", 4, SQLITE_UTF8, NULL, distance_func, NULL, NULL);
  sqlite3_create_function(db, "mercator_x", 1, SQLITE_UTF8, NULL, mercator_x_func, NULL, NULL);
  sqlite3_create_function(db, "mercator_y", 1, SQLITE_UTF8, NULL, mercator_y_func, NULL, NULL);
}
