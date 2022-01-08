#pragma once

#include <string>

#define MAX_DB_COUNT 20
#define MAX_DB_NAME_LEN 20

struct DatabaseMapping {
    std::size_t count{};
    struct {
        std::size_t id;
        char name[MAX_DB_NAME_LEN + 1];
    } mapping[MAX_DB_COUNT]{};
};