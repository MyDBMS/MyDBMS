USE DATASET;

-- Check desc schema
DESC LINEITEM;
-- +-----------------+-------------+------+---------+
-- | Field           | Type        | Null | Default |
-- +-----------------+-------------+------+---------+
-- | L_ORDERKEY      | INT         | NO   | NULL    |
-- | L_PARTKEY       | INT         | NO   | NULL    |
-- | L_SUPPKEY       | INT         | NO   | NULL    |
-- | L_LINENUMBER    | INT         | NO   | NULL    |
-- | L_QUANTITY      | FLOAT       | YES  | NULL    |
-- | L_EXTENDEDPRICE | FLOAT       | YES  | NULL    |
-- | L_DISCOUNT      | FLOAT       | YES  | NULL    |
-- | L_TAX           | FLOAT       | YES  | NULL    |
-- | L_RETURNFLAG    | VARCHAR(1)  | YES  | NULL    |
-- | L_LINESTATUS    | VARCHAR(1)  | YES  | NULL    |
-- | L_SHIPINSTRUCT  | VARCHAR(25) | YES  | NULL    |
-- | L_SHIPMODE      | VARCHAR(10) | YES  | NULL    |
-- | L_COMMENT       | VARCHAR(44) | YES  | NULL    |
-- +-----------------+-------------+------+---------+
-- PRIMARY KEY (L_PARTKEY, L_SUPPKEY);
-- FOREIGN KEY (L_ORDERKEY) REFERENCES ORDERS(O_ORDERKEY);
-- FOREIGN KEY (L_PARTKEY,L_SUPPKEY) REFERENCES PARTSUPP(PS_PARTKEY,PS_SUPPKEY);

-- Check list table names
SHOW TABLES;

USE DB;

-- Check simple table creation
-- Notice: table TBL will be used in feature
CREATE TABLE TBL (a INT NOT NULL, b VARCHAR(16) NOT NULL, c FLOAT NOT NULL);
CREATE TABLE TBL2 (a INT NOT NULL, b VARCHAR(16) NOT NULL, c FLOAT NOT NULL);
SHOW TABLES;
DESC TBL;


-- Check error: create table already existing
CREATE TABLE TBL (a2 INT NOT NULL, b2 VARCHAR(16) NOT NULL, c2 FLOAT NOT NULL);
SHOW TABLES;
-- The first line should get error (or no error but keeping origin TBL)
-- the second line should show only one table named "TBL"


-- Check drop table 
DROP TABLE TBL2;
SHOW TABLES;

-- Check error: drop table not exsiting
DROP TABLE NO_SUCH_TABLE;

