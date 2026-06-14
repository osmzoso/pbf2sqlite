#!/bin/bash
if [ $# != 2 ]; then
    echo "Simple test to verify the functionality of static binaries."
    echo "The binaries for Linux and Windows must be present in the /build directory."
    echo "Usage:"
    echo "$0 BUILD_DIR OSM_FILE"
    exit 1
fi
dir=$1
osm_file=$2

echo "************************************** Check static binaries **************************************"

#
db1="$dir/test_linux.db"
db2="$dir/test_windows.db"

#
rm -f $db1 $db2

echo "Create database '$db1' with the Linux binary..."
$dir/pbf2sqlite          $db1 read $osm_file index rtree addr graph
echo "Create database '$db2' with the Windows binary..."
wine $dir/pbf2sqlite.exe $db2 read $osm_file index rtree addr graph

echo "Size and hash values of the databases:"
ls -l $db1 $db2
sha256sum $db1 $db2

# Compare the hash values
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
hash1=$(sha256sum "$db1" | awk '{print $1}')
hash2=$(sha256sum "$db2" | awk '{print $1}')
if [ $hash1 == $hash2 ]; then
  echo -e "${GREEN}OK${NC} Databases are identical, both binaries should work."
else
  echo -e "${RED}Error${NC} Databases are not identical"
fi

#
rm -f $db1 $db2

echo "***************************************************************************************************"

