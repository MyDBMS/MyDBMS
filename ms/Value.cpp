#include "Value.h"
#include <cstring>

Value::Value(int val)
        : data{(void *) new int(val)}, type{Field::INT} {}

Value::Value(const std::string &val)
        : data{(void *) new char[val.length() + 1]}, type{Field::STR} {
    strcpy((char *) data, val.c_str());
}

int Value::asInt() const {
    assert(type == Field::INT);
    return *(int *) data;
}

std::string Value::asString() const {
    assert(type == Field::STR);
    return (char *) data;
}

Value Value::make_value(int value) {
    return Value(value);
}

Value Value::make_value(const std::string &value) {
    return Value(value);
}
