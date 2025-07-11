#!/bin/bash
#
# Test3: Compare speed
#
if [ $# != 2 ]; then
    echo "error: no test directory and osm file specified"
    echo "Usage:"
    echo "$0 TEST_DIR OSM_FILE"
    exit 1
fi
test_dir=$1
osm_file=$2

echo "-----------------------------------------------------------------"
echo "Test3: Compare speed"
echo "-----------------------------------------------------------------"
echo "Test dir : $test_dir"
echo "OSM file : $osm_file"

rm -f $test_dir/osm_speed1.db $test_dir/osm_speed2.db

echo "read with osmium + osm2sqlite (predecessor of pbf2sqlite)..."
time -p osmium cat $osm_file -f osm -o - | osm2sqlite osm_speed1.db read -

echo "read with pbf2sqlite..."
time -p pbf2sqlite osm_speed2.db read $osm_file
 
