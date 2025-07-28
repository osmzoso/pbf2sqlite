#!/usr/bin/env python
"""
Classification of edges
"""
import sys
import sqlite3


def fill_graph_permit(cur):
    """Fill the field 'permit_v2' in table 'graph'"""
    cur.execute('SELECT DISTINCT way_id FROM graph')
    for (way_id,) in cur.fetchall():
        mask_set = 0b00000000
        mask_clear = 0b11111111
        cur.execute('''
        SELECT gp.set_bit,gp.clear_bit
        FROM way_tags AS wt
        JOIN graph_permit AS gp ON wt.key=gp.key AND wt.value=gp.value
        WHERE wt.way_id=?
        ''', (way_id,))
        for (set_bit, clear_bit) in cur.fetchall():
            mask_set = mask_set | set_bit        # bitwise or
            mask_clear = mask_clear & clear_bit  # bitwise and
        permit = 0b00000000
        permit = permit | mask_set
        permit = permit & mask_clear
        cur.execute('UPDATE graph SET permit_v2=? WHERE way_id=?',
                    (permit, way_id))


def main():
    """entry point"""
    if len(sys.argv) != 2:
        print('Set the bits in the field "permit" in an existing table "graph".\n\n'
              'Bit 0: foot\n'
              'Bit 1: bike_gravel\n'
              'Bit 2: bike_road\n'
              'Bit 3: car\n'
              'Bit 4: bike_oneway\n'
              'Bit 5: car_oneway\n\n'
              'Usage:\n'
              f'{sys.argv[0]} DATABASE')
        sys.exit(1)
    # connect to the database
    con = sqlite3.connect(sys.argv[1])
    cur = con.cursor()   # new database cursor
    #
    fill_graph_permit(cur)
    # write data to database
    con.commit()
    con.close()


if __name__ == "__main__":
    main()
