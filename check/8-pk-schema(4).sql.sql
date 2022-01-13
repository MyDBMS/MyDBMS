USE DB;

-- Prepare data
CREATE TABLE T0 (
    ID      INT,
    ID2     INT,
    NAME    VARCHAR(16)
);
INSERT INTO T0 VALUES (1, 1, '1'), (2, 2, '2'), (3, 3, '3'), (1, 4, '4');
SELECT * FROM T0 WHERE T0.ID > 0 ;

-- Check error: duplication on creating pk
ALTER TABLE T0 ADD CONSTRAINT PK PRIMARY KEY (ID);

-- Check add pk
DELETE FROM T0 WHERE T0.NAME = '4';
ALTER TABLE T0 ADD CONSTRAINT PK PRIMARY KEY (ID);
DESC T0;

-- Check error: one table has only one pk at most
ALTER TABLE T0 ADD CONSTRAINT PK PRIMARY KEY (ID, ID2);


-- Check drop pk
INSERT INTO T0 VALUES (1, 4, '4');
ALTER TABLE T0 DROP PRIMARY KEY;
INSERT INTO T0 VALUES (1, 4, '4');
SELECT * FROM T0 WHERE T0.ID > 0 ;

-- Check error: can't drop pk if there isn't one
ALTER TABLE T0 DROP PRIMARY KEY PK;

-- Check create compound pk
ALTER TABLE T0 ADD CONSTRAINT PK PRIMARY KEY (ID, ID2);
DESC T0;
INSERT INTO T0 VALUES (1, 4, '5');

