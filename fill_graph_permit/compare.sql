/*
**
*/
SELECT 'records   : '||count(*) FROM graph;
SELECT 'equal     : '||count(*) FROM graph WHERE permit=permit_v2;
SELECT 'not equal : '||count(*) FROM graph WHERE permit!=permit_v2;

.print "Compare without bit 2 bike_road:"
-- 11111011 251
SELECT 'equal     : '||count(*) FROM graph WHERE permit & 251  = permit_v2 & 251;
SELECT 'not equal : '||count(*) FROM graph WHERE permit & 251 != permit_v2 & 251;

.mode table
SELECT * FROM graph WHERE permit & 251 != permit_v2 & 251 LIMIT 10;
