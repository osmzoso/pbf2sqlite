#!/bin/bash
if [ $# != 2 ]; then
    echo
    echo "Test1: Read OSM file with Python and C version"
    echo
    echo "Usage:"
    echo "$0 TEST_DIR OSM_FILE"
    exit 1
fi
test_dir=$1
osm_file=$2

echo "-----------------------------------------------------------------"
echo "Test1: Read OSM file with Python and C version"
echo "-----------------------------------------------------------------"
echo "Test dir : $test_dir"
echo "OSM file : $osm_file"

rm -f $test_dir/osm_py.db $test_dir/osm_c.db

echo "read OSM file with Python version in database 'osm_py.db'..."
time -p ./pbf2sqlite.py $test_dir/osm_py.db read $osm_file graph
echo "read OSM file with C version in database 'osm_c.db'..."
time -p ../build/pbf2sqlite $test_dir/osm_c.db read $osm_file graph

echo "size of databases:"
ls -l $test_dir/*.db
echo "MD5 hash values of the databases:"
md5sum $test_dir/*.db
