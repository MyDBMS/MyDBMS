#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include "../ms/Value.h"

class Frontend {
    friend class Application;

    friend class TestSystem;

    virtual void write_string(const std::string &s) const = 0;

    virtual char read_char() = 0;

    std::string read_stmt();

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

    void write_line(const std::string &msg) const;
};

class StdioFrontend : public Frontend {
    void write_string(const std::string &s) const override;

    char read_char() override;
};

class StringStreamFrontend : public Frontend {
    std::stringstream &istream;
    std::stringstream &ostream;

    void write_string(const std::string &s) const override;

    char read_char() override;

public:
    explicit StringStreamFrontend(std::stringstream &istream, std::stringstream &ostream);
};