#pragma once

#include <iostream>
#include <strstream>
#include <vector>
#include "../ms/Value.h"

class Frontend {
    virtual void print_string(const std::string &s) const = 0;

    static std::string pad(const std::string &s, std::size_t max_width);

public:
    struct Column {
        std::string name;
        std::vector<std::string> values;
    };

    typedef std::vector<Column> Table;

    void print_table(const Table &table) const;
};

class StdioFrontend : public Frontend {
    void print_string(const std::string &s) const override;
};

class StrStreamFrontend : public Frontend {
    std::strstream &stream;

    void print_string(const std::string &s) const override;

public:
    explicit StrStreamFrontend(std::strstream &stream);
};