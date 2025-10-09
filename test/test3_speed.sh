#!/bin/bash
if [ $# != 2 ]; then
    echo
    echo "Test3: Speedtest"
    echo
    echo "Measures the time required for each option."
    echo
    echo "Usage:"
    echo "$0 TEST_DIR OSM_FILE"
    exit 1
fi
test_dir=$1
osm_file=$2

echo "-----------------------------------------------------------------"
echo "Test3: Speedtest"
echo "-----------------------------------------------------------------"
echo "Test dir : $test_dir"
echo "OSM file : $osm_file"

#echo "Compare with predecessor osm2sqlite:"
#echo "read with osmium + osm2sqlite (predecessor of pbf2sqlite)..."
#rm -f $test_dir/osm_speed.db
#time -p osmium cat $osm_file -f osm -o - | osm2sqlite osm_speed.db read -
#echo "read with pbf2sqlite..."
#rm -f $test_dir/osm_speed.db
#time -p pbf2sqlite osm_speed.db read $osm_file

rm -f $test_dir/osm_speed.db

echo "Time required for option read:"
time -p pbf2sqlite $test_dir/osm_speed.db read $osm_file
echo "Time required for option index:"
time -p pbf2sqlite $test_dir/osm_speed.db index
echo "Time required for option rtree:"
time -p pbf2sqlite $test_dir/osm_speed.db rtree
echo "Time required for option addr:"
time -p pbf2sqlite $test_dir/osm_speed.db addr
echo "Time required for option graph:"
time -p pbf2sqlite $test_dir/osm_speed.db graph

rm -f $test_dir/osm_speed.db
