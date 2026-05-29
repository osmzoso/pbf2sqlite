#!/usr/bin/env python
"""
Prototype map renderer for pbf2sqlite
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
# Mercator Projection
#
def size_world_map(zoomlevel):
    """
    Calculates the size of a square world map in pixels for a given zoom level
    """
    tile_size = 256     # tile 256px x 256px
    world_map_size = 2**zoomlevel * tile_size
    return world_map_size


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


def webmercator_to_pixel(x, y, world_map_size):
    """
    Transform Web Mercator to pixel coordinates
    """
    world_map_size -= 1
    webmercator = 20037508.342789244
    x += webmercator  # move origin to avoid negativ coordinates
    y += webmercator
    x_px = int(round((x * world_map_size) / (webmercator * 2), 0))
    y_px = int(round((y * world_map_size) / (webmercator * 2), 0))
    return x_px, y_px


def pixel_to_webmercator(x_px, y_px, world_map_size):
    """
    Transform pixel coordinates to Web Mercator
    """
    webmercator = 20037508.342789244
    x_px -= world_map_size / 2
    y_px -= world_map_size / 2
    x = (x_px / world_map_size) * (webmercator * 2)
    y = (y_px / world_map_size) * (webmercator * 2)
    return x, y


def spherical_to_pixel(lon, lat, world_map_size):
    """
    Transform spherical coordinates to pixel coordinates
    """
    x, y = spherical_to_mercator(lon, lat)
    x_px, y_px = webmercator_to_pixel(x, y, world_map_size)
    return x_px, y_px


def pixel_to_spherical(x_px, y_px, world_map_size):
    """
    Transform pixel coordinates to spherical coordinates
    """
    x, y = pixel_to_webmercator(x_px, y_px, world_map_size)
    lon, lat = mercator_to_spherical(x, y)
    return lon, lat


def boundingbox_pixel(lon, lat, world_map_size, width, height):
    """
    Calculate pixel boundingbox
    """
    x, y = spherical_to_pixel(lon, lat, world_map_size)
    min_x = x - int(width / 2)
    min_y = y - int(height / 2)
    max_x = x + int(width / 2)
    max_y = y + int(height / 2)
    return min_x, min_y, max_x, max_y


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


def matrix_world_device(x_min, x_max, y_min, y_max, width, height, origin_top_left):
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
    world_map_size = size_world_map(zoomlevel)
    # calculate pixel and sperical boundingboxes
    x1, y1, x2, y2 = boundingbox_pixel(map_lon, map_lat, world_map_size, width, height)
    lon1, lat1 = pixel_to_spherical(x1, y1, world_map_size)
    lon2, lat2 = pixel_to_spherical(x2, y2, world_map_size)
    #
    d = matrix_world_device(x1, x2, y1, y2, width, height, True)
    # TEST
    if False:
        print("Input:", map_lon, map_lat, zoomlevel, width, height)
        print("World map size:", world_map_size)
        print("Boundingbox pixel:", x1, y1, x2, y2)
        print("Boundingbox spherical:", lon1, lat1, lon2, lat2)
        print("Transform Matrix World -> Screen:\n", d)
        print(x1, y1, "->", transform_point(x1, y1, d))
        print(x2, y2, "->", transform_point(x2, y2, d))
    #
    p['delta_x'] = (lon2 - lon1) / 2
    p['delta_y'] = (lat2 - lat1) / 2
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
            x, y = spherical_to_pixel(lon, lat, world_map_size)
            polyline.append(transform_point(x, y, d))
        canvas.create_line(polyline, fill='blue', width=1)
        #
        cur.execute("SELECT key,value FROM way_tags WHERE way_id=?", (way_id,))
        for (key, value) in cur.fetchall():
            pass


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
    root.title("pbf2sqlite test map")
    # bind any key to a function
    root.bind("<Key>", on_key_press)
    # draw the map
    draw_map(cur, lon, lat, zoomlevel, width, height)
    #
    root.mainloop()


#
#
#
def main():
    """entry point"""
    global p
    if len(sys.argv) == 1:
        print('Creates a simple map.\n'
              'Usage:\n'
              f'{sys.argv[0]} DATABASE LON LAT ZOOMLEVEL WIDTH HEIGHT\n')
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


if __name__ == '__main__':
    main()
