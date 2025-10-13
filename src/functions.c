/*
**
*/
#include "pbf2sqlite.h"

#include <math.h>

/* Conversion degree to radians */
double radians(double deg) {
  return deg * (M_PI/180.0);
}

double degrees(double rad) {
  return rad * (180.0/M_PI);
}

double mercator_x(double lon) {
  const double r = 6378137.0;
  return r * radians(lon);
}

double mercator_y(double lat) {
  const double r = 6378137.0;
  return r * log(tan(M_PI / 4 + radians(lat) / 2));
}



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
  sqlite3_create_function(db, "mercator_x", 1, SQLITE_UTF8, NULL, mercator_x_func, NULL, NULL);
  sqlite3_create_function(db, "mercator_y", 1, SQLITE_UTF8, NULL, mercator_y_func, NULL, NULL);
}
