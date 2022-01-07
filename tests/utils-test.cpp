#include "../utils/Frontend.h"

void test_frontend() {
    std::strstream ss;
    auto frontend = StrStreamFrontend(ss);
    Frontend::Table table;
    table.push_back({"abc", {"123", "45"}});
    table.push_back({"def233", {"666", "long long long"}});
    frontend.print_table(table);
    std::string expected = R"(+-----+----------------+
| abc | def233         |
+-----+----------------+
| 123 | 666            |
| 45  | long long long |
+-----+----------------+
2 rows in set
)";
    assert(ss.str() == expected);
}

int main() {
    test_frontend();
    return 0;
}
