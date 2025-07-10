#!/bin/bash
#
# Test1: Read .osm.pbf file with Python and C version
#
if [ $# != 2 ]; then
    echo "error: no test directory and osm file specified"
    echo "Usage:"
    echo "$0 TEST_DIR TEST_OSM_FILE"
    exit 1
fi
test_dir=$1
test_osm_file=$2

echo "-----------------------------------------------------------------"
echo "Test1: Read .osm.pbf file with Python and C version"
echo "-----------------------------------------------------------------"
echo "Test dir  : $test_dir"
echo "Test file : $test_osm_file"

rm -f $test_dir/osm_py.db $test_dir/osm_c.db

echo "read .osm.pbf file with Python version in database 'osm_py.db'..."
time -p ./pbf2sqlite.py $test_dir/osm_py.db read $test_osm_file
echo "read .osm.pbf file with C version in database 'osm_c.db'..."
time -p ../src/pbf2sqlite $test_dir/osm_c.db read $test_osm_file

echo "size of databases:"
ls -l $test_dir/*.db
echo "MD5 hash values of the databases:"
md5sum $test_dir/*.db
