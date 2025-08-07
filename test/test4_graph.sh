#!/bin/bash
if [ $# != 6 ]; then
    echo
    echo "Test4: Visualizes the table 'graph'"
    echo
    echo "Generates HTML files with the graph in TEST_DIR."
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

permit=0
filename=$test_dir/graph_all_edges.html
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 $permit course $filename
firefox $filename

permit=1
filename=$test_dir/graph_foot.html
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 $permit course $filename
firefox $filename

permit=2
filename=$test_dir/graph_bike.html
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 $permit course $filename
firefox $filename

permit=4
filename=$test_dir/graph_car.html
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 $permit course $filename
firefox $filename

permit=8
filename=$test_dir/graph_paved.html
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 $permit course $filename
firefox $filename

permit=10
filename=$test_dir/graph_bike_paved.html
./html_leaflet_graph.py $db $lon1 $lat1 $lon2  $lat2 $permit course $filename
firefox $filename

