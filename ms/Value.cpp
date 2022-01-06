#include "Value.h"
#include <cstring>

Value::Value() : data{nullptr}, str_len{0}, type{NUL} {}

Value::Value(int val)
        : data{(void *) new int(val)}, str_len{0}, type{INT} {}

Value::Value(const std::string &val)
        : data{(void *) new char[val.length() + 1]}, str_len{val.length()}, type{STR} {
    strcpy((char *) data, val.c_str());
}

int Value::asInt() const {
    if (type == NUL) return 0;
    assert(type == INT);
    return *(int *) data;
}

std::string Value::asString() const {
    if (type == NUL) return "";
    assert(type == STR);
    return (char *) data;
}

bool Value::isNull() const {
    return type == NUL;
}

Value Value::make_value() {
    return Value();
}

Value Value::make_value(int value) {
    return Value(value);
}

Value Value::make_value(const std::string &value) {
    return Value(value);
}

Value::Value(const Value &rhs) : str_len(rhs.str_len), type(rhs.type) {
    switch (rhs.type) {
        case NUL:
            data = nullptr;
            break;
        case STR:
            data = (void *) new char[rhs.str_len + 1];
            strcpy((char *) data, (char *) rhs.data);
            break;
        case INT:
            data = (void *) new int(*(int *) rhs.data);
            break;
        default:
            assert(false);
    }
}

Value::~Value() {
    switch (type) {
        case NUL:
            break;
        case STR:
            delete (char *) data;
            break;
        case INT:
            delete (int *) data;
            break;
        default:
            assert(false);
    }
}
