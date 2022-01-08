#pragma once

#include <string>

struct Column {
public:
    std::string table_name;
    std::string column_name;

    bool has_table;  //  若为 false ，代表其是聚合列，输出时不管 table_name

    Column();

    bool operator < (const Column &other);

    bool operator == (const Column &other);
};