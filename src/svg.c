/**
 * \file svg.c
 * \brief Contains functions for creatingi an SVG file
 */

/**
 * \brief SVG Open a file
 */
FILE *svgOpen(
  const char *svg_file,
  const int width,
  const int height
){
  FILE *svg;
  svg = fopen(svg_file, "w");
  if( svg==NULL ) abort_msg("Error opening file");
  fprintf(svg, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\">\n", width, height);
  return svg;
}

/**
 * \brief SVG Draw line
 */
void svgLine(
  FILE *svg,
  const int x1,
  const int y1,
  const int x2,
  const int y2
){
  char *stroke = "#ff0000";
  int stroke_width = 18;
  char *stroke_linecap = "round";
  fprintf(svg, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"%s\" "
               "stroke-width=\"%d\" stroke-linecap=\"%s\"/>\n",
                x1, y1, x2, y2, stroke, stroke_width, stroke_linecap);
}

/**
 * \brief SVG Close a file
 */
void svgClose(
  FILE *svg
){
  fprintf(svg, "</svg>\n");
  if( fclose(svg)!=0 ) abort_msg("Error closing file");
}

