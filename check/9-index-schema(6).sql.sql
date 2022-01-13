USE DB;


-- Check add index and show desc
ALTER TABLE TBL ADD INDEX (a);
DESC TBL;

-- Check error: add repeat index
ALTER TABLE TBL ADD INDEX (a);
DESC TBL;

-- Check error: add index on column not existing
ALTER TABLE TBL ADD INDEX (e);
DESC TBL;


-- Check drop index
ALTER TABLE TBL DROP INDEX (a);
DESC TBL;


-- Check error: drop index not existing
ALTER TABLE TBL DROP INDEX (a);
