
// Generated from SQL.g4 by ANTLR 4.9.3

#pragma once


#include "antlr4-runtime.h"
#include "SQLVisitor.h"

#include <cstdio>


/**
 * This class provides an empty implementation of SQLVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SQLBaseVisitor : public SQLVisitor {
public:

  virtual antlrcpp::Any visitProgram(SQLParser::ProgramContext *ctx) override {
    /* printf("hello program\n"); */
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatement(SQLParser::StatementContext *ctx) override {
    /* printf("hello statement\n");
    if (ctx->db_statement()) printf("It's a db stmt!\n");
    else printf("It's not a db stmt!\n"); */
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDrop_db(SQLParser::Drop_dbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitShow_dbs(SQLParser::Show_dbsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUse_db(SQLParser::Use_dbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitShow_tables(SQLParser::Show_tablesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitShow_indexes(SQLParser::Show_indexesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLoad_data(SQLParser::Load_dataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDump_data(SQLParser::Dump_dataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCreate_table(SQLParser::Create_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDrop_table(SQLParser::Drop_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDescribe_table(SQLParser::Describe_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInsert_into_table(SQLParser::Insert_into_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDelete_from_table(SQLParser::Delete_from_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUpdate_table(SQLParser::Update_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSelect_table_(SQLParser::Select_table_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSelect_table(SQLParser::Select_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlter_add_index(SQLParser::Alter_add_indexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlter_drop_index(SQLParser::Alter_drop_indexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlter_table_drop_pk(SQLParser::Alter_table_drop_pkContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlter_table_drop_foreign_key(SQLParser::Alter_table_drop_foreign_keyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlter_table_add_pk(SQLParser::Alter_table_add_pkContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlter_table_add_foreign_key(SQLParser::Alter_table_add_foreign_keyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlter_table_add_unique(SQLParser::Alter_table_add_uniqueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitField_list(SQLParser::Field_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNormal_field(SQLParser::Normal_fieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrimary_key_field(SQLParser::Primary_key_fieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForeign_key_field(SQLParser::Foreign_key_fieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType_(SQLParser::Type_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue_lists(SQLParser::Value_listsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue_list(SQLParser::Value_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue(SQLParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhere_and_clause(SQLParser::Where_and_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhere_operator_select(SQLParser::Where_operator_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhere_null(SQLParser::Where_nullContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhere_in_list(SQLParser::Where_in_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhere_in_select(SQLParser::Where_in_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhere_like_string(SQLParser::Where_like_stringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitColumn(SQLParser::ColumnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpression(SQLParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSet_clause(SQLParser::Set_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSelectors(SQLParser::SelectorsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSelector(SQLParser::SelectorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOperator_(SQLParser::Operator_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAggregator(SQLParser::AggregatorContext *ctx) override {
    return visitChildren(ctx);
  }


};

