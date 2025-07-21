#!/usr/bin/env python
"""
Classification of edges
"""
import sys
import sqlite3


def set_bit(value, bit):
    """Set bit in integer"""
    return value | (1 << bit)


def clear_bit(value, bit):
    """Clear bit in integer"""
    return value & ~(1 << bit)


def fill_graph_permit(cur):
    """Fill the field 'permit' in table 'graph'"""
    tags_car = {
      'highway=motorway',
      'highway=motorway_link',
      'highway=trunk',
      'highway=trunk_link'
    }
    tags_car_bike = {
      'highway=primary',
      'highway=primary_link',
      'highway=secondary',
      'highway=secondary_link',
      'highway=tertiary',
      'highway=tertiary_link',
      'highway=unclassified',
      'highway=residential'
    }
    tags_bike_foot = {
      'highway=residential',
      'highway=living_street',
      'highway=service',
      'highway=cycleway',
      'highway=track',
      'highway=unclassified',
      'bicycle=yes',
      'bicycle=designated'
    }
    tags_foot = {
      'highway=pedestrian',
      'highway=track',
      'highway=footway',
      'highway=steps',
      'highway=path',
      'highway=construction',
      'foot=yes',
      'foot=designated',
      'sidewalk=both',
      'sidewalk:both=yes',
      'sidewalk=right',
      'sidewalk:right=yes',
      'sidewalk=left',
      'sidewalk:left=yes',
      'sidewalk=yes'
    }
    cur.execute('SELECT DISTINCT way_id FROM graph')
    for (way_id,) in cur.fetchall():
        permit = 0b00000000
        #
        tags = set()
        cur.execute('SELECT key,value FROM way_tags WHERE way_id=?', (way_id,))
        for (key, value) in cur.fetchall():
            tags.add(key + '=' + value)
        # 1. Set basic flags
        if tags_car.intersection(tags):
            permit = set_bit(permit, 3)
        if tags_car_bike.intersection(tags):
            permit = set_bit(permit, 3)
            permit = set_bit(permit, 2)
            permit = set_bit(permit, 1)
        if tags_bike_foot.intersection(tags):
            permit = set_bit(permit, 2)
            permit = set_bit(permit, 1)
            permit = set_bit(permit, 0)
        if tags_foot.intersection(tags):
            permit = set_bit(permit, 0)
        # 2. Corrections
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
        # 3. One way
        if 'oneway=yes' in tags:
            permit = set_bit(permit, 5)
            permit = set_bit(permit, 4)
        if 'oneway:bicycle=no' in tags:
            permit = clear_bit(permit, 4)
        #
        cur.execute('UPDATE graph SET permit=? WHERE way_id=?',
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
