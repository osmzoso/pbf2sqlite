--
-- Show bitfield permit in table 'graph_permit'
--
.mode table

.print "The following tags set the bits for access:"
SELECT key,value,set_bit,
  CASE WHEN set_bit&128=128 THEN 'set' ELSE '' END AS bit7,
  CASE WHEN set_bit&64 = 64 THEN 'set' ELSE '' END AS bit6,
  CASE WHEN set_bit&32 = 32 THEN 'oneway_car' ELSE '' END AS bit5,
  CASE WHEN set_bit&16 = 16 THEN 'oneway_bike' ELSE '' END AS bit4,
  CASE WHEN set_bit&8  =  8 THEN 'paved' ELSE '' END AS bit3,
  CASE WHEN set_bit&4  =  4 THEN 'car' ELSE '' END AS bit2,
  CASE WHEN set_bit&2  =  2 THEN 'bike' ELSE '' END AS bit1,
  CASE WHEN set_bit&1  =  1 THEN 'foot' ELSE '' END AS bit0
FROM graph_permit
WHERE set_bit>0
ORDER BY rowid
;


.print "The following tags clear the bits for access:"
SELECT key,value,clear_bit,
  CASE WHEN clear_bit&128=0 THEN 'set' ELSE '' END AS bit7,
  CASE WHEN clear_bit&64 =0 THEN 'set' ELSE '' END AS bit6,
  CASE WHEN clear_bit&32 =0 THEN 'oneway_car' ELSE '' END AS bit5,
  CASE WHEN clear_bit&16 =0 THEN 'oneway_bike' ELSE '' END AS bit4,
  CASE WHEN clear_bit&8  =0 THEN 'paved' ELSE '' END AS bit3,
  CASE WHEN clear_bit&4  =0 THEN 'car' ELSE '' END AS bit2,
  CASE WHEN clear_bit&2  =0 THEN 'bike' ELSE '' END AS bit1,
  CASE WHEN clear_bit&1  =0 THEN 'foot' ELSE '' END AS bit0
FROM graph_permit
WHERE clear_bit<255
ORDER BY rowid
;


.print "Number of most frequently used tags for ways that have key='highway':"
SELECT key,value,count(*) AS number
FROM way_tags
WHERE way_id IN (SELECT DISTINCT way_id FROM way_tags WHERE key='highway')
 AND key NOT IN ('FIXME','fixme')
 AND key NOT LIKE 'name%'
 AND key NOT LIKE 'ref%'
 AND key NOT LIKE 'source%'
 AND key NOT LIKE 'note%'
 AND key!='is_in'
GROUP BY key,value HAVING number>=100
ORDER BY number DESC
;

