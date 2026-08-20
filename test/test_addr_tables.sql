--
-- Test addr_street table
--
.mode table

SELECT table_name,format('%12d', addr_tags) AS addr_tags FROM
(
 SELECT 'node_tags'     AS table_name,count(*) AS addr_tags FROM node_tags      WHERE key LIKE 'addr%'
 UNION
 SELECT 'way_tags'      AS table_name,count(*) AS addr_tags FROM way_tags       WHERE key LIKE 'addr%'
 UNION
 SELECT 'relation_tags' AS table_name,count(*) AS addr_tags FROM relation_tags  WHERE key LIKE 'addr%'
 ORDER BY addr_tags
)
;

--
-- Table node_tags
--
.print
.print "Check whether all addr tags in the 'node_tags' table appear in the 'addr_street' table:"
.print
.print "Test table 'node_tags' tag 'addr:country' ..."
SELECT nt.node_id,nt.key,nt.value,hn.street_id,st.country
FROM node_tags AS nt
LEFT JOIN addr_housenumber AS hn ON nt.node_id=hn.node_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE nt.key='addr:country' AND nt.value!=st.country
;
.print "Test table 'node_tags' tag 'addr:postcode' ..."
SELECT nt.node_id,nt.key,nt.value,hn.street_id,st.postcode
FROM node_tags AS nt
LEFT JOIN addr_housenumber AS hn ON nt.node_id=hn.node_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE nt.key='addr:postcode' AND nt.value!=st.postcode
;
.print "Test table 'node_tags' tag 'addr:city' ..."
SELECT nt.node_id,nt.key,nt.value,hn.street_id,st.city
FROM node_tags AS nt
LEFT JOIN addr_housenumber AS hn ON nt.node_id=hn.node_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE nt.key='addr:city' AND nt.value!=st.city
;
.print "Test table 'node_tags' tag 'addr:street' ..."
SELECT nt.node_id,nt.key,nt.value,hn.street_id,st.street
FROM node_tags AS nt
LEFT JOIN addr_housenumber AS hn ON nt.node_id=hn.node_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE nt.key='addr:street' AND nt.value!=st.street
;

--
-- Table way_tags
--
.print
.print "Check whether all addr tags in the 'way_tags' table appear in the 'addr_street' table:"
.print
.print "Test table 'way_tags' tag 'addr:country' ..."
SELECT wt.way_id,wt.key,wt.value,hn.street_id,st.country
FROM way_tags AS wt
LEFT JOIN addr_housenumber AS hn ON wt.way_id=hn.way_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE wt.key='addr:country' AND wt.value!=st.country
;
.print "Test table 'way_tags' tag 'addr:postcode' ..."
SELECT wt.way_id,wt.key,wt.value,hn.street_id,st.postcode
FROM way_tags AS wt
LEFT JOIN addr_housenumber AS hn ON wt.way_id=hn.way_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE wt.key='addr:postcode' AND wt.value!=st.postcode
;
.print "Test table 'way_tags' tag 'addr:city' ..."
SELECT wt.way_id,wt.key,wt.value,hn.street_id,st.city
FROM way_tags AS wt
LEFT JOIN addr_housenumber AS hn ON wt.way_id=hn.way_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE wt.key='addr:city' AND wt.value!=st.city
;
.print "Test table 'way_tags' tag 'addr:street' ..."
SELECT wt.way_id,wt.key,wt.value,hn.street_id,st.street
FROM way_tags AS wt
LEFT JOIN addr_housenumber AS hn ON wt.way_id=hn.way_id
LEFT JOIN addr_street AS st ON hn.street_id=st.street_id
WHERE wt.key='addr:street' AND wt.value!=st.street
;

