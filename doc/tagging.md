# Tagging

In order to create a graph for routing from the OSM data,
the ways must be tagged correctly.  
Basically, all ways with Key=highway form the graph.  
Furthermore, it must be defined who is allowed to use the way.  
There are basically three types of users:  
foot, bike, car

The definition of which tags grant access is specified in the **graph_permit** table.

## Count the tags

``` sql
--
-- Count the keys of all ways that also have key=highway
--
SELECT key,count(*) AS number
FROM way_tags
WHERE way_id IN (SELECT way_id FROM way_tags WHERE key='highway')
GROUP BY key
ORDER BY number DESC
;
--
-- Count the existing tags defined in the graph_permit table.
--
SELECT gp.key,gp.value,count(*)
FROM graph_permit AS gp
LEFT JOIN way_tags AS wt ON gp.key=wt.key AND gp.value=wt.value
GROUP BY gp.key,gp.value
;
--
-- Tags defined multiple times in graph_permit
--
SELECT key,value,count(*) AS number
FROM graph_permit
GROUP BY key,value HAVING number>1
;
```

### Access foot

``` sql
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
<https://wiki.openstreetmap.org/wiki/Key:foot>  
<https://wiki.openstreetmap.org/wiki/Tag:foot%3Duse_sidepath>  


### Access bike

``` sql
SELECT key,value,count(*)
FROM way_tags
WHERE key||'='||value IN
(
  SELECT key||'='||value
  FROM graph_permit
  WHERE set_bit=2
)
GROUP BY key,value
```

```
+----------------+----------------+----------+
|      key       |     value      | count(*) |
+----------------+----------------+----------+
| bicycle        | designated     | 11882    |
| bicycle        | yes            | 23402    |
| cycleway       | lane           | 382      |
| cycleway       | track          | 376      |
| cycleway:right | lane           | 1110     |
| cycleway:right | track          | 658      |
| highway        | cycleway       | 2757     |
| highway        | living_street  | 3798     |
| highway        | residential    | 62987    |
| highway        | secondary      | 15114    |
| highway        | secondary_link | 544      |
| highway        | service        | 100769   |
| highway        | tertiary       | 16097    |
| highway        | tertiary_link  | 432      |
| highway        | track          | 180057   |
| highway        | unclassified   | 14453    |
+----------------+----------------+----------+
```

<https://wiki.openstreetmap.org/wiki/Key:cycleway>  
<https://wiki.openstreetmap.org/wiki/Key:bicycle>  
<https://wiki.openstreetmap.org/wiki/Tag:bicycle%3Duse_sidepath>  


### Access car

``` sql
SELECT key,value,count(*)
FROM way_tags
WHERE key||'='||value IN
(
  SELECT key||'='||value
  FROM graph_permit
  WHERE set_bit=4
)
GROUP BY key,value
```

```
+---------+----------------+----------+
|   key   |     value      | count(*) |
+---------+----------------+----------+
| highway | motorway       | 1455     |
| highway | motorway_link  | 846      |
| highway | primary        | 10034    |
| highway | primary_link   | 908      |
| highway | residential    | 62987    |
| highway | secondary      | 15114    |
| highway | secondary_link | 544      |
| highway | tertiary       | 16097    |
| highway | tertiary_link  | 432      |
| highway | trunk          | 1586     |
| highway | trunk_link     | 801      |
| highway | unclassified   | 14453    |
+---------+----------------+----------+
```

<https://wiki.openstreetmap.org/wiki/Key:motorcar>  

