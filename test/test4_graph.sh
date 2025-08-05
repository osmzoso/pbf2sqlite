#!/bin/bash
if [ $# != 6 ]; then
    echo
    echo "Test4: Show table 'graph'"
    echo
    echo "Generates HTML files with the graph in TEST_DIR"
    echo "A boundingbox must be given."
    echo
    echo "Usage:"
    echo "$0 DATABASE LON1 LAT1 LON2 LAT2 TEST_DIR"
    exit 1
fi
db=$1
lon1=$2
lat1=$3
lon2=$4
lat2=$5
test_dir=$6

# graph foot
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 1 course $test_dir/graph_foot.html
firefox $test_dir/graph_foot.html

# graph bike
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 2 course $test_dir/graph_bike.html
firefox $test_dir/graph_bike.html

# graph car
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 4 course $test_dir/graph_car.html
firefox $test_dir/graph_car.html

# graph paved
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 8 course $test_dir/graph_paved.html
firefox $test_dir/graph_paved.html

# graph bike paved
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 10 course $test_dir/graph_bike_paved.html
firefox $test_dir/graph_bike_paved.html
