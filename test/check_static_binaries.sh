#!/bin/bash
if [ $# != 3 ]; then
    echo "Test to verify the functionality of static binaries for Linux and Windows"
    echo "Usage:"
    echo "$0 PATH/BINARY_LINUX PATH/BINARY_WINDOWS OSM_FILE"
    exit 1
fi
binary_linux=$1
binary_windows=$2
osm_file=$3
dir="${binary_linux%/*}"

echo "************************************** Check static binaries **************************************"

echo "Binary Linux       : " $binary_linux
echo "Binary Windows     : " $binary_windows
echo "OSM file           : " $osm_file
echo "Write databases in : " $dir

#
db1="$dir/test_linux.db"
db2="$dir/test_windows.db"

#
rm -f $db1 $db2

echo "Create database '$db1' with the Linux binary..."
$binary_linux          $db1 read $osm_file index rtree addr graph
echo "Create database '$db2' with the Windows binary..."
wine $binary_windows   $db2 read $osm_file index rtree addr graph

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

