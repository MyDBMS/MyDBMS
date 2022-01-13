USE DB;

-- Check single fk
CREATE TABLE T3 (
    ID INT,
    T1_ID INT,
    NAME VARCHAR(16),
    FOREIGN KEY (T1_ID) REFERENCES T1(ID)
);
CREATE TABLE T4 (
    ID INT,
    T3_ID INT,
    NAME VARCHAR(16),
    FOREIGN KEY (T3_ID) REFERENCES T3(ID)
);

-- Check compound fk
CREATE TABLE T5 (
    ID INT,
    T2_ID1 INT,
    T2_ID2 INT,
    NAME VARCHAR(16),
    FOREIGN KEY (T2_ID1, T2_ID2) REFERENCES T2(ID1, ID2)
);

-- Check basic insertion (and prepare data for next checks)
INSERT INTO T1 VALUES (100, '100'), (101, '101'), (102, '102');
INSERT INTO T3 VALUES (10, 100, '10_100'), (11, 101, '11_101'), (12, 102, '12_102');
INSERT INTO T4 VALUES (0, 10, '0_10_100'), (1, 11, '1_11_101');
INSERT INTO T2 VALUES (10, 10, '10_10'), (10, 11, '10_11'), (11, 10, '11_10'), (11, 11, '11_11');
INSERT INTO T5 VALUES (1, 10, 10, '1_10_10'), (2, 10, 10, '2_10_10'), (3, 10, 11, '3_10_11'), (4, 11, 10, '4_11_10');

-- Check table update or insert integrity
UPDATE T4 SET T3_ID = 12, NAME = '1_12_102' WHERE T4.ID = 1;
INSERT INTO T4 VALUES (2, 12, '2_12_102');
UPDATE T4 SET T3_ID = 13, NAME = '1_13_10?' WHERE T4.ID = 1;
INSERT INTO T4 VALUES (3, 13, '2_13_10?');
SELECT * FROM T4;
-- Line 1, 2 succeeded but line 3, 4 failed

-- Check reference table update or delete integrity
UPDATE T2 SET ID2 = 12 WHERE T2.ID1 = 10 AND T2.ID2 = 10;
UPDATE T2 SET ID2 = 12 WHERE T2.ID1 = 11 AND T2.ID2 = 11;
DELETE FROM T2 WHERE T2.ID2 = 11;
DELETE FROM T2 WHERE T2.ID2 = 12;
-- Line 1, 3 failed but line 2, 4 succeeded 
