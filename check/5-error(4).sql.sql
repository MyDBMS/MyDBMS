USE DATASET;

-- Check error: no such table or column on query
SELECT * FROM NO_SUCH_TABLE WHERE NO_SUCH_TABLE.ANYTHING >= 0;
SELECT REGION.NO_SUCH_COLUMN FROM REGION WHERE REGION.R_REGIONKEY >= 0;

-- Check error: table not existing on insertion
INSERT INTO NO_SUCH_TABLE VALUES (1, 2, 3);

USE DB;

-- Check error: value list length mismatch on insertion
INSERT INTO TBL VALUES (7, '7', 7.0, '777');
SELECT * FROM TBL WHERE TBL.a >= 0;

-- Check error: value type mismatch on insertion
INSERT INTO TBL VALUES (7, '7', 7.0), ('d8', 8.0, 8.0), ('9', 9, 9);
SELECT * FROM TBL WHERE TBL.a >= 0;
-- Type conversion depends on impletation, but anyway you shouldn't convert 'd123' into int
