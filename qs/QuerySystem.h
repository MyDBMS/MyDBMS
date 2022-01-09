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
    void QS_error(const std::string &msg);

    std::string from_Value_to_string(Value value);

    RecordSet join_Recordset(RecordSet a, RecordSet b);

    RecordSet merge_Recordset(RecordSet a, RecordSet b);

    RecordSet add_RecordSet(RecordSet a, RecordSet b);

    bool compare_Value(Value a, Value b, WhereClause::OP_Type op_type);

    bool is_some_column_exist(RecordSet record_set, Column column);

    Value get_column_value(std::vector<Column> columns, RecordData record_data, Column column);

    void set_column_value(std::vector<Column> columns, RecordData &record_data, Column column, Value value);

    Frontend::Table from_RecordSet_to_Table(RecordSet record_set);

    RecordSet search_whole_table(std::string table_name);

    RecordSet search_by_index(std::string table_name, std::string column_name, int lower_bound, int upper_bound);

    RecordSet search_by_another_table(RecordSet input_result, std::string table_name, WhereClause where_clause);

    RecordSet search_where_clause(RecordSet input_result, WhereClause where_clause);

    RecordSet search_where_clauses(std::vector<std::string> table_names, std::vector<WhereClause> where_clauses);

    RecordSet search_selector(RecordSet input_result, Selector selector, GroupBy group_by);

    RecordSet search_selectors(RecordSet input_result, std::vector<Selector> selectors, GroupBy group_by);

    RecordSet search_limit_offset(RecordSet input_result, int limit, int offset);
public:
    ManageSystem &ms;

    bool flag;  //  true 表示正常工作中， false 表示出现了错误

    explicit QuerySystem(ManageSystem &ms);

    void insert_record(std::string table_name, std::vector<Value> values);

    RecordSet search(SelectStmt select_stmt);

    void search_entry(SelectStmt select_stmt);

    void delete_record(std::string table_name, std::vector<WhereClause> where_clauses);

    void update_record(std::string table_name, std::vector<std::string> column_names, std::vector<Value> update_values, std::vector<WhereClause> where_clauses);
};