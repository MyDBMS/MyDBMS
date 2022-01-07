#pragma once

#include "../rs/RecordFile.h"
#include "../is/IndexFile.h"
#include "../ms/ManageSystem.h"


class QuerySystem {
public:
    ManageSystem &ms;

    explicit QuerySystem(ManageSystem &ms);

    void insert_record(std::string table_name, std::vector<Value> values);

    void search(std::string table_name, std::string column_name, int key);
};