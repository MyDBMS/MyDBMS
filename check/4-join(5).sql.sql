USE DATASET;

-- Check two table join
SELECT NATION.N_NAME, NATION.N_NATIONKEY FROM NATION, REGION WHERE NATION.N_NATIONKEY > 20 AND REGION.R_NAME = 'ASIA';
-- +----------------+-------------+
-- | N_NAME         | N_NATIONKEY |
-- +----------------+-------------+
-- | VIETNAM        |          21 |
-- | RUSSIA         |          22 |
-- | UNITED KINGDOM |          23 |
-- | UNITED STATES  |          24 |
-- +----------------+-------------+
-- 4 rows in set (0.001 sec)

-- Check Join with multiple condition
SELECT LINEITEM.L_ORDERKEY, LINEITEM.L_LINENUMBER, PARTSUPP.PS_PARTKEY, PARTSUPP.PS_SUPPKEY, PARTSUPP.PS_SUPPLYCOST FROM LINEITEM, PARTSUPP 
 WHERE LINEITEM.L_PARTKEY = PARTSUPP.PS_PARTKEY AND LINEITEM.L_SUPPKEY = PARTSUPP.PS_SUPPKEY AND LINEITEM.L_ORDERKEY < 3;
-- +------------+--------------+------------+------------+---------------+
-- | L_ORDERKEY | L_LINENUMBER | PS_PARTKEY | PS_SUPPKEY | PS_SUPPLYCOST |
-- +------------+--------------+------------+------------+---------------+
-- |          1 |            1 |      46557 |       2323 |        868.47 |
-- |          1 |            2 |      20193 |       2194 |        401.49 |
-- |          1 |            3 |      19110 |       1111 |        954.82 |
-- |          1 |            4 |        640 |       1391 |        535.48 |
-- |          1 |            5 |       7208 |        465 |        329.99 |
-- |          1 |            6 |       4691 |        194 |        929.43 |
-- |          2 |            1 |      31851 |        372 |        516.25 |
-- +------------+--------------+------------+------------+---------------+
-- 7 rows in set (0.001 sec)
