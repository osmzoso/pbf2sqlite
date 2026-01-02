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
echo "Test 2: Show data (Andorra)"
echo "-----------------------------------------------------------------"

echo "Test option 'node'..."
../build/pbf2sqlite $dir/osm_c.db node 5447216522

echo "Test option 'way'..."
../build/pbf2sqlite $dir/osm_c.db way 1426053359

echo "Test option 'relation'..."
../build/pbf2sqlite $dir/osm_c.db relation 1843715

echo "Test option 'vaddr'..."
../build/pbf2sqlite $dir/osm_c.db vaddr 1.527 42.505 1.536 42.508 $dir/vaddr.html
firefox $dir/vaddr.html

echo "Test option 'vgraph'..."
../build/pbf2sqlite $dir/osm_c.db vgraph 1.520 42.501 1.549 42.517 $dir/vgraph.html
firefox $dir/vgraph.html

echo "Test option 'sql'..."
../build/pbf2sqlite $dir/osm_c.db sql "SELECT * FROM nodes LIMIT 10"


echo "-----------------------------------------------------------------"
echo "Test 3: Routing"
echo "-----------------------------------------------------------------"

echo "Test option 'route'..."
../build/pbf2sqlite $dir/osm_c.db route 1.522 42.505 1.549 42.517 foot $dir/route1.html

