#include "../ms/ManageSystem.h"

#define SYSTEM_ROOT ("bin/root")
#define DB_NAME ("test_db")
#define TABLE_NAME ("test_table")

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
    test();
    return 0;
}
