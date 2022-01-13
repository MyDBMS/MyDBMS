USE DATASET;

-- Check scan speed
SELECT LINEITEM.L_PARTKEY, LINEITEM.L_SUPPKEY, LINEITEM.L_ORDERKEY, LINEITEM.L_LINENUMBER, LINEITEM.L_COMMENT FROM LINEITEM 
 WHERE LINEITEM.L_SUPPKEY < 10 AND LINEITEM.L_COMMENT >= 'z';
-- +-----------+-----------+------------+--------------+-------------------------------+
-- | L_PARTKEY | L_SUPPKEY | L_ORDERKEY | L_LINENUMBER | L_COMMENT                     |
-- +-----------+-----------+------------+--------------+-------------------------------+
-- |     28484 |         3 |    1072288 |            4 | zzle quickly according to the |
-- |     32247 |         8 |    1288486 |            5 | zle carefully bold deposits   |
-- |     30720 |         1 |    1704965 |            4 | ze boldly among th            |
-- +-----------+-----------+------------+--------------+-------------------------------+
-- 3 rows in set (11.560 sec)

-- Check build index on large data
ALTER TABLE LINEITEM ADD INDEX (L_SUPPKEY);


-- Check index speed
SELECT LINEITEM.L_PARTKEY, LINEITEM.L_SUPPKEY, LINEITEM.L_ORDERKEY, LINEITEM.L_LINENUMBER, LINEITEM.L_COMMENT FROM LINEITEM 
 WHERE LINEITEM.L_SUPPKEY < 10 AND LINEITEM.L_COMMENT >= 'z';
-- +-----------+-----------+------------+--------------+-------------------------------+
-- | L_PARTKEY | L_SUPPKEY | L_ORDERKEY | L_LINENUMBER | L_COMMENT                     |
-- +-----------+-----------+------------+--------------+-------------------------------+
-- |     28484 |         3 |    1072288 |            4 | zzle quickly according to the |
-- |     32247 |         8 |    1288486 |            5 | zle carefully bold deposits   |
-- |     30720 |         1 |    1704965 |            4 | ze boldly among th            |
-- +-----------+-----------+------------+--------------+-------------------------------+
-- 3 rows in set (2.908 sec)
