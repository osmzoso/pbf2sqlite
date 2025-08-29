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

## Links

<https://wiki.openstreetmap.org/wiki/Key:highway>  
<https://wiki.openstreetmap.org/wiki/Key:motorcar>  

<https://wiki.openstreetmap.org/wiki/Key:sidewalk>  
<https://wiki.openstreetmap.org/wiki/Key:foot>  
<https://wiki.openstreetmap.org/wiki/Tag:foot%3Duse_sidepath>  

<https://wiki.openstreetmap.org/wiki/Key:cycleway>  
<https://wiki.openstreetmap.org/wiki/Key:bicycle>  
<https://wiki.openstreetmap.org/wiki/Tag:bicycle%3Duse_sidepath>  

