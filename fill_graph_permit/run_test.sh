#!/bin/bash
if [ $# != 1 ]; then
    echo "Test fill permit"
    echo "Usage:"
    echo "$0 DATABASE"
    exit 1
fi
database=$1

echo "Database : " $database

echo "create table 'graph_permit'..."
sqlite3 $database < graph_permit.sql
echo "fill field permit with old testscript..."
time ./fill_graph_permit.py $database
echo "fill field permit_v2 with new testscript..."
time ./new_fill_graph_permit.py $database
echo "compare field permit and permit_v2..."
sqlite3 $database < compare.sql
