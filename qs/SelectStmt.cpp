#include "SelectStmt.h"

#include <cstdio>
#include <iostream>

SelectStmt::SelectStmt(){
    table_names.clear();
    selectors.clear();
    where_clauses.clear();
    limit = TABLE_ROW_MAX;
    offset = 0;
}

void SelectStmt::print_stmt(){
    printf("table names: \n");
    for(auto table_name : table_names)
        std::cout << table_name << " ";
    printf("\n");
    printf("selectors: \n");
    for(auto selector : selectors)
        std::cout << selector.col.table_name << "." << selector.col.column_name << " ";
    printf("\n");
    printf("where clauses: \n");
    for(auto where_clause : where_clauses)
        where_clause.print_wc();
}