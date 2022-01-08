#pragma once

#include <string>
#include <vector>

struct Field {
    std::string name;
    enum Type {
        STR,
        INT,
        FLOAT,
    } type;
    std::size_t str_len;
    bool nullable;
    int def_int;
    float def_float;
    std::string def_str;
};

struct PrimaryField {
    std::string name;
    std::vector<std::string> columns;
};

struct ForeignField {
    std::string name;
    std::vector<std::string> columns;
    std::string foreign_table_name;
    std::vector<std::string> foreign_columns;
};