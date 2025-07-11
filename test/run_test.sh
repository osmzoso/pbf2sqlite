#!/bin/bash
if [ $# != 2 ]; then
    echo "Test pbf2sqlite"
    echo "Read OpenStreetMap .osm.pbf file and compare the databases"
    echo "Usage:"
    echo "$0 TEST_DIR OSM_FILE"
    exit 1
fi
test_dir=$1
osm_file=$2

echo "Test dir  : " $test_dir
echo "OSM file  : " $osm_file

./test1_read_data.sh $test_dir $osm_file
./test2_compare_databases.py $test_dir/osm_py.db $test_dir/osm_c.db
