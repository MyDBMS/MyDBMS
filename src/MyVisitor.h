#pragma once

#include "SQLBaseVisitor.h"
#include "../qs/QuerySystem.h"
#include "../ms/ManageSystem.h"

class MyVisitor : public SQLBaseVisitor {
public:

    QuerySystem qs;

    ManageSystem ms;

    explicit MyVisitor(const std::string &root_dir): ms(ManageSystem::load_system(root_dir)) {
    }
    
    virtual antlrcpp::Any visitProgram(SQLParser::ProgramContext *ctx) override {
        /* printf("hello program\n"); */
        printf("\n");
        auto res = visitChildren(ctx);
        printf("\n");
        return res;
    }

    virtual antlrcpp::Any visitStatement(SQLParser::StatementContext *ctx) override {
        /* printf("hello statement\n"); */
        auto res = visitChildren(ctx);
        if (ctx->db_statement()){
            auto db_ctx = ctx->db_statement();
            switch(db_ctx->db_stmt_type){
                case 1: 
                    printf("It's a create db stmt!\n");
                    break;
                case 2: 
                    printf("It's a drop db stmt!\n");
                    break;
                case 3: 
                    printf("It's a show db stmt!\n");
                    break;
                case 4: 
                    printf("It's a use db stmt!\n");
                    break;
                case 5: 
                    printf("It's a show db stmt!\n");
                    break;
                case 6: 
                    printf("It's a show db index stmt!\n");
                    break;
                default:
                    printf("Ashitemaru is lazy\n");
                    break;
            }
        }
        else printf("It's not a db stmt!\n");
        return res;
    }

    virtual antlrcpp::Any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
        auto res = visitChildren(ctx);
        auto ident = ctx->Identifier();
        std::string name = ident->getText();
        std::cout << "database " + name + " is created" << std::endl;
        ctx->db_stmt_type = 1;
        return res;
    }

    virtual antlrcpp::Any visitDrop_db(SQLParser::Drop_dbContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->db_stmt_type = 2;
        return res;
    }

    virtual antlrcpp::Any visitShow_dbs(SQLParser::Show_dbsContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->db_stmt_type = 3;
        return res;
    }

    virtual antlrcpp::Any visitUse_db(SQLParser::Use_dbContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->db_stmt_type = 4;
        return res;
    }

    virtual antlrcpp::Any visitShow_tables(SQLParser::Show_tablesContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->db_stmt_type = 5;
        return res;
    }

    virtual antlrcpp::Any visitShow_indexes(SQLParser::Show_indexesContext *ctx) override {
        auto res = visitChildren(ctx);
        ctx->db_stmt_type = 6;
        return res;
    }
};