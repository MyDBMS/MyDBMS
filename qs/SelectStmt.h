#pragma once

#include "Selector.h"
#include "WhereClause.h"
#include <vector>

struct SelectStmt {
public:
    std::vector<std::string> table_names;  //  多表 join
    std::vector<Selector> selectors;  //  若干投影子, 长度为 0 代表是 *
    std::vector<WhereClause> where_clauses;  //  若干选择子, 长度为 0 代表无条件

    SelectStmt();
};