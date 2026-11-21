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
echo "1. Test commands: read index rtree addr graph mercator version"
echo "-----------------------------------------------------------------"

echo "read OSM file with C version..."
rm -f $dir/osm_c.db
$binary \
 $dir/osm_c.db \
 read $osm_file \
 index \
 rtree \
 addr \
 graph \
 mercator \
 version

echo "read OSM file with Python version..."
rm -f $dir/osm_py.db
$dir/../test/pbf2sqlite.py $dir/osm_py.db read $osm_file graph

$dir/../test/compare_databases.py $dir/osm_py.db $dir/osm_c.db


echo "-----------------------------------------------------------------"
echo "2. Test commands: node way relation vaddr vgraph"
echo "-----------------------------------------------------------------"

$binary \
 $dir/osm_c.db \
 node 2616037670 \
 way 15805105 \
 relation 2731564 \
 vaddr 11.3317 50.9781 150 $dir/vaddr.html \
 vgraph 11.3317 50.9781 150 all $dir/vgraph.html
xdg-open $dir/vaddr.html
xdg-open $dir/vgraph.html


echo "-----------------------------------------------------------------"
echo "3. Test commands: sql"
echo "-----------------------------------------------------------------"

$binary \
 $dir/osm_c.db \
 sql "SELECT * FROM nodes LIMIT 3" \
 sql "SELECT radians(lat),sin(radians(lat)) FROM nodes LIMIT 3" \
 sql "SELECT distance(13.3645, 52.5108, 13.4096, 52.5182)||' m'"

echo "Test command 'sql' (read from stdin)..."
echo "SELECT * FROM nodes LIMIT 3" | $binary $dir/osm_c.db sql


echo "-----------------------------------------------------------------"
echo "4. Test commands: route"
echo "-----------------------------------------------------------------"

$binary \
  $dir/osm_c.db \
  route \
  foot \
  11.3317806 50.9777393 \
  11.3314828 50.9778879 \
  11.3310429 50.9785668 \
  $dir/route
xdg-open $dir/route.html

# Both coordinates outside the range of weimar.osm -> display error message
#$binary $dir/osm_c.db route foot 11.574 48.137 11.578 48.137 $dir/route2


echo "-----------------------------------------------------------------"
echo "5. Test commands: map"
echo "-----------------------------------------------------------------"

$binary \
 $dir/osm_c.db map 11.3308 50.9782 18 400 400 $dir/map.svg
xdg-open $dir/map.svg

