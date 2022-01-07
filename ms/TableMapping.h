#pragma once

#include <string>
#include "Field.h"

#define MAX_TABLE_COUNT 10
#define MAX_TABLE_NAME_LEN 10
#define MAX_COLUMN_COUNT 10
#define MAX_COLUMN_NAME_LEN 10
#define MAX_DEFAULT_STR_LEN 10
#define MAX_PRIMARY_RESTRICTION_LEN 10
#define MAX_PRIMARY_FIELD_COUNT 10
#define MAX_FOREIGN_RESTRICTION_LEN 10
#define MAX_FOREIGN_FIELD_COUNT 10

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
            int def_int{};
            float def_float{};
            char def_str[MAX_DEFAULT_STR_LEN + 1]{};
        } fields[MAX_COLUMN_COUNT];
        std::size_t primary_field_count{};
        struct {
            char restriction_name[MAX_PRIMARY_RESTRICTION_LEN + 1]{};
            u_int16_t column_bitmap{};
        } primary_fields[MAX_PRIMARY_FIELD_COUNT];
        std::size_t foreign_field_count{};
        struct {
            char restriction_name[MAX_FOREIGN_RESTRICTION_LEN + 1]{};
            u_int16_t column_bitmap{};
            std::size_t foreign_table_id{};
            u_int16_t foreign_column_bitmap{};
        } foreign_fields[MAX_FOREIGN_FIELD_COUNT];
        std::size_t related_table_count{};
        std::size_t related_table_ids[MAX_TABLE_COUNT]{};
    } mapping[MAX_TABLE_COUNT]{};
};