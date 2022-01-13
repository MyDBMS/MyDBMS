-- Check fuzzy match
SELECT CUSTOMER.C_NAME, CUSTOMER.C_ACCTBAL FROM CUSTOMER WHERE CUSTOMER.C_NAME LIKE '%#00%123_';
-- +--------------------+-----------+
-- | C_NAME             | C_ACCTBAL |
-- +--------------------+-----------+
-- | Customer#000001230 |   4787.85 |
-- | Customer#000001231 |   2326.68 |
-- | Customer#000001232 |   8482.51 |
-- | Customer#000001233 |   3649.49 |
-- | Customer#000001234 |   -982.32 |
-- | Customer#000001235 |   -982.05 |
-- | Customer#000001236 |   3600.95 |
-- ..................................
-- | Customer#000041234 |   1286.19 |
-- | Customer#000041235 |    -12.66 |
-- | Customer#000041236 |   5088.25 |
-- | Customer#000041237 |    2212.4 |
-- | Customer#000041238 |   1772.43 |
-- | Customer#000041239 |   -776.74 |
-- +--------------------+-----------+
-- 50 rows in set (0.021 sec)
-- Notice: name suffix should be [0-4]23[0-9]

-- Check aggregate query
SELECT MIN(SUPPLIER.S_NATIONKEY),MAX(SUPPLIER.S_ACCTBAL),SUM(SUPPLIER.S_SUPPKEY),COUNT(*) FROM SUPPLIER WHERE SUPPLIER.S_SUPPKEY < 100;
-- +------------------+----------------+----------------+----------+
-- | MIN(S_NATIONKEY) | MAX(S_ACCTBAL) | SUM(S_SUPPKEY) | COUNT(*) |
-- +------------------+----------------+----------------+----------+
-- |                0 |        9915.24 |           4950 |       99 |
-- +------------------+----------------+----------------+----------+
-- 1 row in set (0.001 sec)

-- Check group query
SELECT SUPPLIER.S_NATIONKEY, COUNT(*), AVG(SUPPLIER.S_ACCTBAL) FROM SUPPLIER WHERE SUPPLIER.S_SUPPKEY < 1000 GROUP BY SUPPLIER.S_NATIONKEY;
-- +-------------+----------+-------------------------+
-- | S_NATIONKEY | COUNT(*) | AVG(SUPPLIER.S_ACCTBAL) |
-- +-------------+----------+-------------------------+
-- |           0 |       36 |       4855.807753245036 |
-- |           1 |       38 |       3700.886332562095 |
-- |           2 |       43 |       4510.536243460899 |
-- |           3 |       37 |       4617.043751690839 |
-- |           4 |       40 |       4750.436246490479 |
-- |           5 |       33 |       4155.503030950373 |
-- |           6 |       35 |       4102.685137939453 |
-- |           7 |       50 |       4444.547816772461 |
-- |           8 |       47 |       4319.385726279401 |
-- |           9 |       45 |       4706.950181664361 |
-- |          10 |       40 |       4902.865003633499 |
-- |          11 |       43 |       4281.248566206112 |
-- |          12 |       41 |       4366.978092100562 |
-- |          13 |       28 |       3830.519643511091 |
-- |          14 |       36 |       4339.997760772705 |
-- |          15 |       40 |       4923.685042190552 |
-- |          16 |       34 |       4998.674407958984 |
-- |          17 |       39 |       4679.288697854066 |
-- |          18 |       53 |       4228.536070661725 |
-- |          19 |       33 |       4212.968801787405 |
-- |          20 |       47 |       4684.638092041016 |
-- |          21 |       39 |       4139.731800250518 |
-- |          22 |       47 |       4802.844003230967 |
-- |          23 |       39 |       3925.545353816106 |
-- |          24 |       36 |      5062.0788820054795 |
-- +-------------+----------+-------------------------+
-- 25 rows in set (0.001 sec)

-- Check limit-offset
SELECT LINEITEM.L_ORDERKEY, LINEITEM.L_LINENUMBER FROM LINEITEM WHERE LINEITEM.L_ORDERKEY < 10 LIMIT 10;
SELECT LINEITEM.L_ORDERKEY, LINEITEM.L_LINENUMBER FROM LINEITEM WHERE LINEITEM.L_ORDERKEY < 10 LIMIT 10 OFFSET 10;
SELECT LINEITEM.L_ORDERKEY, LINEITEM.L_LINENUMBER FROM LINEITEM WHERE LINEITEM.L_ORDERKEY < 10 LIMIT 10 OFFSET 20;
SELECT LINEITEM.L_ORDERKEY, LINEITEM.L_LINENUMBER FROM LINEITEM WHERE LINEITEM.L_ORDERKEY < 10 LIMIT 10 OFFSET 30;
-- There shoud be 25 rows in total, and no intersection between two query
-- So 4 queries return 10, 10, 5, 0 rows respectively
-- Total results are:
-- +------------+--------------+
-- | L_ORDERKEY | L_LINENUMBER |
-- +------------+--------------+
-- |          1 |            1 |
-- |          1 |            2 |
-- |          1 |            3 |
-- |          1 |            4 |
-- |          1 |            5 |
-- |          1 |            6 |
-- |          2 |            1 |
-- |          3 |            1 |
-- |          3 |            2 |
-- |          3 |            3 |
-- |          3 |            4 |
-- |          3 |            5 |
-- |          3 |            6 |
-- |          4 |            1 |
-- |          5 |            1 |
-- |          5 |            2 |
-- |          5 |            3 |
-- |          6 |            1 |
-- |          7 |            1 |
-- |          7 |            2 |
-- |          7 |            3 |
-- |          7 |            4 |
-- |          7 |            5 |
-- |          7 |            6 |
-- |          7 |            7 |
-- +------------+--------------+
-- 25 rows in set (0.000 sec)

-- Check recursive query
SELECT NATION.N_NATIONKEY, NATION.N_NAME FROM NATION WHERE NATION.N_REGIONKEY = 
 (SELECT REGION.R_REGIONKEY FROM REGION WHERE REGION.R_NAME = 'ASIA') 
 AND NATION.N_NATIONKEY < 1000;
-- +-------------+-----------+
-- | N_NATIONKEY | N_NAME    |
-- +-------------+-----------+
-- |           8 | INDIA     |
-- |           9 | INDONESIA |
-- |          12 | JAPAN     |
-- |          18 | CHINA     |
-- |          21 | VIETNAM   |
-- +-------------+-----------+
-- 5 rows in set (0.001 sec)

-- Check deep recursive query
SELECT COUNT(*) FROM PARTSUPP.PARTSUPP WHERE PARTSUPP.PS_SUPPKEY IN (
    SELECT SUPPLIER.S_SUPPKEY FROM SUPPLIER WHERE SUPPLIER.S_NATIONKEY IN (
        SELECT NATION.N_NATIONKEY FROM NATION WHERE NATION.N_REGIONKEY IN (
            SELECT REGION.R_REGIONKEY FROM REGION WHERE REGION.R_NAME = 'ASIA'
        )
    )
);
-- +----------+
-- | COUNT(*) |
-- +----------+
-- |    51440 |
-- +----------+
-- 1 row in set (0.016 sec)
