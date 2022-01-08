#pragma once

#include <cstring>
#include <cassert>
#include "Field.h"

class Value {
    void *data;

    std::size_t str_len;

public:
    explicit Value();

    explicit Value(int val);

    explicit Value(const std::string &val);

    enum Type {
        NUL,
        STR,
        INT,
    } type;

    int asInt() const;

    std::string asString() const;

    bool isNull() const;

    static Value make_value();

    static Value make_value(int value);

    static Value make_value(const std::string &value);

    Value(const Value &rhs);

    Value& operator=(const Value&);

    ~Value();
};