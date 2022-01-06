#pragma once

#include <string>

struct Field {
    std::string name;
    enum Type {
        STR,
        INT,
    } type;
    std::size_t str_len;
    bool nullable;
};