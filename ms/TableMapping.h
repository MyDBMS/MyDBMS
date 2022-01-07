#pragma once

#include <string>
#include "Field.h"

#define MAX_TABLE_COUNT 10
#define MAX_TABLE_NAME_LEN 10
#define MAX_COLUMN_COUNT 10
#define MAX_COLUMN_NAME_LEN 10

struct TableMapping {
    std::size_t db_id{};
    std::size_t count{};
    struct {
        std::size_t id{};
        char name[MAX_TABLE_NAME_LEN + 1]{};
        std::size_t fixed_size{};
        std::size_t var_cnt{};
        std::size_t field_count{};
        struct {
            char column_name[MAX_COLUMN_NAME_LEN + 1]{};
            Field::Type type{};
            std::size_t str_len{};
            bool nullable{};
            bool indexed{};
        } fields[MAX_COLUMN_COUNT];
    } mapping[MAX_TABLE_COUNT]{};
};