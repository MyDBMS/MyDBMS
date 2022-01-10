#include "ManageSystem.h"
#include "QuerySystem.h"
#include "MyVisitor.h"
#include "SQLLexer.h"

#define SYSTEM_ROOT ("bin/dbms-test-root")

class TestSystem {
    std::stringstream iss, oss;

    StringStreamFrontend frontend{iss, oss, true};

    ManageSystem ms{ManageSystem::load_system(SYSTEM_ROOT, &frontend)};

    MyVisitor iVisitor{ms};

public:

    void exec(const std::string &in) {
        iss.clear();
        iss << in;
        while (true) {
            std::string stmt = frontend.read_stmt();
            if (stmt == "EXIT;") break;
            antlr4::ANTLRInputStream sInputStream(stmt);
            SQLLexer iLexer(&sInputStream);
            antlr4::CommonTokenStream sTokenStream(&iLexer);
            SQLParser iParser(&sTokenStream);
            iVisitor.visit(iParser.program());
        }
    }

    void expect(const std::string &in, const std::string &expected) {
        oss.str("");
        exec(in);
        if (oss.str() != expected) {
            std::cerr << "<== EXPECTED ==>" << std::endl
                      << expected << std::endl
                      << "<== ACTUAL ==>" << std::endl
                      << oss.str() << std::endl;
        }
        assert(oss.str() == expected);
    }

    void exec_and_print(const std::string &in) {
        oss.str("");
        exec(in);
        std::cout << oss.str();
    }
};

void test_system_management() {
    std::filesystem::remove_all(SYSTEM_ROOT);
    TestSystem ts;

    ts.exec(R"(CREATE DATABASE TEST_A;
USE TEST_A;
CREATE TABLE test_a1 (x INT, y INT NOT NULL);
CREATE TABLE test_a2 (a INT, b INT);)");

    ts.expect("CREATE TABLE test_a1 (m INT, n INT);", "[ERROR] Table of name test_a1 already exists.\n");

    ts.expect("SHOW TABLES;", R"(+------------------+
| Tables_in_TEST_A |
+------------------+
| test_a1          |
| test_a2          |
+------------------+
2 rows in set
)");

    ts.expect("DESC test_a1;", R"(+-------+------+------+---------+
| Field | Type | Null | Default |
+-------+------+------+---------+
| x     | INT  | YES  | NULL    |
| y     | INT  | NO   | NULL    |
+-------+------+------+---------+
2 rows in set
)");

    ts.exec(R"(CREATE DATABASE TEST_B;
USE TEST_B;
CREATE TABLE test_b1 (p INT DEFAULT 42, q FLOAT NOT NULL, r VARCHAR(24));)");

    ts.expect("DESC test_b1;", R"(+-------+-------------+------+---------+
| Field | Type        | Null | Default |
+-------+-------------+------+---------+
| p     | INT         | YES  | 42      |
| q     | FLOAT       | NO   | NULL    |
| r     | VARCHAR(24) | YES  | NULL    |
+-------+-------------+------+---------+
3 rows in set
)");

    ts.expect("DESC test_b2;", "[ERROR] Table does not exist.\n");

    ts.exec("DROP TABLE test_b1;");
    ts.expect("SHOW TABLES;", "Empty set\n");

    ts.exec("DROP DATABASE TEST_B;");
    ts.expect("SHOW TABLES;", "[ERROR] DB is not opened yet.\n");
    ts.expect("SHOW DATABASES;", R"(+----------+
| Database |
+----------+
| TEST_A   |
+----------+
1 row in set
)");

    ts.exec("USE TEST_A; DROP TABLE test_a1;");
    ts.expect("SHOW TABLES;", R"(+------------------+
| Tables_in_TEST_A |
+------------------+
| test_a2          |
+------------------+
1 row in set
)");

    ts.expect("SELECT * FROM test_a1;", "[ERROR] table name does not exist\n");

    std::filesystem::remove_all(SYSTEM_ROOT);
}

void test_integrity() {
    std::filesystem::remove_all(SYSTEM_ROOT);
    TestSystem ts;

    ts.exec(R"(CREATE DATABASE DBMS_TEST;
USE DBMS_TEST;
CREATE TABLE test_unique (
    x INT,
    y INT,
    z INT
);
ALTER TABLE test_unique ADD UNIQUE (x, y);)");

    ts.expect("INSERT INTO test_unique VALUES (1, 1, 0);", "");

    ts.expect("INSERT INTO test_unique VALUES (1, 2, NULL);", "");

    ts.expect("INSERT INTO test_unique VALUES (2, 2, NULL);", "");

    ts.expect("INSERT INTO test_unique VALUES (1, 1, 0);", "[ERROR] Unique constraint failed!\n");

    ts.expect("ALTER TABLE test_unique ADD UNIQUE (y);", "[ERROR] Duplicate values exist.\n");

    ts.expect("ALTER TABLE test_unique ADD UNIQUE (z);", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("INSERT INTO test_unique VALUES (3, 4, NULL);", "");

    ts.expect("INSERT INTO test_unique VALUES (5, 6, 1);", "");

    ts.expect("INSERT INTO test_unique VALUES (7, 8, 0);", "[ERROR] Unique constraint failed!\n");

    ts.expect("SELECT * FROM test_unique;", R"(+---------------+---------------+---------------+
| test_unique.x | test_unique.y | test_unique.z |
+---------------+---------------+---------------+
| 1             | 1             | 0             |
| 1             | 2             | NULL          |
| 2             | 2             | NULL          |
| 3             | 4             | NULL          |
| 5             | 6             | 1             |
+---------------+---------------+---------------+
5 rows in set
)");

    ts.expect("DELETE FROM test_unique WHERE test_unique.y = 2;", "");

    ts.expect("UPDATE test_unique SET x = 2 WHERE test_unique.y = 6;", "");

    ts.expect("UPDATE test_unique SET z = 0 WHERE test_unique.y = 6;", "[ERROR] Unique constraint failed!\n");

    ts.expect(R"(CREATE TABLE test_primary (
    x INT,
    y INT,
    PRIMARY KEY (x),
    PRIMARY KEY (y)
);)", "[ERROR] Too many primary fields!\n");

    ts.expect(R"(CREATE TABLE test_primary (
    x INT,
    y INT,
    PRIMARY KEY (z)
);)", "[ERROR] In primary restriction, column name of z cannot be found.\n");

    ts.expect(R"(CREATE TABLE test_primary (
    x INT,
    y INT,
    PRIMARY KEY (x)
);)", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("DESC test_primary;", R"(+-------+------+------+---------+
| Field | Type | Null | Default |
+-------+------+------+---------+
| x     | INT  | NO   | NULL    |
| y     | INT  | YES  | NULL    |
+-------+------+------+---------+
2 rows in set
PRIMARY KEY (x);
INDEX (x);
)");

    ts.expect("INSERT INTO test_primary VALUES (NULL, 0);", "[ERROR] Field cannot be null!\n");

    ts.expect("INSERT INTO test_primary VALUES (1, NULL);", "");

    ts.expect("INSERT INTO test_primary VALUES (2, 3);", "");

    ts.expect("INSERT INTO test_primary VALUES (1, NULL);", "[ERROR] Primary constraint failed!\n");

    ts.expect("ALTER TABLE test_primary ADD CONSTRAINT xy PRIMARY KEY (x, y);",
              "[ERROR] Column y has null data. Primary key cannot be set.\n");

    ts.expect("DELETE FROM test_primary WHERE test_primary.y IS NULL;", "");

    ts.expect("ALTER TABLE test_primary ADD CONSTRAINT yx PRIMARY KEY (y, x);",
              "[ERROR] Too many primary constraints! Aborting.\n");

    ts.expect("ALTER TABLE test_primary DROP PRIMARY KEY xyz;",
              "[ERROR] Primary key restriction of name xyz does not exist.\n");

    ts.expect("ALTER TABLE test_primary DROP PRIMARY KEY;", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("ALTER TABLE test_primary ADD CONSTRAINT xy PRIMARY KEY (x, y);", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("DESC test_primary;", R"(+-------+------+------+---------+
| Field | Type | Null | Default |
+-------+------+------+---------+
| x     | INT  | NO   | NULL    |
| y     | INT  | NO   | NULL    |
+-------+------+------+---------+
2 rows in set
PRIMARY KEY xy(x, y);
INDEX (x);
INDEX (y);
)");

    ts.expect("INSERT INTO test_primary VALUES (3, 3);", "");

    ts.expect("INSERT INTO test_primary VALUES (3, NULL);", "[ERROR] Field cannot be null!\n");

    ts.expect("INSERT INTO test_primary VALUES (3, 3);", "[ERROR] Primary constraint failed!\n");

    ts.expect("SELECT * FROM test_primary;", R"(+----------------+----------------+
| test_primary.x | test_primary.y |
+----------------+----------------+
| 2              | 3              |
| 3              | 3              |
+----------------+----------------+
2 rows in set
)");

    ts.expect(R"(CREATE TABLE test_foreign (
    p INT,
    q INT,
    FOREIGN KEY f (p) REFERENCES test_primary (z)
);)", "[ERROR] In foreign restriction, column name of z cannot be found.\n");

    ts.expect(R"(CREATE TABLE test_foreign (
    p INT,
    q INT,
    FOREIGN KEY f (r) REFERENCES test_primary (x)
);)", "[ERROR] In foreign restriction, column name of r cannot be found.\n");

    ts.expect(R"(CREATE TABLE test_foreign (
    p INT,
    q INT,
    FOREIGN KEY f (p, q) REFERENCES test_primary (y, x)
);)", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("DESC test_foreign;", R"(+-------+------+------+---------+
| Field | Type | Null | Default |
+-------+------+------+---------+
| p     | INT  | YES  | NULL    |
| q     | INT  | YES  | NULL    |
+-------+------+------+---------+
2 rows in set
FOREIGN KEY f(p, q) REFERENCES test_primary(y, x);
INDEX (p);
INDEX (q);
)");

    ts.expect(R"(INSERT INTO test_foreign VALUES (3, 2);)", "");

    ts.expect(R"(INSERT INTO test_foreign VALUES (4, 2);)", "[ERROR] Foreign constraint failed!\n");

    ts.expect(R"(INSERT INTO test_foreign VALUES (NULL, 0);)", "");

    ts.expect(R"(DELETE FROM test_primary WHERE test_primary.x = 2 AND test_primary.y = 3;)",
              "[ERROR] Foreign constraint failed.\n");

    ts.expect("ALTER TABLE test_foreign DROP FOREIGN KEY f;", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect(R"(DELETE FROM test_primary WHERE test_primary.x = 2 AND test_primary.y = 3;)", "");

    ts.expect("ALTER TABLE test_foreign ADD CONSTRAINT f FOREIGN KEY (q) REFERENCES test_primary (y);",
              "[ERROR] Some value cannot be found in the referenced table. Foreign constraint cannot be added.\n");

    ts.expect("ALTER TABLE test_foreign ADD CONSTRAINT f FOREIGN KEY (p) REFERENCES test_primary (x);",
              "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("INSERT INTO test_foreign VALUES (4, 3);", "[ERROR] Foreign constraint failed!\n");

    ts.expect("UPDATE test_primary SET y = 0 WHERE test_primary.y = 3;", "");  // TODO: check this!

    ts.expect("ALTER TABLE test_primary DROP PRIMARY KEY xy;",
              "[ERROR] Primary key constraint is referenced by a foreign constraint of table test_foreign, and thus cannot be dropped.\n");

    ts.expect("DROP TABLE test_primary;",
              "[ERROR] Table test_primary is referenced by test_foreign through foreign key, and cannot be dropped.\n");

    ts.expect("ALTER TABLE test_foreign DROP FOREIGN KEY f;", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("ALTER TABLE test_primary DROP PRIMARY KEY;", "[INFO] Query OK, 0 rows affected.\n");

    ts.expect("ALTER TABLE test_primary DROP PRIMARY KEY;", "[ERROR] No primary constraint exists.\n");

    std::filesystem::remove_all(SYSTEM_ROOT);
}

int main() {
    test_system_management();
    test_integrity();
    return 0;
}
