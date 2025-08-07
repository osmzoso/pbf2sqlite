#!/usr/bin/env python
"""
Generate a map to show the table 'graph'
"""
import sys
import sqlite3


class Leaflet:
    """
    A class used to produce HTML file with Leaflet.js
    """
    def __init__(self, html_filename):
        """Initialize the class attributes:
        Color and size properties (init value):
          color        : hexcolor or 'none'       ('#0000ff')
          opacity      : opacity from 0 to 1      (0.5)
          weight       : line thickness in px     (4)
          dasharray    : examples: '5 5', '2 8 4' ('none')
          fillcolor    : hexcolor or 'none'       ('#ff7800')
          fillopacity  : opacity from 0 to 1      (0.5)
        """
        self.file = open(html_filename, 'w', encoding='utf-8')
        self.bbox = {'min_lon': 180, 'min_lat': 90, 'max_lon': -180, 'max_lat': -90}
        self.p = {
          'color': '#0000ff',
          'opacity': 0.5,
          'weight': 4,
          'dasharray': 'none',
          'fillcolor': '#ff7800',
          'fillopacity': 0.5
        }

    def __del__(self):
        """destructor, close the file"""
        self.file.close()

    def adjust_boundingbox(self, lon, lat):
        """Adjusts boundingbox."""
        self.bbox['min_lon'] = min(self.bbox['min_lon'], lon)
        self.bbox['min_lat'] = min(self.bbox['min_lat'], lat)
        self.bbox['max_lon'] = max(self.bbox['max_lon'], lon)
        self.bbox['max_lat'] = max(self.bbox['max_lat'], lat)

    def lonlat2str(self, lonlat):
        """Converts a list [(lon1,lat1),(lon2,lat2),...] into
        a string with JavaScript array code."""
        latlon_str = ''
        for lon, lat in lonlat:
            if latlon_str != '':
                latlon_str += ',\n'
            latlon_str += '[' + str(lat) + ',' + str(lon) + ']'
            self.adjust_boundingbox(lon, lat)
        return latlon_str

    def set_property(self, properties):
        """Changes the properties as they are specified in the dictionary properties."""
        for k, v in properties.items():
            self.p[k] = v

    def write_html_footer(self):
        """Write HTML footer and close the file"""
        self.file.write('\n</body>\n</html>')
        self.file.close()

    def write_html_header(self, title):
        """Write HTML header with link to Leaflet 1.9.4 and a simple CSS."""
        # https://leafletjs.com/
        self.file.write(f'''<!DOCTYPE html>
<html>
<head>
<title>{title}</title>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<link rel="shortcut icon" type="image/x-icon" href="docs/images/favicon.ico" />
<link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" integrity="sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=" crossorigin=""/>
<script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js" integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" crossorigin=""></script>
''')
        self.file.write('''
<style>
body {
 font-family: Verdana, Arial;
 font-size: 1.0em;
 color: #bbbbbb;
 background: #333333;
}
table {
 border: 2px solid #bbbbbb;
 border-collapse: collapse;
}
th {
 border: 1px solid #cccccc;
 background: #555555;
}
td {
 border: 1px solid #aaaaaa;
}
a {
 color: #68B0FD;
}
</style>
</head>
<body>
''')

    def write_html_code(self, html_code):
        """Write HTML code in the file"""
        self.file.write(html_code)

    def write_script_start(self):
        """Write tag <script> and JavaScript code to init Leaflet.js."""
        self.file.write('''
<script>
// define boundingbox
var map_boundingbox = [ [ 52.5, 13.3 ], [ 52.8, 13.5 ] ];
resize_boundingbox();

// init map with given boundingbox
var mymap = L.map('mapid').fitBounds( map_boundingbox, {padding: [0,0], maxZoom: 19} );

// init tile server
var tile_server = 'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png';  // OpenStreetMap's Standard tile layer
//var tile_server = 'https://maps.wikimedia.org/osm-intl/{z}/{x}/{y}.png';  // Wikimedia Maps
L.tileLayer( tile_server, {
   maxZoom: 19,
   attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/">OpenStreetMap</a> contributors'
}).addTo(mymap);

// show scale
L.control.scale( { position: 'bottomleft', maxWidth: 200, metric:true, imperial:false } ).addTo(mymap);
''')

    def add_marker(self, lon, lat, popuptext='', openpopup=False):
        """Write Leaflet.js code to display a marker."""
        self.file.write(f'L.marker([{lat}, {lon}])')
        if popuptext != '':
            self.file.write(f".bindPopup('{popuptext}')")
        self.file.write('.addTo(mymap)')
        if openpopup:
            self.file.write('.openPopup()')
        self.file.write(';\n')
        self.adjust_boundingbox(lon, lat)

    def add_polyline(self, lonlat, popuptext=''):
        """Write Leaflet.js code to display a polyline."""
        lat_lon_str = self.lonlat2str(lonlat)
        self.file.write(f'L.polyline( [ {lat_lon_str} ]')
        self.file.write(f", {{ color:'{self.p['color']}', opacity:{self.p['opacity']}, weight:{self.p['weight']}, dashArray:'{self.p['dasharray']}', stroke:true }} )")
        self.file.write('.addTo(mymap)')
        if popuptext != '':
            self.file.write(f".bindPopup('{popuptext}')")
        self.file.write(';\n')

    def add_line(self, lon1, lat1, lon2, lat2, popuptext=''):
        """Write Leaflet.js code to display a simple line."""
        self.add_polyline([(lon1, lat1), (lon2, lat2)], popuptext)  # wrapper for a simple line

    def add_polygon(self, lonlat, popuptext=''):
        """Write Leaflet.js code to display a polygon."""
        lat_lon_str = self.lonlat2str(lonlat)
        self.file.write(f'L.polygon( [ {lat_lon_str} ]')
        self.file.write(f", {{ color:'{self.p['color']}', opacity:{self.p['opacity']}, weight:{self.p['weight']}, dashArray:'{self.p['dasharray']}', stroke:true }} )")
        self.file.write(".addTo(mymap)")
        if popuptext != '':
            self.file.write(f".bindPopup('{popuptext}')")
        self.file.write(';\n')

    def add_circle(self, lon, lat, radius, popuptext=''):
        """Write Leaflet.js code to display a circle."""
        self.file.write(f'L.circle([{lat}, {lon}], {radius}')
        self.file.write(f", {{ color:'{self.p['color']}', opacity:{self.p['opacity']}, weight:{self.p['weight']}, dashArray:'{self.p['dasharray']}', fillColor:'{self.p['fillcolor']}', fillOpacity:{self.p['fillopacity']} }} )")
        self.file.write(".addTo(mymap)")
        if popuptext != '':
            self.file.write(f".bindPopup('{popuptext}')")
        self.file.write(';\n')
        self.adjust_boundingbox(lon, lat)

    def add_circlemarker(self, lon, lat):
        """Write Leaflet.js code to display a circlemarker."""
        self.file.write(f'L.circleMarker([{lat}, {lon}]')
        self.file.write(f", {{ color:'{self.p['color']}', opacity:{self.p['opacity']}, weight:{self.p['weight']}, dashArray:'{self.p['dasharray']}', fillColor:'{self.p['fillcolor']}', fillOpacity:{self.p['fillopacity']} }} )")
        self.file.write(".addTo(mymap);\n")
        self.adjust_boundingbox(lon, lat)

    def add_rectangle(self, lon1, lat1, lon2, lat2, popuptext=''):
        """Write Leaflet.js code to display a rectangle."""
        self.file.write(f'L.rectangle( [ [ {lat1}, {lon1} ], [ {lat2}, {lon2} ] ]')
        self.file.write(f", {{ color:'{self.p['color']}', opacity:{self.p['opacity']}, weight:{self.p['weight']}, dashArray:'{self.p['dasharray']}', fillColor:'{self.p['fillcolor']}', fillOpacity:{self.p['fillopacity']} }} )")
        self.file.write(".addTo(mymap)")
        if popuptext != '':
            self.file.write(f".bindPopup('{popuptext}')")
        self.file.write(';\n')
        self.adjust_boundingbox(lon1, lat1)
        self.adjust_boundingbox(lon2, lat2)

    def write_script_end(self):
        """Write JavaScript code to finish Leaflet.js code and tag </script>."""
        self.file.write(f'''
//
function resize_boundingbox() {{
    map_boundingbox = [ [ {self.bbox['min_lat']}, {self.bbox['min_lon']} ], [ {self.bbox['max_lat']}, {self.bbox['max_lon']} ] ];
}}

// show popup with coordinates at mouse click
var popup = L.popup();
function onMapClick(e) {{
    // copy coordinates in new object
    var geo = e.latlng;
    var lat = geo.lat;
    var lon = geo.lng;
    // round to 7 decimal places
    lat = lat.toFixed(7);
    lon = lon.toFixed(7);
    // output coordinates and zoomlevel on console
    console.log( 'mouse clicked at ' +
     ' lon: ' + lon + ' lat: ' + lat +
     ' zoomlevel: ' + mymap.getZoom() +
     ' Leaflet version: ' + L.version
      );
    //
    var popuptext = '<pre>lon (x) : '+lon+'<br>lat (y) : '+lat+'<br>';
    popup.setLatLng(e.latlng).setContent(popuptext).openOn(mymap);
}}
mymap.on('click', onMapClick);

</script>
''')




def way_edge_points(cur, way_id, start_node_id, end_node_id):
    """Returns a list with all points (lon,lat) of the edge"""
    point_list = []
    cur.execute('''
    SELECT wn.node_id,n.lon,n.lat
    FROM way_nodes AS wn
    LEFT JOIN nodes AS n ON wn.node_id=n.node_id
    WHERE wn.way_id=?
    ORDER BY wn.node_order
    ''', (way_id,))
    add_point = False
    for (node_id, lon, lat) in cur.fetchall():
        if not add_point and node_id==start_node_id:
            add_point = True
        if add_point:
            point_list.append((lon, lat))
        if add_point and node_id==end_node_id:
            add_point = False
    if add_point:       # something wrong with start_node_id or end_node_id
        point_list = []
    return point_list


def node_point(cur, node_id):
    """Returns lon,lat for a given Node ID"""
    res = cur.execute('SELECT lon,lat FROM nodes WHERE node_id=?', (node_id,))
    return res.fetchone()


def html_map_table_graph(cur, min_lon, min_lat, max_lon, max_lat, permit, style, html_filename):
    """Create map from table 'graph'"""
    m = Leaflet(html_filename)
    m.write_html_header('Map Routing Graph')
    m.write_html_code(f'''
    <h2>Map Routing Graph ({min_lon} {min_lat}) ({max_lon} {max_lat}) {permit}</h2>
    <p><div id="mapid" style="width: 100%; height: 700px;"></div></p>
    ''')
    m.write_script_start()
    cur.execute('''
    SELECT way_id,start_node_id,end_node_id,
           CASE
             WHEN (?=2 AND permit&16=16) OR
                  (?=10 AND permit&16=16) OR
                  (?=4 AND permit&32=32) THEN 1
             ELSE 0
           END AS directed,way_id
    FROM graph
    WHERE permit & ? = ? AND
          way_id IN (
                     SELECT way_id FROM rtree_way
                     WHERE max_lon>=? AND min_lon<=?
                       AND max_lat>=? AND min_lat<=?
                    )
    ''', (permit, permit, permit, permit, permit, min_lon, max_lon, min_lat, max_lat))
    m.set_property({'color':'#0000ff', 'opacity':0.8})
    for (way_id, start_node_id, end_node_id, directed, way_id) in cur.fetchall():
        if directed:
            m.set_property({'dasharray':'5 5'})
        else:
            m.set_property({'dasharray':'none'})
        if style == 'line':
            lon1, lat1 = node_point(cur, start_node_id)
            lon2, lat2 = node_point(cur, end_node_id)
            m.add_line(lon1, lat1, lon2, lat2, f'way_id {way_id}')
        elif style == 'course':
            point_list = way_edge_points(cur, way_id, start_node_id, end_node_id)
            m.add_polyline(point_list, f'way_id {way_id}')
    m.set_property(
      {'color': '#ff0000', 'opacity': 1.0, 'weight': 2, 'dasharray': '5 5',
       'fillcolor': 'none', 'fillopacity': 1.0}
    )
    m.add_rectangle(min_lon, min_lat, max_lon, max_lat, '')
    m.write_script_end()
    m.write_html_footer()


def main():
    """entry point"""
    if len(sys.argv) != 9:
        print('Creates an HTML file with a map to display the data in the "graph" table.\n'
              'PERMIT: 1 (foot), 2 (bike), 4 (car) or 10 (bike_paved)\n'
              'STYLE: "line" or "course" (draw straight line or exact course)\n\n'
              'Usage:\n'
              f'{sys.argv[0]} DATABASE MIN_LON MIN_LAT MAX_LON MAX_LAT PERMIT STYLE HTML_FILE')
        sys.exit(1)
    #
    min_lon = float(sys.argv[2])
    min_lat = float(sys.argv[3])
    max_lon = float(sys.argv[4])
    max_lat = float(sys.argv[5])
    permit = int(sys.argv[6])
    style = str(sys.argv[7])
    html_filename = sys.argv[8]
    # connect to the database
    con = sqlite3.connect(sys.argv[1])
    cur = con.cursor()   # new database cursor
    #
    html_map_table_graph(cur, min_lon, min_lat, max_lon, max_lat, permit, style, html_filename)
    # write data to database
    con.commit()
    con.close()


if __name__ == '__main__':
    main()
