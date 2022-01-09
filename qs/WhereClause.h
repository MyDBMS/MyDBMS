#pragma once

#include "Column.h"
#include "../ms/Value.h"
#include "RecordSet.h"

#include <vector>

struct Expr {
    enum Type{
        VALUE,
        COLUMN
    } type;

    Value value;
    Column column;

    Expr();

    Expr(const Expr &rhs);

    Expr& operator=(const Expr &expr);
};

struct WhereClause {
public:
    enum Type {
        OP_EXPR,
        OP_SELECT,
        IS_NULL_OR_NOT_NULL,
        IN_VALUE_LIST,
        IN_SELECT,
        LIKE_STR
    } type;

    Column column;

    enum OP_Type {
        EQ,
        LT,
        LE,
        GT,
        GE,
        NEQ
    } op_type;

    Expr expr;
    /* SelectStmt* select_stmt; */  //  OP_SELECT 和 IN_SELECT 要用
    RecordSet select_result;  //  OP_SELECT 和 IN_SELECT 用的
    bool null_or_not_null;  //  true -> column Is NULL false -> colum Is NOT NULL
    std::vector<Value> value_list;
    std::string str;  //  LIKE_STR 里的字符串

    bool is_solved;

    WhereClause();

    void print_wc();
};