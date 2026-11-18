#!/bin/bash
if [ $# != 2 ]; then
    echo "Test read OpenStreetMap file and compare the databases"
    echo "Usage:"
    echo "$0 PATH/BINARY OSM_FILE"
    exit 1
fi
binary=$1
osm_file=$2
dir="${binary%/*}"

echo "Binary             : " $binary
echo "OSM file           : " $osm_file
echo "Write databases in : " $dir

echo "-----------------------------------------------------------------"
echo "Test 1: Main options"
echo "-----------------------------------------------------------------"

echo "Test option 'read'..."
rm -f $dir/osm_c.db
$binary $dir/osm_c.db read $osm_file

echo "Test option 'index'..."
$binary $dir/osm_c.db index

echo "Test option 'rtree'..."
$binary $dir/osm_c.db rtree

echo "Test option 'addr'..."
$binary $dir/osm_c.db addr

echo "Test option 'graph'..."
$binary $dir/osm_c.db graph

echo "read OSM file with Python version..."
rm -f $dir/osm_py.db
$dir/../test/pbf2sqlite.py $dir/osm_py.db read $osm_file graph
$dir/../test/compare_databases.py $dir/osm_py.db $dir/osm_c.db


echo "-----------------------------------------------------------------"
echo "Test 2: Show data (weimar.osm)"
echo "-----------------------------------------------------------------"

echo "Test option 'node'..."
$binary $dir/osm_c.db node 2616037670

echo "Test option 'way'..."
$binary $dir/osm_c.db way 15805105

echo "Test option 'relation'..."
$binary $dir/osm_c.db relation 2731564

echo "Test option 'vaddr'..."
$binary $dir/osm_c.db vaddr 11.3309 50.9771 11.3326 50.9786 $dir/vaddr.html
xdg-open $dir/vaddr.html

echo "Test option 'vgraph'..."
$binary $dir/osm_c.db vgraph 11.3309 50.9771 11.3326 50.9786 $dir/vgraph.html
xdg-open $dir/vgraph.html

echo "Test option 'sql'..."
$binary $dir/osm_c.db sql "SELECT * FROM nodes LIMIT 5"
$binary $dir/osm_c.db sql "SELECT radians(lat),sin(radians(lat)) FROM nodes LIMIT 5"

echo "Test option 'sql' (read from stdin)..."
echo "SELECT * FROM nodes LIMIT 5" | $binary $dir/osm_c.db sql


echo "-----------------------------------------------------------------"
echo "Test 3: Routing"
echo "-----------------------------------------------------------------"

echo "Test option 'route'..."
$binary $dir/osm_c.db route foot \
  11.3317806 50.9777393 \
  11.3314828 50.9778879 \
  11.3310429 50.9785668 \
  $dir/route
xdg-open $dir/route.html

# Both coordinates outside the range of weimar.osm -> display error message
#$binary $dir/osm_c.db route foot 11.574 48.137 11.578 48.137 $dir/route2


echo "-----------------------------------------------------------------"
echo "Test 4: Map"
echo "-----------------------------------------------------------------"

echo "Test option 'map'..."
$binary $dir/osm_c.db map 11.3308 50.9782 18 400 400 $dir/map.svg
xdg-open $dir/map.svg

