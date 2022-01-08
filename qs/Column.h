#pragma once

#include <string>

struct Column {
public:
    std::string table_name;
    std::string column_name;

    Column();

    bool operator < (const Column &other);

    bool operator == (const Column &other);
};