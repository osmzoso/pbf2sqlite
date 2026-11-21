#!/bin/bash
if [ $# != 3 ]; then
    echo
    echo "Speedtest"
    echo "Measures the time required for each option."
    echo
    echo "Usage:"
    echo "$0 BINARY DATABASE OSM_FILE"
    exit 1
fi
binary=$1
db=$2
osm=$3

echo "DATABASE file : $db"
echo "OSM file      : $osm"

rm -f $db

echo "Time required for option read:"
time -p $binary $db read $osm
echo "Time required for option index:"
time -p $binary $db index
echo "Time required for option rtree:"
time -p $binary $db rtree
echo "Time required for option addr:"
time -p $binary $db addr
echo "Time required for option graph:"
time -p $binary $db graph
echo "Time required for option mercator:"
time -p $binary $db mercator
