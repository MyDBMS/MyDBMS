#pragma once

#include "../rs/RecordFile.h"
#include "../is/IndexFile.h"
#include "../ms/ManageSystem.h"

#include "Column.h"
#include "Selector.h"
#include "WhereClause.h"
#include "SelectStmt.h"
#include "RecordSet.h"

class QuerySystem {
private:
    bool flag;  //  true 表示正常工作中， false 表示出现了错误

    std::string from_Value_to_string(Value value);

    RecordSet join_Recordset(RecordSet a, RecordSet b);

    RecordSet merge_Recordset(RecordSet a, RecordSet b);

    bool compare_Value(Value a, Value b, WhereClause::OP_Type op_type);

    bool is_some_column_exist(RecordSet record_set, Column column);

    Value get_column_value(std::vector<Column> columns, RecordData record_data, Column column);

    Frontend::Table from_RecordSet_to_Table(RecordSet record_set);

    RecordSet search_whole_table(std::string table_name);

    RecordSet search_by_index(std::string table_name, std::string column_name, int lower_bound, int upper_bound);

    RecordSet search_where_clause(RecordSet input_result, WhereClause where_clause);

    RecordSet search_where_clauses(std::vector<std::string> table_names, std::vector<WhereClause> where_clauses);

    RecordSet search_selector(RecordSet input_result, Selector selector);

    RecordSet search_selectors(RecordSet input_result, std::vector<Selector> selectors);
public:
    ManageSystem &ms;

    explicit QuerySystem(ManageSystem &ms);

    void insert_record(std::string table_name, std::vector<Value> values);

    void delete_record(std::string table_name, std::vector<WhereClause> where_clauses);

    RecordSet search(SelectStmt select_stmt);

    void search_entry(SelectStmt select_stmt);
};