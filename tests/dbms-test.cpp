#include "ManageSystem.h"
#include "QuerySystem.h"
#include "MyVisitor.h"
#include "SQLLexer.h"

#define SYSTEM_ROOT ("bin/dbms-test-root")

class TestSystem {
    std::stringstream iss, oss;

    StringStreamFrontend frontend{iss, oss};

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
        assert(oss.str() == expected);
    }

    void exec_and_print(const std::string &in) {
        oss.str("");
        exec(in);
        std::cout << oss.str();
    }
};

void test() {
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

    std::filesystem::remove_all(SYSTEM_ROOT);
}

int main() {
    test();
    return 0;
}
