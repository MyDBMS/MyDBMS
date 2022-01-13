-- If COUNT(*) isn't implemented, use tables' pks instead
USE DATASET;

-- Check 3 tables join
SELECT SUPPLIER.S_NAME, NATION.N_NAME, REGION.R_NAME FROM SUPPLIER, NATION, REGION
 WHERE SUPPLIER.S_NATIONKEY = NATION.N_NATIONKEY 
  AND NATION.N_REGIONKEY = REGION.R_REGIONKEY
  AND REGION.R_NAME = 'AFRICA';
-- +--------------------+------------+--------+
-- | S_NAME             | N_NAME     | R_NAME |
-- +--------------------+------------+--------+
-- | Supplier#000000024 | ALGERIA    | AFRICA |
-- | Supplier#000000028 | ALGERIA    | AFRICA |
-- | Supplier#000000037 | ALGERIA    | AFRICA |
-- ............................................
-- | Supplier#000002868 | MOZAMBIQUE | AFRICA |
-- | Supplier#000002877 | MOZAMBIQUE | AFRICA |
-- | Supplier#000002918 | MOZAMBIQUE | AFRICA |
-- | Supplier#000002935 | MOZAMBIQUE | AFRICA |
-- | Supplier#000002948 | MOZAMBIQUE | AFRICA |
-- | Supplier#000002953 | MOZAMBIQUE | AFRICA |
-- +--------------------+------------+--------+
-- 583 rows in set (0.003 sec)

-- Check 4 tables join
SELECT COUNT(*) FROM PARTSUPP, SUPPLIER, NATION, REGION
 WHERE PARTSUPP.PS_SUPPKEY = SUPPLIER.S_SUPPKEY
  AND SUPPLIER.S_NATIONKEY = NATION.N_NATIONKEY 
  AND NATION.N_REGIONKEY = REGION.R_REGIONKEY
  AND REGION.R_NAME = 'AFRICA';
-- +----------+
-- | COUNT(*) |
-- +----------+
-- |    46640 |
-- +----------+
-- 1 row in set (1.933 sec)

-- Check 5 tables join
SELECT COUNT(*) FROM LINEITEM, PARTSUPP, SUPPLIER, NATION, REGION
 WHERE LINEITEM.L_SUPPKEY = PARTSUPP.PS_SUPPKEY
  AND LINEITEM.L_PARTKEY = PARTSUPP.PS_PARTKEY
  AND PARTSUPP.PS_SUPPKEY = SUPPLIER.S_SUPPKEY
  AND SUPPLIER.S_NATIONKEY = NATION.N_NATIONKEY 
  AND NATION.N_REGIONKEY = REGION.R_REGIONKEY
  AND REGION.R_NAME = 'ASIA';
-- +----------+
-- | COUNT(*) |
-- +----------+
-- |   386153 |
-- +----------+
-- 1 row in set (6.142 sec)
