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

    ts.exec("USE TEST_A; DROP TABLE test_a1;");
    ts.expect("SHOW TABLES;", R"(+------------------+
| Tables_in_TEST_A |
+------------------+
| test_a2          |
+------------------+
1 row in set
)");

    std::filesystem::remove_all(SYSTEM_ROOT);
}

void test_integrity() {
    std::filesystem::remove_all(SYSTEM_ROOT);
    TestSystem ts;

    ts.exec(R"(CREATE DATABASE DBMS_TEST;
USE DBMS_TEST;
CREATE TABLE test (
    x INT,
    y INT,
    PRIMARY KEY (x)
);)");

    ts.expect("DESC test;", R"(+-------+------+------+---------+
| Field | Type | Null | Default |
+-------+------+------+---------+
| x     | INT  | NO   | NULL    |
| y     | INT  | YES  | NULL    |
+-------+------+------+---------+
2 rows in set
PRIMARY KEY (x);
)");

    ts.expect("SELECT * FROM test;", "Empty set\n");

    std::filesystem::remove_all(SYSTEM_ROOT);
}

int main() {
    test_system_management();
    test_integrity();
    return 0;
}
