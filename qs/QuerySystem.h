#pragma once

#include "../rs/RecordFile.h"
#include "../is/IndexFile.h"
#include "../ms/ManageSystem.h"

typedef std::vector<Value> RecordData;

struct RecordSet {
public:
    std::vector<std::string> column_name;
    std::vector<RecordData> record;

    RecordSet() = default;
};

class QuerySystem {
private:
    std::string from_Value_to_string(Value value);

    Frontend::Table from_RecordSet_to_Table(RecordSet record_set);

    RecordSet search_by_index(std::string table_name, std::string column_name, int key);
public:
    ManageSystem &ms;

    explicit QuerySystem(ManageSystem &ms);

    void insert_record(std::string table_name, std::vector<Value> values);

    void search(std::string table_name, std::string column_name, int key);
};