#!/bin/bash
#
# Test1: Read OSM file with Python and C version
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
echo "Test1: Read OSM file with Python and C version"
echo "-----------------------------------------------------------------"
echo "Test dir : $test_dir"
echo "OSM file : $osm_file"

rm -f $test_dir/osm_py.db $test_dir/osm_c.db

echo "read OSM file with Python version in database 'osm_py.db'..."
time -p ./pbf2sqlite.py $test_dir/osm_py.db read $osm_file addr graph rtree
echo "read OSM file with C version in database 'osm_c.db'..."
time -p ../src/pbf2sqlite $test_dir/osm_c.db read $osm_file addr graph rtree

echo "size of databases:"
ls -l $test_dir/*.db
echo "MD5 hash values of the databases:"
md5sum $test_dir/*.db
