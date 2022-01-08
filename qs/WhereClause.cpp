#include "WhereClause.h"

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
}