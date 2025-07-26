/*
**
*/
SELECT 'records   : '||count(*) FROM graph;
SELECT 'equal     : '||count(*) FROM graph WHERE permit=permit_v2;
SELECT 'not equal : '||count(*) FROM graph WHERE permit!=permit_v2;
.mode table
SELECT * FROM graph WHERE permit!=permit_v2 LIMIT 10;
