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
        // Test null value
        Value v1 = Value::make_value();
        Value v2 = Value::make_value();
        Value v3 = v1;
        assert(v1.isNull() && v2.isNull() && v3.isNull());
        assert(v1.asInt() == 0);
        assert(v2.asString() == "");
        assert(v3.asInt() == 0);
    }

    {
        // Test vector of value
        std::vector<Value> v;
        v.push_back(Value::make_value("x"));
        v.push_back(Value::make_value(1));
        v.push_back(Value::make_value("y"));
        v.push_back(Value::make_value());
        Value v1 = v[0];
        Value v2 = v[1];
        Value v3 = v[2];
        Value v4 = v[3];
        assert(v1.asString() == "x");
        assert(v2.asInt() == 1);
        assert(v3.asString() == "y");
        assert(v4.asInt() == 0);
    }
}

void test_ms() {
    ManageSystem ms = ManageSystem::load_system(SYSTEM_ROOT);
    ms.create_db(DB_NAME);
    ms.use_db(DB_NAME);

    std::vector<Field> fields;
    fields.push_back(Field{"a", Field::INT, 0, false});
    fields.push_back(Field{"b", Field::STR, 5, false});
    fields.push_back(Field{"c", Field::INT, 0, false});
    ms.create_table(TABLE_NAME, fields);

    // validate insert data
    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233"));
        values.push_back(Value::make_value(6));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::NONE);
    }

    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233"));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::VALUE_COUNT_MISMATCH);
    }

    {
        std::vector<Value> values;
        values.push_back(Value::make_value("666"));
        values.push_back(Value::make_value("233"));
        values.push_back(Value::make_value(6));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::TYPE_MISMATCH);
    }

    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233666"));
        values.push_back(Value::make_value(6));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::STR_TOO_LONG);
    }

    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233"));
        values.push_back(Value::make_value());
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::FIELD_CANNOT_BE_NULL);
    }

    // convert between record and bytes
    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233"));
        values.push_back(Value::make_value(6));
        std::size_t l;
        auto received = ms.from_record_to_bytes(TABLE_NAME, values, l);
        assert(l == 14);
        const char expected[14] = {0, 3, 0, 0, 0, 6, 0, 0, 0, 14, 0, '2', '3', '3'};
        assert(memcmp(received, expected, 14) == 0);
    }

    {
        char bytes[14] = {0, 3, 0, 0, 0, 6, 0, 0, 0, 14, 0, '2', '3', '3'};
        auto record = ms.from_bytes_to_record(TABLE_NAME, bytes, 14);
        assert(record.size() == 3);
        assert(record[0].asInt() == 3);
        assert(record[1].asString() == "233");
        assert(record[2].asInt() == 6);
    }

    // test getting record file
    {
        char record[14] = {0, 3, 0, 0, 0, 6, 0, 0, 0, 14, 0, '2', '3', '3'};
        auto record_file = ms.get_record_file(TABLE_NAME);
        assert(record_file->insert_record(14, record).page_id == 2);
    }

    // test other api
    {
        assert(ms.get_column_name(TABLE_NAME, 0) == "a");
        assert(ms.get_column_name(TABLE_NAME, 1) == "b");
        assert(ms.get_column_name(TABLE_NAME, 2) == "c");

        assert(ms.is_table_exist(TABLE_NAME) == true);
        assert(ms.is_table_exist("xyz") == false);

        assert(ms.get_record_length_limit(TABLE_NAME) == 16);
    }

    std::filesystem::remove_all(SYSTEM_ROOT);
}

int main() {
    test_value();
    test_ms();
    return 0;
}
