#!/bin/bash
#
#
#
#db=$HOME/osm/database/germany-latest.db
db=$HOME/osm/database/freiburg-regbez-latest.db

# boundingbox
lon1=7.84
lat1=47.98
lon2=7.86
lat2=47.995

# graph foot
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 1 course ~/graph_foot.html
firefox ~/graph_foot.html

# graph bike
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 2 course ~/graph_bike.html
firefox ~/graph_bike.html

# graph car
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 4 course ~/graph_car.html
firefox ~/graph_car.html

# graph paved
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 8 course ~/graph_paved.html
firefox ~/graph_paved.html

# graph bike paved
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 10 course ~/graph_bike_paved.html
firefox ~/graph_bike_paved.html
