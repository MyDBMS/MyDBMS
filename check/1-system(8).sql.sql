-- Check basic creation
-- NOTICE: database "DB" will be used in feature
CREATE DATABASE DB;
CREATE DATABASE DB1;
CREATE DATABASE DB2;

-- Check list all dbs
SHOW DATABASES;
-- If it's not implemented, check directory when in need

-- Check error: create database already existing
CREATE DATABASE DB1;
SHOW DATABASES;
-- The first line should get error (or no error but keep origin database)
-- the second line should show only one database named "DB2"

-- Check switch db
USE DB;
USE DB2;

-- Check switch db when using some db
CREATE DATABASE DB3;
SHOW DATABASES;

-- Check drop unused db
DROP DATABASE DB1;
SHOW DATABASES;

-- Check error/feature: drop using db
USE DB3;
DROP DATABASE DB3;
-- (supposed to deny to execute or switch to none)

-- Check error: drop db not existings
DROP DATABASE DB4;

