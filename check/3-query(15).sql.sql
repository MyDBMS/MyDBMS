-- Do some read-only operations first
USE DATASET;

-- Notice: omit table name when doing signle table query only if it's implemented

-- Check select *
-- Notice: our grammar must have WHERE clause, so it's fine if any clause without WHERE is implemented
SELECT * FROM REGION WHERE REGION.R_REGIONKEY >= 0;
-- +-------------+-------------+---------------------------------------------------------------------------------------------------------------------+
-- | R_REGIONKEY | R_NAME      | R_COMMENT                                                                                                           |
-- +-------------+-------------+---------------------------------------------------------------------------------------------------------------------+
-- |           0 | AFRICA      | lar deposits. blithely final packages cajole. regular waters are final requests. regular accounts are according to  |
-- |           1 | AMERICA     | hs use ironic  even requests. s                                                                                     |
-- |           2 | ASIA        | ges. thinly even pinto beans ca                                                                                     |
-- |           3 | EUROPE      | ly final courts cajole furiously final excuse                                                                       |
-- |           4 | MIDDLE EAST | uickly special accounts cajole carefully blithely close requests. carefully final asymptotes haggle furiousl        |
-- +-------------+-------------+---------------------------------------------------------------------------------------------------------------------+
-- 5 rows in set (0.000 sec)

-- Check single result where-and clause
SELECT REGION.R_REGIONKEY FROM REGION WHERE REGION.R_REGIONKEY < 3 AND REGION.R_REGIONKEY > 1;

-- Check no matching query
SELECT REGION.R_REGIONKEY FROM REGION WHERE REGION.R_REGIONKEY < 6 AND REGION.R_REGIONKEY > 1 AND REGION.R_NAME = 'AMERICA';

-- Check float comparison
-- Notice: There are two records with S_ACCTBAL = -963.19, -963.79
SELECT SUPPLIER.S_SUPPKEY, SUPPLIER.S_ACCTBAL FROM SUPPLIER WHERE SUPPLIER.S_ACCTBAL <= -963.2;
-- +-----------+-----------+
-- | S_SUPPKEY | S_ACCTBAL |
-- +-----------+-----------+
-- |        22 |    -966.2 |
-- |        65 |   -963.79 |
-- |      1458 |   -980.21 |
-- |      1654 |   -988.37 |
-- |      1764 |   -990.13 |
-- |      1845 |   -971.99 |
-- |      1870 |   -989.05 |
-- |      1907 |   -987.45 |
-- |      2908 |   -986.42 |
-- +-----------+-----------+
-- 9 rows in set (0.001 sec)

-- Check a more complex query
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


-- Then let's have fun by writing options
USE DB;


-- Check normal insertion
INSERT INTO TBL VALUES (1, '1', 1.0);
SELECT * FROM TBL;

-- Check multiple insertion
INSERT INTO TBL VALUES (2, '2', 2.0), (3, '3', 3.0), (4, '4', 4.0), (5, '5', 5.0), (6, '6', 6.0);
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check duplication insertion when no pk or unique constraint
INSERT INTO TBL VALUES (2, '2', 2.0), (3, '3', 3.0);
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check one record update
UPDATE TBL SET b = '66' WHERE TBL.b = '6';
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check multiple records and multiple fields update 
UPDATE TBL SET b = '10', c = 10.0 WHERE TBL.a > 3 AND TBL.a < 6;
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check update nothing
UPDATE TBL SET b = '999' WHERE TBL.a >= 999;
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check normal deletion
DELETE FROM TBL WHERE TBL.a = 3;
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check range deletion
DELETE FROM TBL WHERE TBL.a < 3;
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check empty deletion
DELETE FROM TBL WHERE TBL.a >= 100;
SELECT * FROM TBL WHERE TBL.a >= 0;




