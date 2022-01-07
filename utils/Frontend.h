#pragma once

#include <iostream>
#include <sstream>
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

    void ok(int row_cnt) const;

    void info(const std::string &msg) const;

    void error(const std::string &msg) const;

    void warning(const std::string &msg) const;
};

class StdioFrontend : public Frontend {
    void print_string(const std::string &s) const override;
};

class StringStreamFrontend : public Frontend {
    std::stringstream &stream;

    void print_string(const std::string &s) const override;

public:
    explicit StringStreamFrontend(std::stringstream &stream);
};