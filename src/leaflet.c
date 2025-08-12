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
  leaflet_init(html, "map1", lon1, lat1, lon2, lat2);
  /* Close the file */
  if( fclose(html)!=0 ) {
    printf("Error closing file: %s", strerror(errno));
    return EXIT_FAILURE;
  }
  printf("File written successfully.\n");
  return EXIT_SUCCESS;
}

void leaflet_init(
  FILE *html,
  const char* mapid,
  double lon1,
  double lat1,
  double lon2,
  double lat2
){
  fprintf(html, "// %s init\n", mapid);
  fprintf(html, "const %s = L.map('%s').fitBounds([ [%f, %f], [%f, %f] ], "
                "{padding: [0,0], maxZoom: 19});\n",
                 mapid, mapid, lon1, lat1, lon2, lat2);
  fprintf(html, "L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', "
                "{maxZoom:19}).addTo(%s);\n", mapid);
  fprintf(html, "L.control.scale({ position: 'bottomleft', maxWidth: 200, "
                "metric:true, imperial:false }).addTo(%s);\n", mapid);
  fprintf(html, "var %s_popup = L.popup();\n", mapid);
  fprintf(html, "function %s_map_click(e) {\n"
            "  var geo = e.latlng;\n"
            "  var lat = geo.lat;\n"
            "  var lon = geo.lng;\n"
            "  lat = lat.toFixed(7);\n"
            "  lon = lon.toFixed(7);\n"
            "  var popuptext = '<pre>lon(x)    lat(y)<br>'+lon+' '+lat+'<br></pre>';\n"
            "  %s_popup.setLatLng(e.latlng).setContent(popuptext).openOn(%s);\n"
            "}\n", mapid, mapid, mapid);
  fprintf(html, "%s.on('click', %s_map_click);\n", mapid, mapid);
  fprintf(html, "var style = { color:'#0000ff', opacity:0.5, weight:4, "
                "dashArray:'none', fillColor:'#ff7800', fillOpacity:0.5 };\n");
}

