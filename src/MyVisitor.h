#pragma once

#include "../generated/SQLBaseVisitor.h"
#include "../qs/QuerySystem.h"
#include "../ms/ManageSystem.h"

class MyVisitor : public SQLBaseVisitor {
public:

    QuerySystem qs;

    ManageSystem &ms;

    explicit MyVisitor(ManageSystem &ms): ms(ms), qs(ms) {
    }
    
    Value gen_Value(SQLParser::ValueContext *ctx){
        if (ctx->Integer()){  //  是整型
            return Value::make_value(std::stoi(ctx->Integer()->getText()));
        }
        else if (ctx->String()){  //  是字符串型
            return Value::make_value(ctx->String()->getText());
        }
        else if (ctx->Null()){  //  是NULL型
            return Value::make_value();
        }
    }

    virtual antlrcpp::Any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string db_name = ctx->Identifier()->getText();
        ms.create_db(db_name);
        return res;
    }

    virtual antlrcpp::Any visitDrop_db(SQLParser::Drop_dbContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string db_name = ctx->Identifier()->getText();
        ms.drop_db(db_name);
        return res;
    }

    virtual antlrcpp::Any visitUse_db(SQLParser::Use_dbContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string db_name = ctx->Identifier()->getText();
        ms.use_db(db_name);
        return res;
    }

    virtual antlrcpp::Any visitCreate_table(SQLParser::Create_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        auto field_list_ctx = ctx->field_list();
        std::vector<Field> field_list;
        field_list.clear();
        for(auto field_ctx : field_list_ctx->field()){
            field_list.push_back(field_ctx->field_val);
        }
        ms.create_table(table_name, field_list, {}, {});  // TODO: primary keys and foreign keys
        return res;
    }

    virtual antlrcpp::Any visitNormal_field(SQLParser::Normal_fieldContext *ctx) override {
        auto res = visitChildren(ctx);
        Field field;
        field.name = ctx->Identifier()->getText();
        auto type_ctx = ctx->type_();
        if (type_ctx->children[0]->getText() == "INT"){
            field.type = Field::Type::INT;
        }
        else if (type_ctx->children[0]->getText() == "VARCHAR"){
            field.type = Field::Type::STR;
            field.str_len = std::stoi(type_ctx->Integer()->getText());
        }
        if (ctx->Null()){  //  'NOT' Null
            field.nullable = false;
        }
        else field.nullable = true;
        ctx->field_val = field;
        return res;
    }

    virtual antlrcpp::Any visitDrop_table(SQLParser::Drop_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        ms.drop_table(table_name);
        return res;
    }

    virtual antlrcpp::Any visitAlter_add_index(SQLParser::Alter_add_indexContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        std::vector<std::string> column_list;
        column_list.clear();
        for(auto column : ctx->identifiers()->Identifier()){
            std::string column_name = column->getText();
            column_list.push_back(column_name);
        }
        ms.create_index(table_name, column_list);
        return res;
    }

    virtual antlrcpp::Any visitAlter_drop_index(SQLParser::Alter_drop_indexContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        std::vector<std::string> column_list;
        column_list.clear();
        for(auto column : ctx->identifiers()->Identifier()){
            std::string column_name = column->getText();
            column_list.push_back(column_name);
        }
        ms.drop_index(table_name, column_list);
        return res;
    }

    virtual antlrcpp::Any visitInsert_into_table(SQLParser::Insert_into_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        auto value_lists_ctx = ctx->value_lists();
        auto value_list_ctx_vec = value_lists_ctx->value_list();
        for(auto value_list_ctx : value_list_ctx_vec){
            std::vector<Value> values;
            values.clear();
            auto value_ctx_vec = value_list_ctx->value();
            for(auto value_ctx : value_ctx_vec){
                /* values.push_back(gen_Value(value_ctx)); */
                values.push_back(value_ctx->value);
            }
            qs.insert_record(table_name, values);
        }
        return res;
    }

    virtual antlrcpp::Any visitSelect_table_(SQLParser::Select_table_Context *ctx) override {
        auto res = visitChildren(ctx);
        qs.search_entry(ctx->select_table()->select_stmt);
        return res;
    }

    //  TODO: 变成真正的版本
    virtual antlrcpp::Any visitSelect_table(SQLParser::Select_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->select_stmt.table_names = ctx->identifiers()->idents;
        ctx->select_stmt.selectors = ctx->selectors()->selectors;
        if (ctx->where_and_clause())
            ctx->select_stmt.where_clauses = ctx->where_and_clause()->where_clauses;
        return res;
    }

    virtual antlrcpp::Any visitSelectors(SQLParser::SelectorsContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->selectors.clear();
        if (ctx->children[0]->getText() == "*"){  //  全选
            return res;
        }
        for(auto selector_ctx : ctx->selector()){
            ctx->selectors.push_back(selector_ctx->selector);
        }
        return res;
    }

    virtual antlrcpp::Any visitSelector(SQLParser::SelectorContext *ctx) override {
        auto res = visitChildren(ctx);
        if (ctx->column()){  // 直接投影一列
            ctx->selector.type = Selector::Type::COL;
            ctx->selector.col = ctx->column()->column;
        }
        return res;
    }

    virtual antlrcpp::Any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->idents.clear();
        for(auto ident : ctx->Identifier())
            ctx->idents.push_back(ident->getText());
        return res;
    }

    virtual antlrcpp::Any visitWhere_and_clause(SQLParser::Where_and_clauseContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->where_clauses.clear();
        for(auto where_clause_ctx : ctx->where_clause())
            ctx->where_clauses.push_back(where_clause_ctx->where_clause);
        return res;
    }

    virtual antlrcpp::Any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->where_clause.type = WhereClause::Type::OP_EXPR;
        ctx->where_clause.column = ctx->column()->column;
        ctx->where_clause.op_type = ctx->operator_()->op_type;
        ctx->where_clause.expr = ctx->expression()->expr;
        /* ctx->column_name = (ctx->column()->Identifier())[1]->getText();
        ctx->key = std::stoi(ctx->expression()->value()->Integer()->getText()); */
        return res;
    }

    virtual antlrcpp::Any visitWhere_operator_select(SQLParser::Where_operator_selectContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->where_clause.type = WhereClause::Type::OP_SELECT;
        ctx->where_clause.column = ctx->column()->column;
        ctx->where_clause.op_type = ctx->operator_()->op_type;
        ctx->where_clause.select_result = qs.search(ctx->select_table()->select_stmt);
        return res;
    }

    virtual antlrcpp::Any visitWhere_null(SQLParser::Where_nullContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->where_clause.type = WhereClause::Type::IS_NULL_OR_NOT_NULL;
        ctx->where_clause.column = ctx->column()->column;
        ctx->where_clause.null_or_not_null = (ctx->children[2]->getText() != "NOT");
        return res;
    }

    virtual antlrcpp::Any visitWhere_in_list(SQLParser::Where_in_listContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->where_clause.type = WhereClause::Type::IN_VALUE_LIST;
        ctx->where_clause.column = ctx->column()->column;
        ctx->where_clause.value_list = ctx->value_list()->value_list;
        return res;
    }

    virtual antlrcpp::Any visitWhere_in_select(SQLParser::Where_in_selectContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->where_clause.type = WhereClause::Type::IN_SELECT;
        ctx->where_clause.column = ctx->column()->column;
        ctx->where_clause.select_result = qs.search(ctx->select_table()->select_stmt);
        return res;
    }

    virtual antlrcpp::Any visitWhere_like_string(SQLParser::Where_like_stringContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->where_clause.type = WhereClause::Type::LIKE_STR;
        ctx->where_clause.column = ctx->column()->column;
        ctx->where_clause.str = ctx->String()->getText();
        return res;
    }

    virtual antlrcpp::Any visitValue_list(SQLParser::Value_listContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->value_list.clear();
        for(auto value_ctx : ctx->value()){
            ctx->value_list.push_back(value_ctx->value);
        }
        return res;
    }

    virtual antlrcpp::Any visitValue(SQLParser::ValueContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->value = gen_Value(ctx);
        return res;
    }

    virtual antlrcpp::Any visitColumn(SQLParser::ColumnContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->column.table_name = (ctx->Identifier())[0]->getText();
        ctx->column.column_name = (ctx->Identifier())[1]->getText();
        return res;
    }

    virtual antlrcpp::Any visitExpression(SQLParser::ExpressionContext *ctx) override {
        auto res = visitChildren(ctx);
        if (ctx->value()){
            ctx->expr.type = Expr::Type::VALUE;
            ctx->expr.value = ctx->value()->value;
        }
        else{
            ctx->expr.type = Expr::Type::COLUMN;
            ctx->expr.column = ctx->column()->column;
        }
        return res;
    }

    virtual antlrcpp::Any visitOperator_(SQLParser::Operator_Context *ctx) override {
        auto res = visitChildren(ctx);
        if (ctx->EqualOrAssign()){
            ctx->op_type = WhereClause::OP_Type::EQ;
        }
        else if (ctx->Less()){
            ctx->op_type = WhereClause::OP_Type::LT;
        }
        else if (ctx->LessEqual()){
            ctx->op_type = WhereClause::OP_Type::LE;
        }
        else if (ctx->Greater()){
            ctx->op_type = WhereClause::OP_Type::GT;
        }
        else if (ctx->GreaterEqual()){
            ctx->op_type = WhereClause::OP_Type::GE;
        }
        else if (ctx->NotEqual()){
            ctx->op_type = WhereClause::OP_Type::NEQ;
        }
        return res;
    }
};