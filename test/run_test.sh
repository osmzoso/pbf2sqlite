#!/bin/bash
if [ $# != 2 ]; then
    echo "Test pbf2sqlite"
    echo "Read OpenStreetMap .osm.pbf file and compare the databases"
    echo "Usage:"
    echo "$0 TEST_DIR OSM_FILE"
    exit 1
fi
dir=$1
osm_file=$2

echo "Test dir  : " $dir
echo "OSM file  : " $osm_file

echo "-----------------------------------------------------------------"
echo "Test 1: Main options"
echo "-----------------------------------------------------------------"

echo "Test option 'read'..."
rm -f $dir/osm_c.db
../build/pbf2sqlite $dir/osm_c.db read $osm_file

echo "Test option 'index'..."
../build/pbf2sqlite $dir/osm_c.db index

echo "Test option 'rtree'..."
../build/pbf2sqlite $dir/osm_c.db rtree

echo "Test option 'addr'..."
../build/pbf2sqlite $dir/osm_c.db addr

echo "Test option 'graph'..."
../build/pbf2sqlite $dir/osm_c.db graph

echo "read OSM file with Python version..."
rm -f $dir/osm_py.db
./pbf2sqlite.py $dir/osm_py.db read $osm_file graph
./compare_databases.py $dir/osm_py.db $dir/osm_c.db


echo "-----------------------------------------------------------------"
echo "Test 2: Show data (weimar.osm)"
echo "-----------------------------------------------------------------"

echo "Test option 'node'..."
../build/pbf2sqlite $dir/osm_c.db node 2616037670

echo "Test option 'way'..."
../build/pbf2sqlite $dir/osm_c.db way 15805105

echo "Test option 'relation'..."
../build/pbf2sqlite $dir/osm_c.db relation 2731564

echo "Test option 'vaddr'..."
../build/pbf2sqlite $dir/osm_c.db vaddr 11.3309 50.9771 11.3326 50.9786 $dir/vaddr.html
firefox $dir/vaddr.html

echo "Test option 'vgraph'..."
../build/pbf2sqlite $dir/osm_c.db vgraph 11.3309 50.9771 11.3326 50.9786 $dir/vgraph.html
firefox $dir/vgraph.html

echo "Test option 'sql'..."
../build/pbf2sqlite $dir/osm_c.db sql "SELECT * FROM nodes LIMIT 5"
../build/pbf2sqlite $dir/osm_c.db sql "SELECT radians(lat),sin(radians(lat)) FROM nodes LIMIT 5"

echo "Test option 'sql' (read from stdin)..."
echo "SELECT * FROM nodes LIMIT 5" | ../build/pbf2sqlite $dir/osm_c.db sql


echo "-----------------------------------------------------------------"
echo "Test 3: Routing"
echo "-----------------------------------------------------------------"

echo "Test option 'route'..."
../build/pbf2sqlite $dir/osm_c.db route 11.3314 50.9778 11.3320 50.9785 foot $dir/route1
firefox $dir/route1.html

../build/pbf2sqlite $dir/osm_c.db route 1.530 42.507 1.549 42.517 car $dir/route2
firefox $dir/route2.html

