#!/usr/bin/env python
"""
Comparison of all tables of two pbf2sqlite databases
"""
import sys
import sqlite3


def compare_table(cur, table, columns):
    """Compares the specified columns of a table"""
    print(f'compare table "{table}" ', end='', flush=True)
    try:
        diff1 = cur.execute(f'''
        SELECT count(*) FROM
        (
          SELECT {columns} FROM db1.{table}
          EXCEPT
          SELECT {columns} FROM db2.{table}
        )
        ''').fetchone()[0]
    except:
        print('\033[31m' + 'missing table?' + '\033[0m') 
        return False
    try:
        diff2 = cur.execute(f'''
        SELECT count(*) FROM
        (
          SELECT {columns} FROM db2.{table}
          EXCEPT
          SELECT {columns} FROM db1.{table}
        )
        ''').fetchone()[0]
    except:
        print('\033[31m' + 'missing table?' + '\033[0m') 
        return False
    if diff1 == 0 and diff2 == 0:
        print('\033[32m' + 'OK' + '\033[0m')
        return True
    else:
        print('\033[31m' + 'ERROR' + '\033[0m' +
              f' -> diff db1->db2: {diff1} rows, diff db2->db1: {diff2} rows')
        return False


def compare_pbf2sqlite_db(db1, db2):
    """Establishing database connection, compare each table"""
    print("-----------------------------------------------------------------\n"
          "Test2: Compare two databases\n"
          "-----------------------------------------------------------------\n"
          f"db1 : {db1}\ndb2 : {db2}")
    con = sqlite3.connect(":memory:")
    cur = con.cursor()
    cur.execute(f"ATTACH DATABASE '{db1}' AS db1")
    cur.execute(f"ATTACH DATABASE '{db2}' AS db2")
    rc = compare_table(cur, 'nodes', 'node_id,lon,lat')
    rc = compare_table(cur, 'node_tags', 'node_id,key,value')
    rc = compare_table(cur, 'way_nodes', 'way_id,node_id,node_order')
    rc = compare_table(cur, 'way_tags', 'way_id,key,value')
    rc = compare_table(cur, 'relation_members', 'relation_id,ref,ref_id,role,member_order')
    rc = compare_table(cur, 'relation_tags', 'relation_id,key,value')
    rc = compare_table(cur, 'graph', 'edge_id,start_node_id,end_node_id,dist,way_id,permit')
    if not rc:
        print(" Due to floating point rounding, the 'dist' column in the 'graph' table\n"
              " may have in rare cases different values.\n"
              " Therefore, the comparison again without the 'dist' column:")
        rc = compare_table(cur, 'graph', 'edge_id,start_node_id,end_node_id,way_id,permit')
    rc = compare_table(cur, 'graph_permit', 'key,value,set_bit,clear_bit')


def main():
    """entry point"""
    if len(sys.argv) != 3:
        print('\n'
              'Test2: Compare databases\n'
              '\n'
              'Comparison of all tables of two pbf2sqlite databases.\n\n'
              'Usage:\n'
              f'{sys.argv[0]} DB1 DB2')
        sys.exit(1)
    compare_pbf2sqlite_db(sys.argv[1], sys.argv[2])


if __name__ == "__main__":
    main()
