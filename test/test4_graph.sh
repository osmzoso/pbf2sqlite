#!/bin/bash
if [ $# != 6 ]; then
    echo
    echo "Test4: Visualizes the table 'graph'"
    echo
    echo "Generates a HTML file with maps of the graph."
    echo "A boundingbox must be given."
    echo
    echo "Usage:"
    echo "$0 DATABASE LON1 LAT1 LON2 LAT2 HTML_FILE"
    exit 1
fi
db=$1
lon1=$2
lat1=$3
lon2=$4
lat2=$5
html_file=$6

../build/pbf2sqlite $db vgraph $lon1 $lat1 $lon2 $lat2 $html_file
firefox $html_file

