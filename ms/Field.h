#pragma once

#include <string>
#include <vector>

struct Field {
    std::string name;                          // 字段名
    enum Type {
        STR,
        INT,
        FLOAT,
    } type;                                    // 字段类型
    std::size_t str_len;                       // 字符串类型的最大长度
    bool nullable;                             // 字段是否可空
    bool has_def;                              // 字段是否有默认值
    int def_int;                               // INT 型默认值
    float def_float;                           // FLOAT 型默认值
    std::string def_str;                       // STR 型默认值
};

struct PrimaryField {
    std::string name;                          // 约束名称
    std::vector<std::string> columns;          // 约束各列名称
};

struct ForeignField {
    std::string name;                          // 约束名称
    std::vector<std::string> columns;          // 从表各列名称
    std::string foreign_table_name;            // 主表名称
    std::vector<std::string> foreign_columns;  // 主表各列名称
};