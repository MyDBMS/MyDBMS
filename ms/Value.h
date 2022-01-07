#pragma once

#include <cstring>
#include <cassert>
#include <string>

class Value {
    void *data;

    std::size_t str_len;

public:
    explicit Value();

    explicit Value(int val);

    explicit Value(const std::string &val);

    explicit Value(float val);

    enum Type {
        NUL,
        STR,
        INT,
        FLOAT,
    } type;

    int asInt() const;

    std::string asString() const;

    float asFloat() const;

    bool isNull() const;

    static Value make_value();

    static Value make_value(int value);

    static Value make_value(const std::string &value);

    static Value make_value(float value);

    Value(const Value &rhs);

    std::partial_ordering operator<=>(const Value &rhs) const;

    bool operator==(const Value &rhs) const;

    Value& operator=(const Value&);

    ~Value();
};