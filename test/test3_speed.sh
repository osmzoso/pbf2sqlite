#!/bin/bash
if [ $# != 2 ]; then
    echo
    echo "Test3: Speedtest"
    echo
    echo "Measures the time required for each option."
    echo
    echo "Usage:"
    echo "$0 PBF_FILE DATABASE_FILE"
    exit 1
fi
pbf=$1
db=$2

echo "-----------------------------------------------------------------"
echo "Test3: Speedtest"
echo "-----------------------------------------------------------------"
echo "PBF file      : $pbf"
echo "DATABASE file : $db"

rm -f $db

echo "Time required for option read:"
time -p ../build/pbf2sqlite $db read $pbf
echo "Time required for option index:"
time -p ../build/pbf2sqlite $db index
echo "Time required for option rtree:"
time -p ../build/pbf2sqlite $db rtree
echo "Time required for option addr:"
time -p ../build/pbf2sqlite $db addr
echo "Time required for option graph:"
time -p ../build/pbf2sqlite $db graph
