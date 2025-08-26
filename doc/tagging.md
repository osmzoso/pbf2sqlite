# Tagging

In order to create a graph for routing from the OSM data,
the ways must be tagged correctly.  
Basically, all ways with Key=highway form the graph.  
Furthermore, it must be defined who is allowed to use the way.  
There are basically three types of users:  
foot, bike, car

The definition of which tags grant access is specified in the graph_permit table.

## Access foot

Currently used tags:  
```
SELECT key,value,count(*)
FROM way_tags
WHERE key||'='||value IN
(
  SELECT key||'='||value
  FROM graph_permit
  WHERE set_bit=1
)
GROUP BY key,value
```

```
+----------------+---------------+----------+
|      key       |     value     | count(*) |
+----------------+---------------+----------+
| foot           | designated    | 15018    |
| foot           | yes           | 24168    |
| highway        | construction  | 260      |
| highway        | footway       | 55681    |
| highway        | living_street | 3798     |
| highway        | path          | 54852    |
| highway        | pedestrian    | 2477     |
| highway        | residential   | 62987    |
| highway        | service       | 100769   |
| highway        | steps         | 10368    |
| highway        | track         | 180057   |
| highway        | unclassified  | 14453    |
| sidewalk       | both          | 9651     |
| sidewalk       | left          | 2783     |
| sidewalk       | right         | 4907     |
| sidewalk       | yes           | 6        |
| sidewalk:both  | yes           | 19       |
| sidewalk:left  | yes           | 515      |
| sidewalk:right | yes           | 554      |
+----------------+---------------+----------+
```

<https://wiki.openstreetmap.org/wiki/Key:sidewalk>


## Access bike

TODO

<https://wiki.openstreetmap.org/wiki/Key:cycleway>

**key: cycleway**  

``` sql
SELECT key,value,count(*) AS anzahl
FROM way_tags
WHERE key LIKE 'cycleway%'
GROUP BY key,value
ORDER BY anzahl DESC
```

**key: bicycle**  

``` sql
SELECT key,value,count(*) AS anzahl
FROM way_tags
WHERE key LIKE 'bicycle%'
GROUP BY key,value
ORDER BY anzahl DESC
```

