#include "../ms/ManageSystem.h"
#include "QuerySystem.h"

#define SYSTEM_ROOT ("bin/ms-test-root")
#define DB_NAME ("test_db")
#define TABLE_NAME ("test_table")
#define FOREIGN_TABLE_NAME ("foreign_tb")

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
        // Test float value
        Value v1 = Value::make_value(1.0f);
        Value v2 = Value::make_value(2.0f);
        Value v3 = v1;
        assert(v1.asFloat() == 1.0f);
        assert(v2.asFloat() == 2.0f);
        assert(v3.asFloat() == 1.0f);
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
    std::filesystem::remove_all(SYSTEM_ROOT);
    std::stringstream iss, oss;
    ManageSystem ms = ManageSystem::load_system(SYSTEM_ROOT, new StringStreamFrontend(iss, oss));
    QuerySystem qs{ms};
    ms.qs = &qs;
    ms.create_db(DB_NAME);
    ms.use_db(DB_NAME);

    ms.create_table(FOREIGN_TABLE_NAME, {{"e", Field::INT, 0, false},
                                         {"f", Field::INT, 0, false}}, {{"", {"f"}}}, {});
    qs.insert_record(FOREIGN_TABLE_NAME, {Value::make_value(2), Value::make_value(3)});

    std::vector<Field> fields;
    fields.push_back(Field{"a", Field::INT, 0, false});
    fields.push_back(Field{"b", Field::STR, 5, false});
    fields.push_back(Field{"c", Field::INT, 0, false});
    fields.push_back(Field{"d", Field::FLOAT, 0, false});

    std::vector<PrimaryField> primary_fields;
    primary_fields.push_back({"test_pk", {"c"}});

    std::vector<ForeignField> foreign_fields;
    foreign_fields.push_back({"foreign", {"a"}, FOREIGN_TABLE_NAME, {"f"}});

    ms.create_table(TABLE_NAME, fields, primary_fields, foreign_fields);

    // validate insert data
    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233"));
        values.push_back(Value::make_value(6));
        values.push_back(Value::make_value(8.0f));
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
        values.push_back(Value::make_value(8.0f));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::TYPE_MISMATCH);
    }

    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233666"));
        values.push_back(Value::make_value(6));
        values.push_back(Value::make_value(8.0f));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::STR_TOO_LONG);
    }

    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233"));
        values.push_back(Value::make_value());
        values.push_back(Value::make_value(8.0f));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::FIELD_CANNOT_BE_NULL);
    }

    {
        std::vector<Value> values;
        values.push_back(Value::make_value(2));
        values.push_back(Value::make_value("xyz"));
        values.push_back(Value::make_value(0));
        values.push_back(Value::make_value(1.0f));
        assert(ms.validate_insert_data(TABLE_NAME, values) == Error::INSERT_FOREIGN_RESTRICTION_FAIL);
    }

    // validate delete data
    {
        qs.insert_record(TABLE_NAME, {Value::make_value(3),
                                      Value::make_value("233"),
                                      Value::make_value(6),
                                      Value::make_value(8.0f)});
        assert(ms.validate_delete_data(FOREIGN_TABLE_NAME, {Value::make_value(2), Value::make_value(4)}, {1, 1}) ==
               Error::DELETE_NONE);
        assert(ms.validate_delete_data(FOREIGN_TABLE_NAME, {Value::make_value(2), Value::make_value(3)}, {1, 1}) ==
               Error::DELETE_FOREIGN_RESTRICTION_FAIL);
    }

    // test comparison
    {
        assert(Value::make_value() == Value::make_value());
        assert((Value::make_value() == Value::make_value(1)) == false);
        assert((Value::make_value() != Value::make_value(233)) == false);
        assert(Value::make_value(42) == Value::make_value(42));
        assert(Value::make_value(80) > Value::make_value(42));
        assert(Value::make_value(12) < Value::make_value(42));
        assert(Value::make_value("xyz") != Value::make_value("abc"));
        assert(!(Value::make_value() > Value::make_value(42)));
        assert(!(Value::make_value(42) == Value::make_value("abc")));
        assert(!(Value::make_value(42) < Value::make_value("abc")));
        assert((Value::make_value(42) != Value::make_value("abc")) == false);
    }

    // convert between record and bytes
    {
        std::vector<Value> values;
        values.push_back(Value::make_value(3));
        values.push_back(Value::make_value("233"));
        values.push_back(Value::make_value(6));
        values.push_back(Value::make_value(0.0f));
        std::size_t l;
        auto received = ms.from_record_to_bytes(TABLE_NAME, values, l);
        assert(l == 18);
        const char expected[18] = {0, 3, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 18, 0, '2', '3', '3'};
        assert(memcmp(received, expected, 18) == 0);
    }

    {
        char bytes[18] = {0, 3, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 18, 0, '2', '3', '3'};
        auto record = ms.from_bytes_to_record(TABLE_NAME, bytes, 18);
        assert(record.size() == 4);
        assert(record[0].asInt() == 3);
        assert(record[1].asString() == "233");
        assert(record[2].asInt() == 6);
        assert(record[3].asFloat() == 0);
    }

    // test indexing
    {
        auto indices = ms.get_index_ids(TABLE_NAME);
        assert(indices.size() == 2);
        assert(indices[0] == 0);
        assert(indices[1] == 2);

        assert(ms.is_index_exist(TABLE_NAME, "b") == false);
        assert(ms.is_index_exist(TABLE_NAME, "c") == true);
    }

    // test altering pk, unique and foreign keys
    {
        ms.drop_primary_key(TABLE_NAME, "test_pk");
        ms.add_primary_key(TABLE_NAME, {"test_pk2", {"a"}});
        ms.add_unique(TABLE_NAME, {"c"});
        ms.drop_foreign_key(TABLE_NAME, "foreign");
    }

    // test getting record file
    {
        char record[18] = {0, 3, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 18, 0, '2', '3', '3'};
        auto record_file = ms.get_record_file(TABLE_NAME);
        assert(record_file->insert_record(18, record).page_id == 2);
    }

    // test other api
    {
        assert(ms.get_column_name(TABLE_NAME, 0) == "a");
        assert(ms.get_column_name(TABLE_NAME, 1) == "b");
        assert(ms.get_column_name(TABLE_NAME, 2) == "c");

        assert(ms.is_table_exist(TABLE_NAME) == true);
        assert(ms.is_table_exist("xyz") == false);

        assert(ms.get_record_length_limit(TABLE_NAME) == 20);
    }

    {
        oss.str("");
        ms.show_dbs();
        ms.show_tables();
        ms.describe_table(TABLE_NAME);
        std::string expected = R"(+----------+
| Database |
+----------+
| test_db  |
+----------+
1 row in set
+-------------------+
| Tables_in_test_db |
+-------------------+
| foreign_tb        |
| test_table        |
+-------------------+
2 rows in set
+-------+------------+------+---------+
| Field | Type       | Null | Default |
+-------+------------+------+---------+
| a     | INT        | NO   | NULL    |
| b     | VARCHAR(5) | NO   | NULL    |
| c     | INT        | NO   | NULL    |
| d     | FLOAT      | NO   | NULL    |
+-------+------------+------+---------+
4 rows in set
PRIMARY KEY test_pk2(a);
UNIQUE (c);
INDEX (a);
INDEX (c);
)";
        assert(oss.str() == expected);
    }

    // Test api related to dropping
    {
        ms.drop_index(TABLE_NAME, {"c"});
        ms.drop_table(TABLE_NAME);
        ms.drop_db(DB_NAME);
    }

    std::filesystem::remove_all(SYSTEM_ROOT);
}

int main() {
    test_value();
    test_ms();
    return 0;
}
