/*
** Visualize the data with Leaflet.js
*/
#include "pbf2sqlite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* TODO dummy function */
point* generate_pointlist(int n) {
  point *pointlist = malloc(n * sizeof(point));
  if( !pointlist ){
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }
  pointlist[0].lon = 7.8425217;
  pointlist[0].lat = 47.9857186;
  pointlist[1].lon = 7.8564262;
  pointlist[1].lat = 47.9892802;
  pointlist[2].lon = 7.8434658;
  pointlist[2].lat = 47.9933011;
  pointlist[3].lon = 7.8559113;
  pointlist[3].lat = 47.9961730;
  pointlist[0].no = 4;
  return pointlist;
}


/*
** Creates visualization of the table graph TODO
*/
int html_graph(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *html_file
){
  FILE *html;
  html = fopen(html_file, "w");
  if( html==NULL ) {
    printf("Error opening file: %s", strerror(errno));
    return EXIT_FAILURE;
  }
  /* Write text to the file */
  leaflet_html_header(html);
  fprintf(html,
    "<h2>Map 1</h2>\n"
    "<div id=\"map1\"></div>\n"
    "<h2>Map 2</h2>\n"
    "<div id=\"map2\"></div>\n"
    "<h2>Map 3</h2>\n"
    "<div id=\"map3\"></div>\n");
  fprintf(html, "<script>\n");
  /* map1 */
  leaflet_init(html, "map1", lon1, lat1, lon2, lat2);
  leaflet_marker(html, "map1", 7.85, 47.99, "");
  leaflet_marker(html, "map1", 7.852, 47.992, "Mit Text...");
  leaflet_circle(html, "map1", 7.852, 47.983, 150, "Hello I'm a circle");
  leaflet_circlemarker(html, "map1", 7.852, 47.985, "I'm a circlemarker");
  leaflet_rectangle(html, "map1", 7.824, 47.983, 7.871, 47.995, "I'm a rectangle");
  leaflet_style(html, "#ff0000", 0.9, 2, "", "none", 1.0);
  leaflet_rectangle(html, "map1", lon1, lat1, lon2, lat2, "Boundingbox");
  point *pointlist = generate_pointlist(5);
  leaflet_polyline(html, "map1", pointlist, "");
  free(pointlist);
  /* map2 */
  leaflet_init(html, "map2", lon1, lat1, lon2, lat2);
  leaflet_circle(html, "map2", 7.852, 47.983, 150, "Hello I'm a circle on map2");
  /* map3 */
  leaflet_init(html, "map3", lon1, lat1, lon2, lat2);
  fprintf(html, "</script>\n");
  leaflet_html_footer(html);
  /* Close the file */
  if( fclose(html)!=0 ) {
    printf("Error closing file: %s", strerror(errno));
    return EXIT_FAILURE;
  }
  printf("File %s written successfully.\n", html_file);
  return EXIT_SUCCESS;
}



/*
** Functions for creating an HTML file with Leaflet.js
*/

void leaflet_html_header(FILE *html) {
  fprintf(html,
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "  <title>Multiple Leaflet Maps</title>\n"
    "  <meta charset=\"utf-8\" />\n"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "  <link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.css\" integrity=\"sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=\" crossorigin=\"\"/>\n"
    "  <script src=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.js\" integrity=\"sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=\" crossorigin=\"\"></script>\n"
    "  <style>\n"
    "  .leaflet-container {\n"
    "    height: 300px; /* must set height for Leaflet maps */\n"
    "    margin-bottom: 20px;\n"
    "  }\n"
    "  </style>\n"
    "</head>\n"
    "<body>\n"
  );
}

void leaflet_html_footer(FILE *html) {
  fprintf(html, "</body>\n</html>\n");
}

void leaflet_init(
  FILE *html,
  const char *mapid,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2
){
  fprintf(html, "// %s init\n", mapid);
  fprintf(html, "const %s = L.map('%s').fitBounds([ [%.7f, %.7f], [%.7f, %.7f] ], "
                "{padding: [0,0], maxZoom: 19});\n",
                 mapid, mapid, lat1, lon1, lat2, lon2);
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

void leaflet_marker(
  FILE *html,
  const char *mapid,
  const double lon,
  const double lat,
  const char *text
){
  fprintf(html, "L.marker([%.7f, %.7f]).addTo(%s)", lat, lon, mapid);
  if( text[0]!='\0' ) fprintf(html, ".bindPopup(\"%s\")", text);
  fprintf(html, ";\n");
}

void leaflet_polyline(
  FILE *html,
  const char *mapid,
  point *pointlist,
  const char *text
){
  fprintf(html, "L.polyline( [\n");
  for(int i=0; i<pointlist[0].no; i++){
    if( i>0 ) fprintf(html, ",\n");
    fprintf(html, "[%.7f, %.7f]", pointlist[i].lat, pointlist[i].lon);
  }
  fprintf(html, " ], style).addTo(%s)",  mapid);
  if( text[0]!='\0' ) fprintf(html, ".bindPopup(\"%s\")", text);
  fprintf(html, ";\n");
}

void leaflet_circle(
  FILE *html,
  const char *mapid,
  const double lon,
  const double lat,
  const int radius,
  const char *text
){
  fprintf(html, "L.circle([%.7f, %.7f], %d, style).addTo(%s)", lat, lon, radius, mapid);
  if( text[0]!='\0' ) fprintf(html, ".bindPopup(\"%s\")", text);
  fprintf(html, ";\n");
}

void leaflet_circlemarker(
  FILE *html,
  const char *mapid,
  const double lon,
  const double lat,
  const char *text
){
  fprintf(html, "L.circleMarker([%.7f, %.7f], style).addTo(%s)", lat, lon,  mapid);
  if( text[0]!='\0' ) fprintf(html, ".bindPopup(\"%s\")", text);
  fprintf(html, ";\n");
}

void leaflet_rectangle(
  FILE *html,
  const char *mapid,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const char *text
){
  fprintf(html, "L.rectangle([[%.7f, %.7f], [%.7f, %.7f]], style).addTo(%s)", lat1, lon1, lat2, lon2, mapid);
  if( text[0]!='\0' ) fprintf(html, ".bindPopup(\"%s\")", text);
  fprintf(html, ";\n");
}

void leaflet_style(
  FILE *html,
  const char *color,
  const double opacity,
  const int weight,
  const char *dasharray,
  const char *fillcolor,
  const double fillopacity
){
  fprintf(html,
    "style = { color:'%s', opacity:%f, weight:%d, dashArray:'%s', fillColor:'%s', "
    "fillOpacity:%f };\n", color, opacity, weight, dasharray, fillcolor, fillopacity);
}

