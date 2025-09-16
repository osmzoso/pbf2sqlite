# Tagging the routing graph

In order to create a graph for routing from the OSM data,
the ways must be tagged correctly.  
Basically all ways that have the key 'highway' form the graph.  
Furthermore, it must be defined who is allowed to use the way.  
There are basically three types of users:  
foot, bike, car

The definition of which tags grant access is specified in the **graph_permit** table.

To fill this table, the first step is to determine all existing keys:

``` sql
SELECT key,count(*) AS number
FROM way_tags
WHERE way_id IN (SELECT way_id FROM way_tags WHERE key='highway')
GROUP BY key
ORDER BY number DESC
```

Testing a pre-filled table:

``` sql
SELECT wt.key,wt.value,wt.number,gp.key,gp.value FROM
(
  SELECT key,value,count(*) AS number
  FROM way_tags
  WHERE key='highway' OR
        key LIKE 'cycleway%' OR
        key LIKE 'sidewalk%' OR
        key LIKE 'bicycle%' OR
        key LIKE 'foot%'
  GROUP BY key,value
) AS wt
LEFT JOIN graph_permit AS gp ON wt.key=gp.key AND wt.value=gp.value
ORDER BY wt.number DESC
```


## Links

<https://taginfo.openstreetmap.org/>  

<https://wiki.openstreetmap.org/wiki/Key:highway>  
<https://wiki.openstreetmap.org/wiki/Key:motor_vehicle>  
<https://wiki.openstreetmap.org/wiki/Key:motorcar>  
<https://wiki.openstreetmap.org/wiki/Key:cycleway>  
<https://wiki.openstreetmap.org/wiki/Key:bicycle>  
<https://wiki.openstreetmap.org/wiki/Key:sidewalk>  
<https://wiki.openstreetmap.org/wiki/Key:foot>  

<https://wiki.openstreetmap.org/wiki/Tag:foot%3Duse_sidepath>  
<https://wiki.openstreetmap.org/wiki/Tag:bicycle%3Duse_sidepath>  
<https://wiki.openstreetmap.org/wiki/Tag:bicycle%3Dyes>  

