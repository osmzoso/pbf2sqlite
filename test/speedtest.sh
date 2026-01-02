#!/bin/bash
if [ $# != 2 ]; then
    echo
    echo "Speedtest pbf2sqlite"
    echo "Measures the time required for each option."
    echo
    echo "Usage:"
    echo "$0 DATABASE OSM_FILE"
    exit 1
fi
db=$1
osm=$2

echo "DATABASE file : $db"
echo "OSM file      : $osm"

rm -f $db

echo "Time required for option read:"
time -p ../build/pbf2sqlite $db read $osm
echo "Time required for option index:"
time -p ../build/pbf2sqlite $db index
echo "Time required for option rtree:"
time -p ../build/pbf2sqlite $db rtree
echo "Time required for option addr:"
time -p ../build/pbf2sqlite $db addr
echo "Time required for option graph:"
time -p ../build/pbf2sqlite $db graph
