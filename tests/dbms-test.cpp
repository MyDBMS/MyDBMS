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

    ts.exec(R"(CREATE TABLE test_primary (
    x INT,
    y INT,
    PRIMARY KEY (x)
);)");

    ts.expect("DESC test_primary;", R"(+-------+------+------+---------+
| Field | Type | Null | Default |
+-------+------+------+---------+
| x     | INT  | NO   | NULL    |
| y     | INT  | YES  | NULL    |
+-------+------+------+---------+
2 rows in set
PRIMARY KEY (x);
)");

    std::filesystem::remove_all(SYSTEM_ROOT);
}

int main() {
    test_system_management();
    test_integrity();
    return 0;
}
