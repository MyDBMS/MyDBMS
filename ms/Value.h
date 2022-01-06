#pragma once

#include <cassert>
#include "Field.h"

class Value {
    void *data;

    explicit Value(int val);

    explicit Value(const std::string &val);

public:
    const Field::Type type;

    int asInt() const;

    std::string asString() const;

    static Value make_value(int value);

    static Value make_value(const std::string &value);

    ~Value() {
        switch (type) {
            case Field::STR:
                delete (char *) data;
                break;
            case Field::INT:
                delete (int *) data;
                break;
            default:
                assert(false);
        }
    }
};