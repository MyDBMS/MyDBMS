#include "WhereClause.h"

#include <cstdio>
#include <iostream>

Expr::Expr(): value(Value::make_value()){

}

Expr::Expr(const Expr &rhs): value(Value(rhs.value)){
    type = rhs.type;
    column = rhs.column;
}

Expr& Expr::operator=(const Expr& expr){
    type = expr.type;
    value = expr.value;
    column = expr.column;
    return *this;
}

WhereClause::WhereClause(){
    value_list.clear();
    is_solved = false;
}

void WhereClause::print_wc(){
    printf("WC: ");
    if (type == Type::OP_EXPR){
        std::cout << column.table_name << "." << column.column_name << " ";
        if (expr.type == Expr::Type::VALUE)
            std::cout << "Value\n";
        else
            std::cout << expr.column.table_name << "." << expr.column.column_name << " ";
    }
}