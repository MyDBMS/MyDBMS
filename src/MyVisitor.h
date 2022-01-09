#pragma once

#include "../generated/SQLBaseVisitor.h"
#include "../qs/QuerySystem.h"
#include "../ms/ManageSystem.h"

class MyVisitor : public SQLBaseVisitor {
public:

    QuerySystem qs;

    ManageSystem &ms;

    explicit MyVisitor(ManageSystem &ms): ms(ms), qs(ms) {
        ms.qs = &qs;
    }
    
    Value gen_Value(SQLParser::ValueContext *ctx){
        if (ctx->Integer()){  //  是整型
            return Value::make_value(std::stoi(ctx->Integer()->getText()));
        }
        else if (ctx->String()){  //  是字符串型
            std::string v = ctx->String()->getText();
            return Value::make_value(v.substr(1, v.length() - 2));
        }
        else if (ctx->Float()){  //  是浮点数型
            return Value::make_value(std::stof(ctx->Float()->getText()));
        }
        else if (ctx->Null()){  //  是NULL型
            return Value::make_value();
        }
        else assert(false);
    }

    virtual antlrcpp::Any visitStatement(SQLParser::StatementContext *ctx) override {
        qs.flag = true;  //  语句开始前，将 qs 的 flag 设为 true ，表示还没有错误发生，qs flag 赋值的唯一处
        auto res = visitChildren(ctx);
        return res;
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

    virtual antlrcpp::Any visitShow_dbs(SQLParser::Show_dbsContext *ctx) override {
        auto res = visitChildren(ctx);
        ms.show_dbs();
        return res;
    }

    virtual antlrcpp::Any visitUse_db(SQLParser::Use_dbContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string db_name = ctx->Identifier()->getText();
        ms.use_db(db_name);
        return res;
    }

    virtual antlrcpp::Any visitShow_tables(SQLParser::Show_tablesContext *ctx) override {
        auto res = visitChildren(ctx);
        ms.show_tables();
        return res;
    }

    virtual antlrcpp::Any visitShow_indexes(SQLParser::Show_indexesContext *ctx) override {
        auto res = visitChildren(ctx);
        ms.show_indexes();
        return res;
    }

    virtual antlrcpp::Any visitCreate_table(SQLParser::Create_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        auto field_list_ctx = ctx->field_list();
        std::vector<Field> field_list;
        std::vector<PrimaryField> primary_field_list;
        std::vector<ForeignField> foreign_field_list;
        for(auto field_ctx : field_list_ctx->field()){
            switch (field_ctx->field_type) {
                case SQLParser::FieldContext::BASIC:
                    field_list.push_back(field_ctx->field_val);
                    break;
                case SQLParser::FieldContext::PRIMARY:
                    primary_field_list.push_back(field_ctx->primary_field_val);
                    break;
                case SQLParser::FieldContext::FOREIGN:
                    foreign_field_list.push_back(field_ctx->foreign_field_val);
                    break;
                default:
                    assert(false);
            }
        }
        ms.create_table(table_name, field_list, primary_field_list, foreign_field_list);
        return res;
    }

    virtual antlrcpp::Any visitNormal_field(SQLParser::Normal_fieldContext *ctx) override {
        auto res = visitChildren(ctx);
        Field field;
        field.name = ctx->Identifier()->getText();
        auto type_ctx = ctx->type_();
        if (type_ctx->children[0]->getText() == "INT"){
            field.type = Field::Type::INT;
            if (ctx->value() != nullptr) {  // DEFAULT
                field.has_def = true;
                field.def_int = std::stoi(ctx->value()->Integer()->getText());
            }
            else field.has_def = false;
        }
        else if (type_ctx->children[0]->getText() == "VARCHAR"){
            field.type = Field::Type::STR;
            field.str_len = std::stoi(type_ctx->Integer()->getText());
            if (ctx->value() != nullptr) {  // DEFAULT
                field.has_def = true;
                field.def_str = ctx->value()->String()->getText();
            }
            else field.has_def = false;
        }
        else if (type_ctx->children[0]->getText() == "FLOAT"){
            field.type = Field::Type::FLOAT;
            if (ctx->value() != nullptr) {  // DEFAULT
                field.has_def = true;
                field.def_float = std::stof(ctx->value()->Float()->getText());
            }
            else field.has_def = false;
        }
        if (ctx->Null()){  //  'NOT' Null
            field.nullable = false;
        }
        else field.nullable = true;
        ctx->field_val = field;
        ctx->field_type = SQLParser::FieldContext::BASIC;
        return res;
    }

    virtual antlrcpp::Any visitPrimary_key_field(SQLParser::Primary_key_fieldContext *ctx) override {
        auto res = visitChildren(ctx);
        PrimaryField field;
        field.name = ctx->Identifier() == nullptr ? "" : ctx->Identifier()->getText();
        for (const auto &id : ctx->identifiers()->Identifier()) {
            field.columns.push_back(id->getText());
        }
        ctx->primary_field_val = field;
        ctx->field_type = SQLParser::FieldContext::PRIMARY;
        return res;
    }

    virtual antlrcpp::Any visitForeign_key_field(SQLParser::Foreign_key_fieldContext *ctx) override {
        auto res = visitChildren(ctx);
        ForeignField field;
        field.name = ctx->Identifier(1) == nullptr ? "" : ctx->Identifier(0)->getText();
        for (const auto &id : ctx->identifiers(0)->Identifier()) {
            field.columns.push_back(id->getText());
        }
        field.foreign_table_name = ctx->Identifier(1) == nullptr ? ctx->Identifier(0)->getText() : ctx->Identifier(1)->getText();
        for (const auto &id : ctx->identifiers(1)->Identifier()) {
            field.foreign_columns.push_back(id->getText());
        }
        ctx->foreign_field_val = field;
        ctx->field_type = SQLParser::FieldContext::FOREIGN;
        return res;
    }

    virtual antlrcpp::Any visitDrop_table(SQLParser::Drop_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        ms.drop_table(table_name);
        return res;
    }

    virtual antlrcpp::Any visitDescribe_table(SQLParser::Describe_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        ms.describe_table(table_name);
        return res;
    }

    virtual antlrcpp::Any visitAlter_table_drop_pk(SQLParser::Alter_table_drop_pkContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier(0)->getText();
        std::string restriction_name = ctx->Identifier().size() == 1 ? "" : ctx->Identifier(1)->getText();
        ms.drop_primary_key(table_name, restriction_name);
        return res;
    }

    virtual antlrcpp::Any visitAlter_table_drop_foreign_key(SQLParser::Alter_table_drop_foreign_keyContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier(0)->getText();
        std::string restriction_name = ctx->Identifier(1)->getText();
        ms.drop_foreign_key(table_name, restriction_name);
        return res;
    }

    virtual antlrcpp::Any visitAlter_table_add_pk(SQLParser::Alter_table_add_pkContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier(0)->getText();
        std::string constraint_name = ctx->Identifier(1)->getText();
        std::vector<std::string> identifiers;
        for (const auto &it : ctx->identifiers()->Identifier()) {
            identifiers.push_back(it->getText());
        }
        ms.add_primary_key(table_name, {constraint_name, identifiers});
        return res;
    }

    virtual antlrcpp::Any visitAlter_table_add_foreign_key(SQLParser::Alter_table_add_foreign_keyContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier(0)->getText();
        std::string constraint_name = ctx->Identifier(1)->getText();
        std::string foreign_table_name = ctx->Identifier(2)->getText();
        std::vector<std::string> identifiers;
        for (const auto &it : ctx->identifiers(0)->Identifier()) {
            identifiers.push_back(it->getText());
        }
        std::vector<std::string> foreign_identifiers;
        for (const auto &it : ctx->identifiers(1)->Identifier()) {
            foreign_identifiers.push_back(it->getText());
        }
        ms.add_foreign_key(table_name, {constraint_name, identifiers, foreign_table_name, foreign_identifiers});
        return res;
    }

    virtual antlrcpp::Any visitAlter_table_add_unique(SQLParser::Alter_table_add_uniqueContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        std::vector<std::string> identifiers;
        for(auto column : ctx->identifiers()->Identifier()){
            identifiers.push_back(column->getText());
        }
        ms.add_unique(table_name, identifiers);
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

    virtual antlrcpp::Any visitDelete_from_table(SQLParser::Delete_from_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        auto where_clauses = ctx->where_and_clause()->where_clauses;
        qs.delete_record(table_name, where_clauses);
        return res;
    }

    virtual antlrcpp::Any visitUpdate_table(SQLParser::Update_tableContext *ctx) override {
        auto res = visitChildren(ctx);
        std::string table_name = ctx->Identifier()->getText();
        auto column_names = ctx->set_clause()->column_names;
        auto values = ctx->set_clause()->values;
        auto where_clauses = ctx->where_and_clause()->where_clauses;
        qs.update_record(table_name, column_names, values, where_clauses);
        return res;
    }

    virtual antlrcpp::Any visitSet_clause(SQLParser::Set_clauseContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->column_names.clear();
        ctx->values.clear();
        for(auto ident : ctx->Identifier())
            ctx->column_names.push_back(ident->getText());
        for(auto value_ctx : ctx->value())
            ctx->values.push_back(value_ctx->value);
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
        if (ctx->column()){
            ctx->select_stmt.group_by.is_empty = false;
            ctx->select_stmt.group_by.column = ctx->column()->column;
        }
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
        if (ctx->Count()){
            ctx->selector.type = Selector::Type::CT;
        }
        else if (ctx->aggregator()){
            ctx->selector.type = Selector::Type::AGR_COL;
            ctx->selector.agr_type = ctx->aggregator()->agr_type;
            ctx->selector.col = ctx->column()->column;
        }
        else{
            ctx->selector.type = Selector::Type::COL;
            ctx->selector.col = ctx->column()->column;
        }
        return res;
    }

    virtual antlrcpp::Any visitAggregator(SQLParser::AggregatorContext *ctx) override {
        auto res = visitChildren(ctx);
        if (ctx->Count()){
            ctx->agr_type = Selector::Aggregator_Type::COUNT;
        }
        else if (ctx->Average()){
            ctx->agr_type = Selector::Aggregator_Type::AVERAGE;
        }
        else if (ctx->Max()){
            ctx->agr_type = Selector::Aggregator_Type::MAX;
        }
        else if (ctx->Min()){
            ctx->agr_type = Selector::Aggregator_Type::MIN;
        }
        else if (ctx->Sum()){
            ctx->agr_type = Selector::Aggregator_Type::SUM;
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