#include "ManageSystem.h"
#include <cstring>

ManageSystem::ManageSystem(const std::string &root_dir, const Frontend *frontend) :
        system_root(root_dir),
        global_root(std::filesystem::path(root_dir).append("global")),
        base_root(std::filesystem::path(root_dir).append("base")),
        db_mapping_path(std::filesystem::path(root_dir).append("global").append("global.txt")),
        frontend(frontend) {}

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

std::size_t ManageSystem::find_table_by_name(const std::string &table_name) {
    assert(current_db.valid);

    auto &map = table_mapping_map[current_db.id];
    for (std::size_t i = 0; i < map.count; ++i) {
        if (table_name == map.mapping[i].name) {
            return i;
        }
    }

    assert(false);
}

std::size_t ManageSystem::find_column_by_name(std::size_t table_loc, const std::string &column_name) {
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    for (std::size_t i = 0; i < info.field_count; ++i) {
        if (info.fields[i].column_name == column_name) {
            return i;
        }
    }

    assert(false);
}

std::string ManageSystem::find_column_by_id(std::size_t table_loc, std::size_t column_id) {
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    assert(column_id >= 0 && column_id < info.field_count);
    return info.fields[column_id].column_name;
}

ManageSystem ManageSystem::load_system(const std::string &root_dir, const Frontend *frontend) {
    ManageSystem ms(root_dir, frontend);

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

void ManageSystem::drop_db(const std::string &db_name) {
    if (current_db.valid) {
        current_db.valid = false;
    }

    // Find db by name
    std::size_t idx = 0;
    while (idx < db_mapping.count) {
        if (db_mapping.mapping[idx].name == db_name) {
            break;
        }
        ++idx;
    }
    if (idx == db_mapping.count) {
        issue();
        return;
    }

    // Remove table mapping
    table_mapping_map.erase(db_mapping.mapping[idx].id);
    auto global_file_path = global_root;
    global_file_path.append(std::to_string(db_mapping.mapping[idx].id) + ".txt");
    std::filesystem::remove(global_file_path);

    // Remove directory
    auto directory_path = base_root;
    directory_path.append(std::to_string(db_mapping.mapping[idx].id));
    std::filesystem::remove_all(directory_path);

    // Remove record from db mapping
    --db_mapping.count;
    while (idx < db_mapping.count) {
        db_mapping.mapping[idx] = db_mapping.mapping[idx + 1];
        ++idx;
    }
    update_db_mapping_file();
}

void ManageSystem::show_dbs() {
    Frontend::Column c;
    c.name = "Database";
    for (std::size_t i = 0; i < db_mapping.count; ++i) {
        c.values.emplace_back(db_mapping.mapping[i].name);
    }
    frontend->print_table({c});
}

void ManageSystem::show_tables() {
    if (!current_db.valid) {
        issue();
        return;
    }

    Frontend::Column c;
    c.name = std::string("Tables_in_") + db_mapping.mapping[current_db.loc].name;
    for (std::size_t i = 0; i < table_mapping_map[current_db.id].count; ++i) {
        c.values.emplace_back(table_mapping_map[current_db.id].mapping[i].name);
    }
    frontend->print_table({c});
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
                table_info_field.nullable = f.nullable;
                table_info_field.indexed = false;
                break;
            case Field::INT:
                fixed_size += 4;
                var_cnt += 0;
                strcpy(table_info_field.column_name, f.name.substr(0, MAX_COLUMN_NAME_LEN).c_str());
                table_info_field.type = Field::INT;
                table_info_field.str_len = 0;
                table_info_field.nullable = f.nullable;
                table_info_field.indexed = false;
                break;
            default:
                break;
        }
        ++table_info.field_count;
    }
    table_info.fixed_size = fixed_size;
    table_info.var_cnt = var_cnt;
    update_table_mapping_file(current_db.id);

    // Create record file
    std::filesystem::path file_path = current_db.dir;
    file_path.append(std::to_string(table_id) + ".txt");
    rs.create_file(file_path.c_str(), fixed_size, var_cnt);
}

void ManageSystem::drop_table(const std::string &table_name) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &mapping = table_mapping_map[current_db.id].mapping;
    auto &info = mapping[table_id];

    // Remove record file
    std::filesystem::path file_path = current_db.dir;
    file_path.append(std::to_string(info.id) + ".txt");
    RecordSystem::remove_file(file_path.c_str());

    // Remove index files
    for (std::size_t i = 0; i < info.field_count; ++i) {
        if (info.fields[i].indexed) {
            auto index_file_path = current_db.dir;
            index_file_path.append("idx_" + std::to_string(info.id) + "_" + info.fields[i].column_name + ".txt");
            IndexSystem::remove_file(index_file_path.c_str());
        }
    }

    // Remove table from table mapping info
    --table_mapping_map[current_db.id].count;
    while (table_id < table_mapping_map[current_db.id].count) {
        mapping[table_id] = mapping[table_id + 1];
        ++table_id;
    }
    update_table_mapping_file(current_db.id);
}

void ManageSystem::create_index(const std::string &table_name, const std::vector<std::string> &column_list) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];

    if (column_list.size() != 1) {
        issue();
        return;
    }

    std::size_t column_id = find_column_by_name(table_id, column_list[0]);
    auto &field = info.fields[column_id];
    if (field.indexed) {
        issue();
        return;
    }

    auto index_file_path = current_db.dir;
    index_file_path.append("idx_" + std::to_string(info.id) + "_" + column_list[0] + ".txt");
    is.create_file(index_file_path.c_str());
    field.indexed = true;
    update_table_mapping_file(current_db.id);
}

void ManageSystem::drop_index(const std::string &table_name, const std::vector<std::string> &column_list) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];

    if (column_list.size() != 1) {
        issue();
        return;
    }

    std::size_t column_id = find_column_by_name(table_id, column_list[0]);
    auto &field = info.fields[column_id];
    if (!field.indexed) {
        issue();
        return;
    }

    auto index_file_path = current_db.dir;
    index_file_path.append("idx_" + std::to_string(info.id) + "_" + column_list[0] + ".txt");
    IndexSystem::remove_file(index_file_path.c_str());
    field.indexed = false;
    update_table_mapping_file(current_db.id);
}

Error::InsertError ManageSystem::validate_insert_data(const std::string &table_name, const std::vector<Value> &values) {
    using namespace Error;

    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    if (info.field_count != values.size()) {
        return VALUE_COUNT_MISMATCH;
    }

    for (std::size_t i = 0; i < info.field_count; ++i) {
        if (!(values[i].type == Value::NUL
              || info.fields[i].type == Field::STR && values[i].type == Value::STR
              || info.fields[i].type == Field::INT && values[i].type == Value::INT)) {
            return TYPE_MISMATCH;
        }

        if (!info.fields[i].nullable && values[i].type == Value::NUL) {
            return FIELD_CANNOT_BE_NULL;
        }

        if (info.fields[i].type == Field::STR && values[i].asString().length() > info.fields[i].str_len) {
            return STR_TOO_LONG;
        }
    }
    return NONE;
}

char *ManageSystem::from_record_to_bytes(const std::string &table_name, const std::vector<Value> &values, size_t &l) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    assert(info.field_count == values.size());

    l = (info.field_count + 7) / 8;
    for (std::size_t i = 0; i < values.size(); ++i) {
        auto v = values[i];
        switch (v.type) {
            case Value::NUL:
                switch (info.fields[i].type) {
                    case Field::STR:
                        l += 0;
                        break;
                    case Field::INT:
                        l += 4;
                        break;
                    default:
                        assert(false);
                }
                break;
            case Value::STR:
                l += 2 + (v.asString().length());
                break;
            case Value::INT:
                l += 4;
                break;
            default:
                assert(false);
        }
    }

    auto buffer = new char[l]{};
    auto bitmap = (uint8_t *) buffer;
    std::size_t fixed_pos = (info.field_count + 7) / 8;
    std::size_t var_info_pos = info.fixed_size - info.var_cnt * 2;
    std::size_t var_data_pos = info.fixed_size;
    for (std::size_t i = 0; i < values.size(); ++i) {
        auto v = values[i];

        // null bitmap
        bitmap[i / 8] += ((v.isNull() ? 1u : 0u) << (i & 7));

        // get actual type
        auto actual_type = v.type;
        if (actual_type == Value::NUL) {
            switch (info.fields[i].type) {
                case Field::STR:
                    actual_type = Value::STR;
                    break;
                case Field::INT:
                    actual_type = Value::INT;
                    break;
                default:
                    assert(false);
            }
        }

        // fill in data
        switch (actual_type) {
            case Value::STR:
                memcpy(buffer + var_data_pos, v.asString().c_str(), v.asString().length());
                var_data_pos += v.asString().length();
                *(uint16_t *) (buffer + var_info_pos) = var_data_pos;
                var_info_pos += 2;
                break;
            case Value::INT:
                *(int *) (buffer + fixed_pos) = v.asInt();
                fixed_pos += 4;
                break;
            default:
                assert(false);
        }
    }

    return buffer;
}

std::vector<Value> ManageSystem::from_bytes_to_record(const std::string &table_name, char *buffer, std::size_t length) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];

    auto result = std::vector<Value>();
    auto bitmap = (uint8_t *) buffer;
    std::size_t fixed_pos = (info.field_count + 7) / 8;
    std::size_t var_info_pos = info.fixed_size - info.var_cnt * 2;
    std::size_t var_data_pos = info.fixed_size;
    for (std::size_t i = 0; i < info.field_count; ++i) {
        if ((bitmap[i / 8] >> (i & 7)) & 1) {
            result.push_back(Value::make_value());
            continue;
        }

        auto f = info.fields[i];
        switch (f.type) {
            case Field::STR: {
                std::size_t var_data_end_pos = *(uint16_t *) (buffer + var_info_pos);
                char value[var_data_end_pos - var_data_pos + 1];
                if (var_data_end_pos - var_data_pos > 0) {
                    memcpy(value, buffer + var_data_pos, var_data_end_pos - var_data_pos);
                }
                value[var_data_end_pos - var_data_pos] = 0;
                var_data_pos = var_data_end_pos;
                var_info_pos += 2;
                result.push_back(Value::make_value(value));
                break;
            }
            case Field::INT: {
                int value = *(int *) (buffer + fixed_pos);
                fixed_pos += 4;
                result.push_back(Value::make_value(value));
                break;
            }
            default:
                assert(false);
        }
    }

    return result;
}

std::vector<std::size_t> ManageSystem::get_index_ids(const std::string &table_name) {
    std::vector<std::size_t> result;
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    for (std::size_t i = 0; i < info.field_count; ++i) {
        if (info.fields[i].indexed) {
            result.push_back(i);
        }
    }
    return result;
}

bool ManageSystem::is_index_exist(const std::string &table_name, const std::string &column_name) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    std::size_t column_id = find_column_by_name(table_id, column_name);
    return info.fields[column_id].indexed;
}

std::string ManageSystem::get_column_name(const std::string &table_name, std::size_t column_id) {
    std::size_t table_id = find_table_by_name(table_name);
    return find_column_by_id(table_id, column_id);
}

bool ManageSystem::is_table_exist(const std::string &table_name) {
    assert(current_db.valid);

    auto &map = table_mapping_map[current_db.id];
    for (std::size_t i = 0; i < map.count; ++i) {
        if (table_name == map.mapping[i].name) {
            return true;
        }
    }

    return false;
}

RecordFile *ManageSystem::get_record_file(const std::string &table_name) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    auto file_path = current_db.dir;
    file_path.append(std::to_string(info.id) + ".txt");
    return rs.open_file(file_path.c_str());
}

IndexFile *ManageSystem::get_index_file(const std::string &table_name, const std::string &column_name) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    auto index_file_path = current_db.dir;
    std::size_t column_id = find_column_by_name(table_id, column_name);
    assert(info.fields[column_id].indexed);
    index_file_path.append("idx_" + std::to_string(info.id) + "_" + info.fields[column_id].column_name + ".txt");
    return is.open_file(index_file_path.c_str());
}

std::size_t ManageSystem::get_record_length_limit(const std::string &table_name) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    std::size_t length = (info.field_count + 7) / 8;
    for (std::size_t i = 0; i < info.field_count; ++i) {
        auto f = info.fields[i];
        switch (f.type) {
            case Field::STR:
                length += 2 + f.str_len;
                break;
            case Field::INT:
                length += 4;
                break;
            default:
                assert(false);
        }
    }
    return length;
}
