#include "../ms/ManageSystem.h"

#define SYSTEM_ROOT ("bin/root")
#define DB_NAME ("test_db")
#define TABLE_NAME ("test_table")

void test_value() {
    {
        // Test int value
        Value v1 = Value::make_value(233);
        Value v2 = Value::make_value(666);
        Value v3 = v1;
        assert(v1.asInt() == 233);
        assert(v2.asInt() == 666);
        assert(v3.asInt() == 233);
    }

    {
        // Test str value
        Value v1 = Value::make_value("abc");
        Value v2 = Value::make_value("def");
        Value v3 = v1;
        assert(v1.asString() == "abc");
        assert(v2.asString() == "def");
        assert(v3.asString() == "abc");
    }

    {
        // Test vector of value
        std::vector<Value> v;
        v.push_back(Value::make_value("x"));
        v.push_back(Value::make_value(1));
        v.push_back(Value::make_value("y"));
        Value v1 = v[0];
        Value v2 = v[1];
        Value v3 = v[2];
        assert(v1.asString() == "x");
        assert(v2.asInt() == 1);
        assert(v3.asString() == "y");
    }
}

void test() {
    ManageSystem ms = ManageSystem::load_system(SYSTEM_ROOT);
    ms.create_db(DB_NAME);
    ms.use_db(DB_NAME);

    std::vector<Field> fields;
    fields.push_back(Field{"a", Field::INT, 0});
    fields.push_back(Field{"b", Field::STR, 5});
    fields.push_back(Field{"c", Field::INT, 0});
    ms.create_table(TABLE_NAME, fields);
    // std::filesystem::remove_all(SYSTEM_ROOT);
}

int main() {
    test_value();
    test();
    return 0;
}
