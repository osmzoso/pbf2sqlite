/*
** Visualize the data with Leaflet.js
*/
#include "pbf2sqlite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
** TODO dummy function
*/
int html_graph(
  sqlite3 *db,
  double lon1,
  double lat1,
  double lon2,
  double lat2,
  const char *html_file
){
  FILE *html;
  printf("Visualize Graph\nBoundingbox: %f %f\n", lon1, lat1);
  printf("HTML file: %s \n", html_file);
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file: %s", strerror(errno));
    return EXIT_FAILURE;
  }
  /* Write text to the file */
  fprintf(html, "Hello, this should be HTML code... TODO\n");
  fprintf(html, "This is the second line of the file.\n");
  /* Close the file */
  if( fclose(html)!=0 ) {
    printf("Error closing file: %s", strerror(errno));
    return EXIT_FAILURE;
  }
  printf("File written successfully.\n");
  return EXIT_SUCCESS;
}

