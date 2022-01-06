#include "ManageSystem.h"
#include <cstring>

ManageSystem::ManageSystem(const std::string &root_dir) :
        system_root(root_dir),
        global_root(std::filesystem::path(root_dir).append("global")),
        base_root(std::filesystem::path(root_dir).append("base")),
        db_mapping_path(std::filesystem::path(root_dir).append("global").append("global.txt")) {}

void ManageSystem::load_db_mapping_file() {
    assert(std::filesystem::is_regular_file(db_mapping_path));
    auto db_mapping_file = fopen(db_mapping_path.c_str(), "rb");
    fread((void *) &db_mapping, sizeof db_mapping, 1, db_mapping_file);
    fclose(db_mapping_file);
    assert(db_mapping.count <= MAX_DB_COUNT);
}

void ManageSystem::update_db_mapping_file() {
    auto db_mapping_file = fopen(db_mapping_path.c_str(), "wb");
    fwrite((void *) &db_mapping, sizeof db_mapping, 1, db_mapping_file);
    fclose(db_mapping_file);
}

void ManageSystem::load_table_mapping_file(std::size_t db_id) {
    auto table_mapping_file_name = global_root;
    table_mapping_file_name.append(std::to_string(db_id) + ".txt");
    assert(std::filesystem::is_regular_file(db_mapping_path));

    TableMapping table_mapping;
    auto table_mapping_file = fopen(table_mapping_file_name.c_str(), "rb");
    fread((void *) &table_mapping, sizeof table_mapping, 1, table_mapping_file);
    fclose(table_mapping_file);
    assert(table_mapping.db_id == db_id);
    assert(table_mapping.count <= MAX_TABLE_COUNT);
    table_mapping_map[db_id] = table_mapping;
}

void ManageSystem::update_table_mapping_file(std::size_t db_id) {
    auto table_mapping_file_name = global_root;
    table_mapping_file_name.append(std::to_string(db_id) + ".txt");

    auto table_mapping = table_mapping_map[db_id];
    auto table_mapping_file = fopen(table_mapping_file_name.c_str(), "wb");
    fwrite((void *) &table_mapping, sizeof table_mapping, 1, table_mapping_file);
    fclose(table_mapping_file);
}

ManageSystem ManageSystem::load_system(const std::string &root_dir) {
    ManageSystem ms(root_dir);

    if (!std::filesystem::exists(ms.system_root)) {
        std::filesystem::create_directories(ms.system_root);
        std::filesystem::create_directory(ms.global_root);
        std::filesystem::create_directory(ms.base_root);
        ms.update_db_mapping_file();
    } else {
        assert(std::filesystem::is_directory(ms.system_root));
        assert(std::filesystem::is_directory(ms.global_root));
        assert(std::filesystem::is_directory(ms.base_root));
        ms.load_db_mapping_file();

        for (std::size_t i = 0; i < ms.db_mapping.count; ++i) {
            ms.load_table_mapping_file(ms.db_mapping.mapping[i].id);
        }
    }

    return ms;
}

void ManageSystem::issue() {

}

void ManageSystem::create_db(const std::string &db_name) {
    if (db_name.empty() || db_name.length() > MAX_DB_NAME_LEN) {
        issue();
        return;
    }

    if (db_mapping.count + 1 >= MAX_DB_COUNT) {
        issue();
        return;
    }

    // Add record to db mapping
    std::size_t current_id_max = 0;
    for (std::size_t i = 0; i < db_mapping.count; ++i) {
        if (db_mapping.mapping[i].id > current_id_max) {
            current_id_max = db_mapping.mapping[i].id;
        }
        if (strcmp(db_mapping.mapping[i].name, db_name.c_str()) == 0) {
            issue();
            return;
        }
    }
    std::size_t db_id = current_id_max + 1;
    db_mapping.mapping[db_mapping.count].id = db_id;
    strcpy(db_mapping.mapping[db_mapping.count].name, db_name.c_str());
    ++db_mapping.count;
    update_db_mapping_file();

    // Create table mapping
    table_mapping_map[db_id] = TableMapping{db_id};
    update_table_mapping_file(db_id);

    // Create directory
    auto db_directory = base_root;
    db_directory.append(std::to_string(db_id));
    if (std::filesystem::exists(db_directory)) {
        std::filesystem::remove_all(db_directory);
    }
    std::filesystem::create_directory(db_directory);
}

void ManageSystem::use_db(const std::string &db_name) {
    // Check if there is already an open db
    if (current_db.valid) {
        current_db.valid = false;
    }

    for (std::size_t i = 0; i < db_mapping.count; ++i) {
        if (strcmp(db_mapping.mapping[i].name, db_name.c_str()) == 0) {
            current_db.valid = true;
            current_db.loc = i;
            current_db.id = db_mapping.mapping[i].id;
            current_db.dir = base_root;
            current_db.dir.append(std::to_string(current_db.id));
        }
    }

    if (!current_db.valid) {
        issue();
        return;
    }
}

void ManageSystem::create_table(const std::string &table_name, const std::vector<Field> &field_list) {
    if (table_name.empty() || table_name.length() > MAX_TABLE_NAME_LEN) {
        issue();
        return;
    }

    if (db_mapping.count + 1 >= MAX_TABLE_COUNT) {
        issue();
        return;
    }

    if (!current_db.valid) {
        issue();
        return;
    }

    // Add record to table mapping
    TableMapping &table_mapping = table_mapping_map[current_db.id];
    std::size_t current_id_max = 0;
    for (std::size_t i = 0; i < table_mapping.count; ++i) {
        if (table_mapping.mapping[i].id > current_id_max) {
            current_id_max = table_mapping.mapping[i].id;
        }
        if (strcmp(table_mapping.mapping[i].name, table_name.c_str()) == 0) {
            issue();
            return;
        }
    }
    std::size_t table_id = current_id_max + 1;
    table_mapping.mapping[table_mapping.count].id = table_id;
    strcpy(table_mapping.mapping[table_mapping.count].name, table_name.c_str());
    auto &table_info = table_mapping.mapping[table_mapping.count];
    ++table_mapping.count;

    // Calculate fixed_size and var_cnt; maintain table mapping info
    std::size_t fixed_size = /* null bitmap */ (field_list.size() + 7) / 8;
    std::size_t var_cnt = 0;
    table_info.field_count = 0;
    for (const auto &f: field_list) {
        if (f.name.length() > MAX_COLUMN_NAME_LEN) {
            issue();
        }
        auto &table_info_field = table_info.fields[table_info.field_count];
        switch (f.type) {
            case Field::STR:
                fixed_size += 2;
                var_cnt += 1;
                strcpy(table_info_field.column_name, f.name.substr(0, MAX_COLUMN_NAME_LEN).c_str());
                table_info_field.type = Field::STR;
                table_info_field.str_len = f.str_len;
                break;
            case Field::INT:
                fixed_size += 4;
                var_cnt += 0;
                strcpy(table_info_field.column_name, f.name.substr(0, MAX_COLUMN_NAME_LEN).c_str());
                table_info_field.type = Field::INT;
                table_info_field.str_len = 0;
                break;
            default:
                break;
        }
        ++table_info.field_count;
    }
    update_table_mapping_file(current_db.id);

    // Create record file
    std::filesystem::path file_path = current_db.dir;
    file_path.append(std::to_string(table_id) + ".txt");
    rs.create_file(file_path.c_str(), fixed_size, var_cnt);
}
