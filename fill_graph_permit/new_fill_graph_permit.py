#!/usr/bin/env python
"""
Classification of edges
"""
import sys
import sqlite3


def clear_bit(value, bit):
    """Clear bit in integer"""
    return value & ~(1 << bit)


def fill_graph_permit(cur):
    """Fill the field 'permit_v2' in table 'graph'"""
    cur.execute('SELECT DISTINCT way_id FROM graph')
    for (way_id,) in cur.fetchall():
        permit = 0b00000000
        #
        tags = set()
        cur.execute('''
        SELECT wt.key,wt.value,gp.set_bit
        FROM way_tags AS wt
        JOIN graph_permit AS gp ON wt.key=gp.key AND wt.value=gp.value
        WHERE wt.way_id=?
        ''', (way_id,))
        for (key, value, set_bit) in cur.fetchall():
            permit = permit | set_bit    # bitwise or
            tags.add(key + '=' + value)
        # Corrections
        if 'surface=asphalt' not in tags and \
           'surface=sett' not in tags and \
           'surface=paving_stones' not in tags:
            permit = clear_bit(permit, 2)
        if 'sidewalk=separate' in tags or \
           'foot=use_sidepath' in tags or \
           'access=no' in tags:
            permit = clear_bit(permit, 0)
        if 'cycleway=separate' in tags or \
           'cycleway:both=separate' in tags or \
           'cycleway:right=separate' in tags or \
           'cycleway:left=separate' in tags or \
           'bicycle=use_sidepath' in tags or \
           'access=no' in tags:
            permit = clear_bit(permit, 2)
            permit = clear_bit(permit, 1)
        if 'oneway:bicycle=no' in tags:
            permit = clear_bit(permit, 4)
        #
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
