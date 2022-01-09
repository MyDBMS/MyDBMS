#pragma once

#include "Selector.h"
#include "WhereClause.h"
#include "GroupBy.h"
#include <vector>

#define TABLE_ROW_MAX 1000000000

struct SelectStmt {
public:
    std::vector<std::string> table_names;  //  多表 join
    std::vector<Selector> selectors;  //  若干投影子, 长度为 0 代表是 *
    std::vector<WhereClause> where_clauses;  //  若干选择子, 长度为 0 代表无条件
    GroupBy group_by;
    int limit;
    int offset;

    SelectStmt();

    void print_stmt();
};