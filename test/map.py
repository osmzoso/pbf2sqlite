#!/usr/bin/env python
"""
Test map renderer

spherical : lon (-180 to +180 degrees), lat (-90 to +90 degrees)
mercator  : x (-20037508.343 to -20037508.343 meter), y (-20037508.343 to -20037508.343)
world     : Square world map (size in pixels is arbitrary)
picture   : part of world, translate to 0,0
"""
import sys
import time
import sqlite3
import tkinter as tk
import math as m
import numpy as np

# Global variables
p = None
canvas = None
root = None


#
# Size world
#
def world_pixel(zoomlevel):
    """
    Calculates the size of a square world map in pixels for a given zoom level
    """
    tile_size = 256     # tile 256px x 256px
    world_size = 2**zoomlevel * tile_size
    return world_size


#
# Mercator Projection
#
def spherical_to_mercator(lon, lat):
    "Converts spherical coordinates into planar coordinates"
    r = 6378137.0
    x = r * m.radians(lon)
    y = r * m.log(m.tan(m.pi / 4 + m.radians(lat) / 2))
    return x, y


def mercator_to_spherical(x, y):
    "Converts planar coordinates into spherical coordinates"
    r = 6378137.0
    lon = m.degrees(x / r)
    lat = m.degrees(2 * m.atan(m.exp(y / r)) - m.pi / 2.0)
    return lon, lat


#
# Matrix Functions
#
def matrix_scaling(sx, sy):
    "Returns a scaling matrix"
    return np.array([
     (sx,  0,  0),
     ( 0, sy,  0),
     ( 0,  0,  1) ])


def matrix_rotation(theta):
    "Returns a rotation matrix"
    return np.array([
     (   np.cos(theta), np.sin(theta), 0),
     (-1*np.sin(theta), np.cos(theta), 0),
     (               0,             0, 1) ])


def matrix_translation(tx, ty):
    "Returns a translation matrix"
    return np.array([
     ( 1,  0,  0),
     ( 0,  1,  0),
     (tx, ty,  1) ])


#
#
#
def mercator_to_world(world_size):
    "Returns a transform matrix to convert webmercator coordinates into world coordinates"
    webmercator = 20037508.342789244
    # Translate coordinates to avoid negative numbers
    tx = webmercator
    ty = webmercator
    t = matrix_translation(tx, ty)
    # Scale webmercator coordinates to world coordinates
    sx = (world_size - 1) / (2 * webmercator)
    sy = (world_size - 1) / (2 * webmercator)
    s = matrix_scaling(sx, sy)
    # Create the transformation matrix
    mercator_to_world = t @ s
    return mercator_to_world


def world_to_mercator(world_size):
    "Returns a transform matrix to convert world coordinates into webmercator coordinates"
    webmercator = 20037508.342789244
    # Translate coordinates to get negative numbers
    tx = -1 * world_size / 2
    ty = -1 * world_size / 2
    t = matrix_translation(tx, ty)
    # Scale world coordinates to webmercator coordinates
    sx = (2 * webmercator) / (world_size - 1)
    sy = (2 * webmercator) / (world_size - 1)
    s = matrix_scaling(sx, sy)
    # Create the transformation matrix
    world_to_mercator = t @ s
    return world_to_mercator


def world_to_picture(x_min, x_max, y_min, y_max, width, height, origin_top_left):
    """
    Returns a transform matrix A to convert world coordinate into screen coordinates.
    First a translation matrix T and a scaling matrix S is generated.
    Then the transform matrix is calculated with A = S⋅T
    """
    # check if aspect ratio of world and device is the same
    if not np.isclose(width / height, (x_max - x_min) / (y_max - y_min)):
        print("Warning: aspect ratio differs between world and device coordinates")
    # scaling
    sx = width / (x_max - x_min)
    sy = height / (y_max - y_min)
    if origin_top_left:
        sy = sy * -1
    # translation
    tx = -1 * (width * x_min) / (x_max - x_min)
    ty = -1 * (height * y_min) / (y_max - y_min)
    if origin_top_left:
        ty = ((height * y_min) / (y_max - y_min)) + height
    #
    s = matrix_scaling(sx, sy)
    t = matrix_translation(tx, ty)
    a = s @ t
    return a


def transform_point(x, y, t):
    "Transform a point (x, y) with a matrix t"
    point = np.array([x, y, 1]) @ t
    return (point[0], point[1])


#
#
#
def draw_map(cur, map_lon, map_lat, zoomlevel, width, height):
    """
    Draw a map.
    """
    # Determine the size of the world map
    world_size = world_pixel(zoomlevel)
    # Create the transformation matrices for mercator to world and vice versa
    matrix_mercator_to_world = mercator_to_world(world_size)
    matrix_world_to_mercator = world_to_mercator(world_size)
    # Calculate world boundingbox x1,y1,x2,y2
    x, y = spherical_to_mercator(map_lon, map_lat)
    x, y = transform_point(x, y, matrix_mercator_to_world)
    x1 = x - int(width / 2)
    y1 = y - int(height / 2)
    x2 = x + int(width / 2)
    y2 = y + int(height / 2)
    # Calculate spherical boundingbox lon1,lat1,lon2,lat2
    x, y = transform_point(x1, y1, matrix_world_to_mercator)
    lon1, lat1 = mercator_to_spherical(x, y)
    x, y = transform_point(x2, y2, matrix_world_to_mercator)
    lon2, lat2 = mercator_to_spherical(x, y)
    # Only Tk steering
    p['delta_x'] = (lon2 - lon1) / 2
    p['delta_y'] = (lat2 - lat1) / 2
    # Create the transformation matrix world to picture
    matrix_world_to_picture = world_to_picture(x1, x2, y1, y2, width, height, True)
    # Create the transformation matrix mercator to picture
    matrix_mercator_to_picture = matrix_mercator_to_world @ matrix_world_to_picture
    # get all ways in the boundingbox
    cur.execute("""
    SELECT way_id
    FROM rtree_way
    WHERE max_lon>=? AND min_lon<=?
     AND  max_lat>=? AND min_lat<=?
    """, (lon1, lon2, lat1, lat2))
    for (way_id,) in cur.fetchall():
        # coordinates of the way
        polyline = []
        cur.execute("""
        SELECT nodes.lon,nodes.lat
        FROM way_nodes
        LEFT JOIN nodes ON way_nodes.node_id=nodes.node_id
        WHERE way_nodes.way_id=?
        ORDER BY way_nodes.node_order
        """, (way_id,))
        for (lon, lat) in cur.fetchall():
            x, y = spherical_to_mercator(lon, lat)
            x, y = transform_point(x, y, matrix_mercator_to_picture)
            polyline.append((x, y))
        #
        cur.execute("SELECT key,value FROM way_tags WHERE way_id=?", (way_id,))
        for (key, value) in cur.fetchall():
            pass
        #
        canvas.create_line(polyline, fill='blue', width=1)


#
# Tk functions
#
def on_key_press(event):
    "Evaluates key inputs in Tk"
    if event.keysym == 'q':
        sys.exit()
    elif event.keysym == 'z':
        p['zoomlevel'] = p['zoomlevel'] + 1
    elif event.keysym == 'u':
        p['zoomlevel'] = p['zoomlevel'] - 1
    elif event.keysym == 'Right':
        p['lon'] = p['lon'] + p['delta_x']
    elif event.keysym == 'Left':
        p['lon'] = p['lon'] - p['delta_x']
    elif event.keysym == 'Up':
        p['lat'] = p['lat'] + p['delta_y']
    elif event.keysym == 'Down':
        p['lat'] = p['lat'] - p['delta_y']
    print(f"map  lon {p['lon']:.7f}  lat {p['lat']:.7f}  "
          f"zoomlevel {p['zoomlevel']}", end='', flush=True)
    start = time.time()
    canvas.delete("all")
    draw_map(p['cur'], p['lon'], p['lat'], p['zoomlevel'], p['width'], p['height'])
    elapsed_time = time.time()-start
    print(f"  {elapsed_time:.5f} s")


def tk_show_map(cur, lon, lat, zoomlevel, width, height):
    "Opens a Tk window with a canvas"
    global root, canvas
    print("Keys: z(oom), u(nzoom), right, left, up, down, q(uit)")
    # init canvas
    root = tk.Tk()
    canvas = tk.Canvas(root, width=width, height=height)
    canvas.pack()
    root.title("test map renderer")
    # bind any key to a function
    root.bind("<Key>", on_key_press)
    # draw the map
    draw_map(cur, lon, lat, zoomlevel, width, height)
    #
    root.mainloop()


#
# Test
#
def test_spherical_to_mercator(lon, lat):
    mx, my = spherical_to_mercator(lon, lat)
    print(f' lon: {lon:13.8f} lat: {lat:13.8f}  ->  mx: {mx:19.9f} my: {my:19.9f}')


def test_mercator_to_spherical(mx, my):
    lon, lat = mercator_to_spherical(mx, my)
    print(f' mx: {mx:19.9f} my: {my:19.9f}  ->  lon: {lon:13.8f} lat: {lat:13.8f}')


def test_mercator_to_world(mx, my, world_size):
    matrix_mercator_to_world = mercator_to_world(world_size)
    #
    wx, wy = transform_point(mx, my, matrix_mercator_to_world)
    wx = int(round(wx , 0))
    wy = int(round(wy , 0))
    print(f' mx: {mx:19.9f} my: {my:19.9f} world_size: {world_size:10}  ->  wx: {wx:10} wy: {wy:10}')


def test_world_to_mercator(wx, wy, world_size):
    matrix_world_to_mercator = world_to_mercator(world_size)
    #
    mx, my = transform_point(wx, wy, matrix_world_to_mercator)
    print(f' wx: {wx:10} wy: {wy:10} world_size: {world_size:10}  ->  mx: {mx:19.9f} my: {my:19.9f}')


def test_proj():
    print('Test world_pixel():')
    for zoomlevel in range(19):
        world_size = world_pixel(zoomlevel)
        print(f'zoomlevel: {zoomlevel:3}  ->  world_size: {world_size:10}')
    print('Test spherical_to_mercator():')
    test_spherical_to_mercator( 180.00,  85.05112878)
    test_spherical_to_mercator(-180.00, -85.05112878)
    test_spherical_to_mercator(   0.00,   0.00)
    test_spherical_to_mercator(   7.81,  48.00)
    print('Test mercator_to_spherical():')
    test_mercator_to_spherical( 20037508.343,  20037508.343)
    test_mercator_to_spherical(-20037508.343, -20037508.343)
    test_mercator_to_spherical(0,0)
    #
    world_size = 100000
    print('Test matrix mercator_to_world():')
    test_mercator_to_world( 20037508.343,  20037508.343, world_size)
    test_mercator_to_world(            0,             0, world_size)
    test_mercator_to_world(-20037508.343, -20037508.343, world_size)
    print('Test matrix world_to_mercator():')
    test_world_to_mercator( world_size - 1,  world_size - 1, world_size)
    test_world_to_mercator( world_size / 2,  world_size / 2, world_size)
    test_world_to_mercator(              0,               0, world_size)


#
#
#
def main():
    """entry point"""
    global p
    if len(sys.argv) == 1:
        print('Creates a simple map.\n'
              'Usage:\n'
              f'{sys.argv[0]} DATABASE LON LAT ZOOMLEVEL WIDTH HEIGHT\n'
              f'{sys.argv[0]} test\n')
        sys.exit(1)
    if len(sys.argv) >= 7:
        con = sqlite3.connect(sys.argv[1])  # database connection
        cur = con.cursor()                  # new database cursor
        lon = float(sys.argv[2])
        lat = float(sys.argv[3])
        zoomlevel = int(sys.argv[4])
        width = int(sys.argv[5])
        height = int(sys.argv[6])
        # dict with actual status of the map
        p = {'cur': cur, 'lon': lon, 'lat': lat, 'zoomlevel': zoomlevel,
             'width': width, 'height': height,
             'delta_x': 0.001, 'delta_y': 0.001}
        #
        tk_show_map(cur, lon, lat, zoomlevel, width, height)
    if sys.argv[1] == 'test' and len(sys.argv) == 2:
        test_proj()


if __name__ == '__main__':
    main()
