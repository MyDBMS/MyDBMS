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
        ms.create_table(table_name, field_list);
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
                values.push_back(gen_Value(value_ctx));
            }
            qs.insert_record(table_name, values);
        }
        return res;
    }

    //  TODO: 变成真正的版本
    virtual antlrcpp::Any visitSelect_table(SQLParser::Select_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        //  无视selectors，视为 * 
        //  表视为只有一个
        std::string table_name = (ctx->identifiers()->Identifier())[0]->getText();
        //  where and clause 视为有且仅有一项
        auto wc_ctx = (ctx->where_and_clause()->where_clause())[0];
        if (wc_ctx->wc_type == 1){
            qs.search(table_name, wc_ctx->column_name, wc_ctx->key);
        }
        return res;
    }

    virtual antlrcpp::Any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->wc_type = 1;
        //  默认是 column = expr,文法里 column 是 table.column，取第二个 ident
        ctx->column_name = (ctx->column()->Identifier())[1]->getText();
        ctx->key = std::stoi(ctx->expression()->value()->Integer()->getText());
        return res;
    }
};