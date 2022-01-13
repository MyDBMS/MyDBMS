USE DB;

-- Check create table with pk
CREATE TABLE T1 (
    ID      INT,
    NAME    VARCHAR(16),
    PRIMARY KEY (ID)
);
-- Use alter instead if student didn't implement primary clause in table creation
-- ALTER TABLE T1 ADD CONSTRAINT PK PRIMARY KEY (ID);

-- Check duplication handle
INSERT INTO T1 VALUES (1, '1'), (2, '2'), (3, '3');
INSERT INTO T1 VALUES (4, '1'), (5, '2'), (6, '3');
INSERT INTO T1 VALUES (7, '7'), (4, '8'), (9, '9');
SELECT * FROM T1 WHERE T1.ID > 0;
-- 6 or 7 or or 8 records are acceptable (depending on record (7, '7') and (9, '9'))

-- Check create table with compound pk
CREATE TABLE T2 (
    ID1     INT,
    ID2     INT,
    NAME    VARCHAR(16),
    PRIMARY KEY (ID1, ID2)
);

-- Check duplication handle
INSERT INTO T2 VALUES (1, 1, '1'), (1, 2, '2'), (1, 3, '3');
INSERT INTO T2 VALUES (2, 1, '1'), (2, 2, '2'), (2, 3, '3');
INSERT INTO T2 VALUES (1, 1, '7'), (2, 2, '8'), (3, 3, '9');
SELECT * FROM T2 WHERE T2.ID1 > 0;
-- 6 or 7 records are both acceptable (depending on record (3, 3, '9'))
