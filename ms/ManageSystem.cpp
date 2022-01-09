#include "ManageSystem.h"
#include "../qs/QuerySystem.h"
#include "TableMapping.h"
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

bool ManageSystem::ensure_db_valid() const {
    if (!current_db.valid) {
        frontend->error("DB is not opened yet.");
    }
    return current_db.valid;
}

std::size_t ManageSystem::find_table_by_name(const std::string &table_name, bool no_assert) {
    auto &map = table_mapping_map[current_db.id];
    for (std::size_t i = 0; i < map.count; ++i) {
        if (table_name == map.mapping[i].name) {
            return i;
        }
    }

    if (no_assert) {
        return map.count;
    }
    assert(false);
}

std::size_t ManageSystem::find_table_by_id(std::size_t table_id, bool no_assert) {
    auto &map = table_mapping_map[current_db.id];
    for (std::size_t i = 0; i < map.count; ++i) {
        if (table_id == map.mapping[i].id) {
            return i;
        }
    }

    if (no_assert) {
        return map.count;
    }
    assert(false);
}

std::size_t ManageSystem::find_column_by_name(std::size_t table_loc, const std::string &column_name, bool no_assert) {
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    for (std::size_t i = 0; i < info.field_count; ++i) {
        if (info.fields[i].column_name == column_name) {
            return i;
        }
    }

    if (no_assert) {
        return info.field_count;
    }
    assert(false);
}

std::string ManageSystem::find_column_by_id(std::size_t table_loc, std::size_t column_id) {
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    assert(column_id >= 0 && column_id < info.field_count);
    return info.fields[column_id].column_name;
}

void ManageSystem::add_index(std::size_t table_loc, std::size_t column_id, bool need_add_data) {
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    auto index_file_path = current_db.dir;
    index_file_path.append("idx_" + std::to_string(info.id) + "_" + info.fields[column_id].column_name + ".txt");
    is.create_file(index_file_path.c_str());
    info.fields[column_id].indexed = true;

    if (!need_add_data) return;

    std::size_t length_limit = get_record_length_limit(info.name);
    auto buffer = new char[length_limit + 5];
    auto record_file = get_record_file(info.name);
    auto index_file = get_index_file(info.name, info.fields[column_id].column_name);
    for (RID rid = record_file->find_first(); rid.page_id != 0; rid = record_file->find_next(rid)) {
        std::size_t length = record_file->get_record(rid, buffer);
        auto v = from_bytes_to_record(info.name, buffer, length)[column_id];
        if (!v.isNull()) index_file->insert_record(v.asInt(), rid);
    }
    delete[] buffer;
}

bool ManageSystem::add_primary_key(std::size_t table_loc, const PrimaryField &f, bool need_add_data_to_index) {
    auto &table_info = table_mapping_map[current_db.id].mapping[table_loc];
    if (table_info.primary_field_count == MAX_PRIMARY_FIELD_COUNT) {
        frontend->error("Too many primary constraints! Aborting.");
        return false;
    }
    auto &table_info_field = table_info.primary_fields[table_info.primary_field_count++];
    if (f.name.length() > MAX_PRIMARY_RESTRICTION_LEN) {
        frontend->warning(
                "Primary restriction name shall be no longer than " + std::to_string(MAX_PRIMARY_RESTRICTION_LEN) +
                " chars.");
    }
    strcpy(table_info_field.restriction_name, f.name.substr(0, MAX_PRIMARY_RESTRICTION_LEN).c_str());
    table_info_field.column_bitmap = 0;
    for (const auto &column_name: f.columns) {
        auto column_id = find_column_by_name(table_loc, column_name, true);
        if (column_id == table_info.field_count) {
            frontend->error("In primary restriction, column name of " + column_name + " cannot be found.");
            --table_info.primary_field_count;
            return false;
        }
        table_info.fields[column_id].nullable = false;
        table_info_field.column_bitmap |= (1u << column_id);
    }
    for (const auto &column_name: f.columns) {
        auto column_id = find_column_by_name(table_loc, column_name, true);
        if (column_id == table_info.field_count) {
            frontend->warning("Internal warning regarding adding primary key.");
            continue;
        }
        if (!table_info.fields[column_id].indexed) {
            add_index(table_loc, column_id, need_add_data_to_index);
        }
    }
    return true;
}

bool ManageSystem::add_foreign_key(std::size_t table_loc, const ForeignField &f, bool need_add_data_to_index) {
    auto &table_mapping = table_mapping_map[current_db.id];
    auto &table_info = table_mapping.mapping[table_loc];
    if (table_info.foreign_field_count == MAX_FOREIGN_FIELD_COUNT) {
        frontend->error("Too many foreign constraints! Aborting.");
        return false;
    }
    auto &table_info_field = table_info.foreign_fields[table_info.foreign_field_count++];
    if (f.name.length() > MAX_FOREIGN_RESTRICTION_LEN) {
        frontend->warning(
                "Foreign restriction name shall be no longer than " + std::to_string(MAX_FOREIGN_RESTRICTION_LEN) +
                " chars.");
    }
    strcpy(table_info_field.restriction_name, f.name.substr(0, MAX_FOREIGN_RESTRICTION_LEN).c_str());
    table_info_field.column_bitmap = 0;
    for (const auto &column_name: f.columns) {
        auto column_id = find_column_by_name(table_loc, column_name, true);
        if (column_id == table_info.field_count) {
            frontend->error("In foreign restriction, column name of " + column_name + " cannot be found.");
            --table_info.foreign_field_count;
            return false;
        }
        table_info_field.column_bitmap |= (1u << column_id);
    }
    auto foreign_table_loc = find_table_by_name(f.foreign_table_name, true);
    if (foreign_table_loc == table_mapping.count) {
        frontend->error("In foreign restriction, table name of " + f.foreign_table_name + " cannot be found.");
        --table_info.foreign_field_count;
        return false;
    }
    auto &foreign_table = table_mapping.mapping[foreign_table_loc];
    table_info_field.foreign_table_id = foreign_table.id;
    table_info_field.foreign_column_bitmap = 0;
    for (int i = 0; i < f.columns.size(); ++i) {
        const auto &column_name = f.columns[i];
        const auto &foreign_column_name = f.foreign_columns[i];
        auto column_id = find_column_by_name(table_loc, column_name, true);
        auto foreign_column_id = find_column_by_name(foreign_table_loc, foreign_column_name, true);
        if (foreign_column_id == table_info.field_count) {
            frontend->error("In foreign restriction, column name of " + foreign_column_name + " cannot be found.");
            --table_info.foreign_field_count;
            return false;
        }
        if (!foreign_table.fields[foreign_column_id].indexed) {
            frontend->error(
                    "Cannot find index of column " + foreign_column_name + " in table " + f.foreign_table_name + ".");
            --table_info.foreign_field_count;
            return false;
        }
        table_info_field.foreign_column_bitmap |= (1u << foreign_column_id);
        table_info_field.foreign_column_ids[column_id] = foreign_column_id;
    }
    for (const auto &column_name: f.columns) {
        auto column_id = find_column_by_name(table_loc, column_name, true);
        if (column_id == table_info.field_count) {
            frontend->warning("Internal warning regarding adding foreign key.");
            continue;
        }
        if (!table_info.fields[column_id].indexed) {
            add_index(table_loc, column_id, need_add_data_to_index);
        }
    }
    return true;
}

bool ManageSystem::add_unique(std::size_t table_loc, const std::vector<std::string> &columns) {
    auto &table_info = table_mapping_map[current_db.id].mapping[table_loc];
    if (table_info.unique_constraint_count == MAX_UNIQUE_CONSTRAINT_COUNT) {
        frontend->error("Too many unique constraints! Aborting.");
        return false;
    }
    auto &table_info_field = table_info.unique_constraints[table_info.unique_constraint_count++];
    table_info_field.column_bitmap = 0;
    for (const auto &column_name: columns) {
        auto column_id = find_column_by_name(table_loc, column_name, true);
        if (column_id == table_info.field_count) {
            frontend->error("In unique constraint, column name of " + column_name + " cannot be found.");
            --table_info.unique_constraint_count;
            return false;
        }
        table_info_field.column_bitmap |= (1u << column_id);
    }
    for (const auto &column_name: columns) {
        auto column_id = find_column_by_name(table_loc, column_name, true);
        if (column_id == table_info.field_count) {
            frontend->warning("Internal warning regarding adding unique constraint.");
            continue;
        }
        if (!table_info.fields[column_id].indexed) {
            add_index(table_loc, column_id, true);
        }
    }
    return true;
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

void ManageSystem::create_db(const std::string &db_name) {
    if (db_name.empty() || db_name.length() > MAX_DB_NAME_LEN) {
        frontend->error("DB name shall be neither empty nor longer than " + std::to_string(MAX_DB_NAME_LEN) + ".");
        return;
    }

    if (db_mapping.count + 1 >= MAX_DB_COUNT) {
        frontend->error("No more DB! You promised!");
        return;
    }

    // Add record to db mapping
    std::size_t current_id_max = 0;
    for (std::size_t i = 0; i < db_mapping.count; ++i) {
        if (db_mapping.mapping[i].id > current_id_max) {
            current_id_max = db_mapping.mapping[i].id;
        }
        if (strcmp(db_mapping.mapping[i].name, db_name.c_str()) == 0) {
            frontend->error("DB of name " + db_name + " already exists.");
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

    frontend->ok(1);
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
        frontend->error("DB of name " + db_name + " does not exist.");
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

    frontend->ok(1);
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
    if (!ensure_db_valid()) {
        return;
    }

    Frontend::Column c;
    c.name = std::string("Tables_in_") + db_mapping.mapping[current_db.loc].name;
    for (std::size_t i = 0; i < table_mapping_map[current_db.id].count; ++i) {
        c.values.emplace_back(table_mapping_map[current_db.id].mapping[i].name);
    }
    frontend->print_table({c});
}

void ManageSystem::show_indexes() const {
    if (!ensure_db_valid()) {
        return;
    }
    frontend->error("Bare SHOW INDEXES stmt is not supported!");
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
        frontend->error("DB of name " + db_name + " does not exist.");
        return;
    }

    frontend->info("Database changed.");
}

void ManageSystem::create_table(const std::string &table_name, const std::vector<Field> &field_list,
                                const std::vector<PrimaryField> &primary_field_list,
                                const std::vector<ForeignField> &foreign_field_list) {
    if (!ensure_db_valid()) {
        return;
    }

    if (table_name.empty() || table_name.length() > MAX_TABLE_NAME_LEN) {
        frontend->error("Tb name shall be neither empty nor longer than " + std::to_string(MAX_TABLE_NAME_LEN) + ".");
        return;
    }

    if (db_mapping.count + 1 >= MAX_TABLE_COUNT) {
        frontend->error("No more tables! You promised!");
        return;
    }

    if (field_list.size() > MAX_COLUMN_COUNT) {
        frontend->error("Too many columns!");
        return;
    }

    if (primary_field_list.size() > MAX_PRIMARY_FIELD_COUNT) {
        frontend->error("Too many primary fields!");
        return;
    }

    if (foreign_field_list.size() > MAX_FOREIGN_FIELD_COUNT) {
        frontend->error("Too many foreign fields!");
        return;
    }

    std::set<std::string> field_set;
    for (const auto &f: field_list) {
        if (field_set.find(f.name) != field_set.end()) {
            frontend->error("Duplicate field name: " + f.name);
            return;
        }
        field_set.insert(f.name);
    }

    // Add record to table mapping
    TableMapping &table_mapping = table_mapping_map[current_db.id];
    std::size_t current_id_max = 0;
    for (std::size_t i = 0; i < table_mapping.count; ++i) {
        if (table_mapping.mapping[i].id > current_id_max) {
            current_id_max = table_mapping.mapping[i].id;
        }
        if (strcmp(table_mapping.mapping[i].name, table_name.c_str()) == 0) {
            frontend->error("Table of name " + table_name + " already exists.");
            return;
        }
    }
    std::size_t table_id = current_id_max + 1;
    table_mapping.mapping[table_mapping.count].id = table_id;
    strcpy(table_mapping.mapping[table_mapping.count].name, table_name.c_str());
    auto &table_info = table_mapping.mapping[table_mapping.count];

    // Calculate fixed_size and var_cnt; maintain table mapping info
    std::size_t fixed_size = /* null bitmap */ (field_list.size() + 7) / 8;
    std::size_t var_cnt = 0;
    table_info.field_count = 0;
    for (const auto &f: field_list) {
        if (f.name.length() > MAX_COLUMN_NAME_LEN) {
            frontend->warning("Column name shall be no longer than " + std::to_string(MAX_COLUMN_NAME_LEN) + " chars.");
        }
        auto &table_info_field = table_info.fields[table_info.field_count];
        switch (f.type) {
            case Field::STR:
                fixed_size += 2;
                var_cnt += 1;
                table_info_field.type = Field::STR;
                table_info_field.str_len = f.str_len;
                strcpy(table_info_field.def_str, f.def_str.substr(0, MAX_DEFAULT_STR_LEN).c_str());
                if (f.def_str.length() > MAX_DEFAULT_STR_LEN) {
                    frontend->warning("Default string of field " + f.name + " is over long!");
                    break;
                }
                break;
            case Field::INT:
                fixed_size += 4;
                var_cnt += 0;
                table_info_field.type = Field::INT;
                table_info_field.str_len = 0;
                table_info_field.def_int = f.def_int;
                break;
            case Field::FLOAT:
                fixed_size += 4;
                var_cnt += 0;
                table_info_field.type = Field::FLOAT;
                table_info_field.str_len = 0;
                table_info_field.def_float = f.def_float;
                break;
            default:
                break;
        }

        strcpy(table_info_field.column_name, f.name.substr(0, MAX_COLUMN_NAME_LEN).c_str());
        table_info_field.nullable = f.nullable;
        table_info_field.indexed = false;
        table_info_field.has_def = f.has_def;

        ++table_info.field_count;
    }
    table_info.fixed_size = fixed_size;
    table_info.var_cnt = var_cnt;

    table_info.primary_field_count = 0;
    for (const auto &f: primary_field_list) {
        if (!add_primary_key(table_mapping.count, f, false)) {
            return;
        }
    }

    table_info.foreign_field_count = 0;
    for (const auto &f: foreign_field_list) {
        if (!add_foreign_key(table_mapping.count, f, false)) {
            return;
        }
    }

    ++table_mapping.count;
    update_table_mapping_file(current_db.id);

    // Create record file
    std::filesystem::path file_path = current_db.dir;
    file_path.append(std::to_string(table_id) + ".txt");
    rs.create_file(file_path.c_str(), fixed_size, var_cnt);

    frontend->ok(0);
}

void ManageSystem::drop_table(const std::string &table_name) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &mapping = table_mapping_map[current_db.id].mapping;
    auto &info = mapping[table_loc];

    // Foreign key restriction
    for (std::size_t reference_loc = 0; reference_loc < table_mapping_map[current_db.id].count; ++reference_loc) {
        if (reference_loc == table_loc) continue;
        auto &related_table_info = table_mapping_map[current_db.id].mapping[reference_loc];
        for (std::size_t i = 0; i < related_table_info.foreign_field_count; ++i) {
            auto &f = related_table_info.foreign_fields[i];
            if (f.foreign_table_id == info.id) {
                frontend->error("Table " + table_name + " is referenced by " + related_table_info.name +
                                " through foreign key, and cannot be dropped.");
                return;
            }
        }
    }

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
    while (table_loc < table_mapping_map[current_db.id].count) {
        mapping[table_loc] = mapping[table_loc + 1];
        ++table_loc;
    }
    update_table_mapping_file(current_db.id);

    frontend->ok(0);
}

void ManageSystem::describe_table(const std::string &table_name) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    Frontend::Table out_table{{"Field",   {}},
                              {"Type",    {}},
                              {"Null",    {}},
                              {"Default", {}}};
    for (std::size_t i = 0; i < info.field_count; ++i) {
        auto &f = info.fields[i];
        out_table[0].values.emplace_back(f.column_name);
        std::string type;
        std::string def;
        switch (f.type) {
            case Field::INT:
                type = "INT";
                def = f.has_def ? std::to_string(f.def_int) : "NULL";
                break;
            case Field::FLOAT:
                type = "FLOAT";
                def = f.has_def ? std::to_string(f.def_float) : "NULL";
                break;
            case Field::STR:
                type = "VARCHAR(" + std::to_string(f.str_len) + ")";
                def = f.has_def ? f.def_str : "NULL";
                break;
            default:
                assert(false);
                break;
        }
        out_table[1].values.emplace_back(type);
        out_table[2].values.emplace_back(f.nullable ? "YES" : "NO");
        out_table[3].values.emplace_back(def);
    }
    frontend->print_table(out_table);
    for (std::size_t i = 0; i < info.primary_field_count; ++i) {
        auto &f = info.primary_fields[i];
        std::string primary_msg = "PRIMARY KEY ";
        primary_msg += f.restriction_name;
        bool l_par_printed = false;
        for (int c = 0; c < info.field_count; ++c) {
            if ((f.column_bitmap >> c) & 1) {
                primary_msg += (l_par_printed ? ", " : "(");
                primary_msg += info.fields[c].column_name;
                l_par_printed = true;
            }
        }
        primary_msg += ");";
        frontend->write_line(primary_msg);
    }
    for (std::size_t i = 0; i < info.foreign_field_count; ++i) {
        auto &f = info.foreign_fields[i];
        std::string foreign_msg = "FOREIGN KEY ";
        foreign_msg += f.restriction_name;
        bool l_par_printed = false;
        for (int c = 0; c < info.field_count; ++c) {
            if ((f.column_bitmap >> c) & 1) {
                foreign_msg += (l_par_printed ? ", " : "(");
                foreign_msg += info.fields[c].column_name;
                l_par_printed = true;
            }
        }
        foreign_msg += ") REFERENCES ";
        auto foreign_table = table_mapping_map[current_db.id].mapping[find_table_by_id(f.foreign_table_id)];
        foreign_msg += foreign_table.name;
        l_par_printed = false;
        for (int c = 0; c < info.field_count; ++c) {
            if ((f.column_bitmap >> c) & 1) {
                foreign_msg += (l_par_printed ? ", " : "(");
                foreign_msg += foreign_table.fields[f.foreign_column_ids[c]].column_name;
                l_par_printed = true;
            }
        }
        foreign_msg += ");";
        frontend->write_line(foreign_msg);
    }
    for (std::size_t i = 0; i < info.unique_constraint_count; ++i) {
        auto &f = info.unique_constraints[i];
        std::string unique_msg = "UNIQUE ";
        bool l_par_printed = false;
        for (int c = 0; c < info.field_count; ++c) {
            if ((f.column_bitmap >> c) & 1) {
                unique_msg += (l_par_printed ? ", " : "(");
                unique_msg += info.fields[c].column_name;
                l_par_printed = true;
            }
        }
        unique_msg += ");";
        frontend->write_line(unique_msg);
    }
    for (std::size_t i = 0; i < info.field_count; ++i) {
        auto &f = info.fields[i];
        if (f.indexed) {
            frontend->write_line(std::string("INDEX (") + f.column_name + ");");
        }
    }
}

void ManageSystem::create_index(const std::string &table_name, const std::vector<std::string> &column_list) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];

    if (column_list.size() != 1) {
        frontend->error("Only single-column indexing is supported.");
        return;
    }

    std::size_t column_id = find_column_by_name(table_loc, column_list[0]);
    auto &field = info.fields[column_id];
    if (field.indexed) {
        frontend->error("Column " + column_list[0] + " is already indexed.");
        return;
    }
    if (field.type != Field::INT) {
        frontend->error("Column " + column_list[0] + " is not of type INT and cannot be indexed.");
        return;
    }

    add_index(table_loc, column_id, true);
    update_table_mapping_file(current_db.id);

    frontend->ok(0);
}

void ManageSystem::drop_index(const std::string &table_name, const std::vector<std::string> &column_list) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];

    if (column_list.size() != 1) {
        frontend->error("Only single-column indexing is supported.");
        return;
    }

    std::size_t column_id = find_column_by_name(table_loc, column_list[0]);
    auto &field = info.fields[column_id];
    if (!field.indexed) {
        frontend->error("Column " + column_list[0] + " is not indexed.");
        return;
    }

    auto index_file_path = current_db.dir;
    index_file_path.append("idx_" + std::to_string(info.id) + "_" + column_list[0] + ".txt");
    IndexSystem::remove_file(index_file_path.c_str());
    field.indexed = false;
    update_table_mapping_file(current_db.id);

    frontend->ok(0);
}

void ManageSystem::add_primary_key(const std::string &table_name, const PrimaryField &primary_restriction) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];

    SelectStmt search_stmt;
    search_stmt.table_names.push_back(table_name);
    for (const auto &r: qs->search(search_stmt).record) {
        SelectStmt stmt;
        stmt.table_names.push_back(table_name);
        for (const auto &column_name: primary_restriction.columns) {
            auto column_id = find_column_by_name(table_loc, column_name, true);
            if (column_id == info.field_count) {
                frontend->error("In primary constraint, column name of " + column_name + " cannot be found.");
                return;
            }
            if (r.values[column_id].isNull()) {
                frontend->error("Column " + column_name + " has null data. Primary key cannot be set.");
                return;
            }
            WhereClause clause;
            Column column;
            Expr expr;
            column.table_name = table_name;
            column.column_name = column_name;
            clause.type = WhereClause::OP_EXPR;
            clause.column = column;
            clause.op_type = WhereClause::EQ;
            expr.type = Expr::VALUE;
            expr.value = r.values[column_id];
            expr.column = column;
            clause.expr = expr;
            stmt.where_clauses.push_back(clause);
        }
        if (qs->search(stmt).record.size() > 1) {
            frontend->error("Duplicate values exist. Primary key cannot be set.");
            return;
        }
    }

    if (add_primary_key(table_loc, primary_restriction, true)) {
        update_table_mapping_file(current_db.id);
        frontend->ok(0);
    }
}

void ManageSystem::drop_primary_key(const std::string &table_name, const std::string &restriction_name) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];

    if (info.primary_field_count == 0) {
        frontend->error("No primary constraint exists.");
        return;
    }

    if (!restriction_name.empty() && info.primary_fields[0].restriction_name != restriction_name) {
        frontend->error("Primary key restriction of name " + restriction_name + " does not exist.");
        return;
    }

    for (std::size_t i = 0; i < table_mapping_map[current_db.id].count; ++i) {
        auto &foreign_table = table_mapping_map[current_db.id].mapping[i];
        for (int c = 0; c < foreign_table.foreign_field_count; ++c) {
            auto &r = foreign_table.foreign_fields[c];
            if (r.foreign_table_id == info.id) {
                frontend->error(
                        std::string("Primary key constraint is referenced by a foreign constraint of table ") +
                        foreign_table.name + ", and thus cannot be dropped.");
                return;
            }
        }
    }

    --info.primary_field_count;
    update_table_mapping_file(current_db.id);
    frontend->ok(0);
}

void ManageSystem::add_foreign_key(const std::string &table_name, const ForeignField &foreign_restriction) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];

    auto foreign_table_loc = find_table_by_name(foreign_restriction.foreign_table_name, true);
    if (foreign_table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("In foreign restriction, table name of " + foreign_restriction.foreign_table_name +
                        " cannot be found.");
        return;
    }
    auto &foreign_info = table_mapping_map[current_db.id].mapping[foreign_table_loc];

    SelectStmt search_stmt;
    search_stmt.table_names.push_back(table_name);
    for (const auto &r: qs->search(search_stmt).record) {
        SelectStmt stmt;
        bool has_null = false;
        stmt.table_names.push_back(foreign_restriction.foreign_table_name);
        for (int i = 0; i < foreign_restriction.columns.size(); ++i) {
            const auto &column_name = foreign_restriction.columns[i];
            auto column_id = find_column_by_name(table_loc, column_name, true);
            if (column_id == info.field_count) {
                frontend->error("In foreign constraint, column name of " + column_name + " cannot be found.");
                return;
            }
            const auto &foreign_column_name = foreign_restriction.foreign_columns[i];
            auto foreign_column_id = find_column_by_name(foreign_table_loc, foreign_column_name, true);
            if (foreign_column_id == foreign_info.field_count) {
                frontend->error("In foreign constraint, column name of " + foreign_column_name + " cannot be found.");
                return;
            }
            if (r.values[column_id].isNull()) {
                has_null = true;
                break;
            }
            WhereClause clause;
            Column column;
            Expr expr;
            column.table_name = foreign_info.name;
            column.column_name = foreign_column_name;
            clause.type = WhereClause::OP_EXPR;
            clause.column = column;
            clause.op_type = WhereClause::EQ;
            expr.type = Expr::VALUE;
            expr.value = r.values[column_id];
            expr.column = column;
            clause.expr = expr;
            stmt.where_clauses.push_back(clause);
        }
        if (!has_null && qs->search(stmt).record.empty()) {
            frontend->error("Some value cannot be found in the referenced table. Foreign constraint cannot be added.");
            return;
        }
    }

    if (add_foreign_key(table_loc, foreign_restriction, true)) {
        update_table_mapping_file(current_db.id);
        frontend->ok(0);
    }
}

void ManageSystem::drop_foreign_key(const std::string &table_name, const std::string &restriction_name) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    std::size_t restriction_loc = 0;
    while (restriction_loc < info.foreign_field_count) {
        if (info.foreign_fields[restriction_loc].restriction_name == restriction_name) {
            break;
        }
        ++restriction_loc;
    }
    if (restriction_loc == info.foreign_field_count) {
        frontend->error("Foreign key restriction of name " + restriction_name + " does not exist.");
        return;
    }
    --info.foreign_field_count;
    while (restriction_loc < info.foreign_field_count) {
        info.foreign_fields[restriction_loc] = info.foreign_fields[restriction_loc + 1];
        ++restriction_loc;
    }
    update_table_mapping_file(current_db.id);
    frontend->ok(0);
}

void ManageSystem::add_unique(const std::string &table_name, const std::vector<std::string> &column_list) {
    if (!ensure_db_valid()) {
        return;
    }
    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        frontend->error("Table does not exist.");
        return;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];

    SelectStmt search_stmt;
    search_stmt.table_names.push_back(table_name);
    for (const auto &r: qs->search(search_stmt).record) {
        SelectStmt stmt;
        stmt.table_names.push_back(table_name);
        bool has_null = false;
        for (const auto &column_name: column_list) {
            auto column_id = find_column_by_name(table_loc, column_name, true);
            if (column_id == info.field_count) {
                frontend->error("In unique constraint, column name of " + column_name + " cannot be found.");
                return;
            }
            if (r.values[column_id].isNull()) {
                has_null = true;
                continue;
            }
            WhereClause clause;
            Column column;
            Expr expr;
            column.table_name = table_name;
            column.column_name = column_name;
            clause.type = WhereClause::OP_EXPR;
            clause.column = column;
            clause.op_type = WhereClause::EQ;
            expr.type = Expr::VALUE;
            expr.value = r.values[column_id];
            expr.column = column;
            clause.expr = expr;
            stmt.where_clauses.push_back(clause);
        }
        if (!has_null && qs->search(stmt).record.size() > 1) {
            frontend->error("Duplicate values exist.");
            return;
        }
    }

    if (add_unique(table_loc, column_list)) {
        update_table_mapping_file(current_db.id);
        frontend->ok(0);
    }
}

Error::InsertError ManageSystem::validate_insert_data(const std::string &table_name, const std::vector<Value> &values) {
    using namespace Error;

    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        return TABLE_DOES_NOT_EXIST;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];
    if (info.field_count != values.size()) {
        return VALUE_COUNT_MISMATCH;
    }

    // Basic validation
    for (std::size_t i = 0; i < info.field_count; ++i) {
        if (!(values[i].type == Value::NUL
              || info.fields[i].type == Field::STR && values[i].type == Value::STR
              || info.fields[i].type == Field::FLOAT && values[i].type == Value::FLOAT
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

    // Unique restriction
    for (std::size_t i = 0; i < info.unique_constraint_count; ++i) {
        auto &f = info.unique_constraints[i];
        bool has_null = false;
        SelectStmt stmt;
        stmt.table_names.push_back(table_name);
        for (std::size_t c = 0; c < info.field_count; ++c) {
            if ((f.column_bitmap >> c) & 1) {
                if (values[c].isNull()) {
                    has_null = true;
                    break;
                }
                WhereClause clause;
                Column column;
                Expr expr;
                column.table_name = table_name;
                column.column_name = info.fields[c].column_name;
                clause.type = WhereClause::OP_EXPR;
                clause.column = column;
                clause.op_type = WhereClause::EQ;
                expr.type = Expr::VALUE;
                expr.value = values[c];
                expr.column = column;
                clause.expr = expr;
                stmt.where_clauses.push_back(clause);
            }
        }
        if (!has_null && !qs->search(stmt).record.empty()) {
            return UNIQUE_RESTRICTION_FAIL;
        }
    }

    // Primary key restriction
    for (std::size_t i = 0; i < info.primary_field_count; ++i) {
        auto &f = info.primary_fields[i];
        SelectStmt stmt;
        stmt.table_names.push_back(table_name);
        for (std::size_t c = 0; c < info.field_count; ++c) {
            if ((f.column_bitmap >> c) & 1) {
                WhereClause clause;
                Column column;
                Expr expr;
                column.table_name = table_name;
                column.column_name = info.fields[c].column_name;
                clause.type = WhereClause::OP_EXPR;
                clause.column = column;
                clause.op_type = WhereClause::EQ;
                expr.type = Expr::VALUE;
                expr.value = values[c];
                expr.column = column;
                clause.expr = expr;
                stmt.where_clauses.push_back(clause);
            }
        }
        if (!qs->search(stmt).record.empty()) {
            return PRIMARY_RESTRICTION_FAIL;
        }
    }

    // Foreign key restriction
    for (std::size_t i = 0; i < info.foreign_field_count; ++i) {
        auto &f = info.foreign_fields[i];
        SelectStmt stmt;
        bool has_null = false;
        std::size_t foreign_table_loc = find_table_by_id(f.foreign_table_id, true);
        if (foreign_table_loc == table_mapping_map[current_db.id].count) {
            frontend->warning("Internal warning: checking foreign constraint with non-existing table.");
            continue;
        }
        auto &foreign_info = table_mapping_map[current_db.id].mapping[foreign_table_loc];
        stmt.table_names.emplace_back(foreign_info.name);

        for (std::size_t c = 0; c < info.field_count; ++c) {
            if ((f.column_bitmap >> c) & 1) {
                if (values[c].isNull()) {
                    has_null = true;
                    break;
                }
                WhereClause clause;
                Column column;
                Expr expr;
                column.table_name = foreign_info.name;
                column.column_name = foreign_info.fields[f.foreign_column_ids[c]].column_name;
                clause.type = WhereClause::OP_EXPR;
                clause.column = column;
                clause.op_type = WhereClause::EQ;
                expr.type = Expr::VALUE;
                expr.value = values[c];
                expr.column = column;
                clause.expr = expr;
                stmt.where_clauses.push_back(clause);
            }
        }

        if (!has_null && qs->search(stmt).record.empty()) {
            return INSERT_FOREIGN_RESTRICTION_FAIL;
        }
    }
    return NONE;
}

Error::DeleteError ManageSystem::validate_delete_data(const std::string &table_name, const std::vector<Value> &values) {
    using namespace Error;

    std::size_t table_loc = find_table_by_name(table_name, true);
    if (table_loc == table_mapping_map[current_db.id].count) {
        return DELETE_TABLE_DOES_NOT_EXIST;
    }
    auto &info = table_mapping_map[current_db.id].mapping[table_loc];

    // Foreign key restriction
    for (std::size_t reference_loc = 0; reference_loc < table_mapping_map[current_db.id].count; ++reference_loc) {
        if (reference_loc == table_loc) continue;
        auto &related_table_info = table_mapping_map[current_db.id].mapping[reference_loc];
        for (std::size_t i = 0; i < related_table_info.foreign_field_count; ++i) {
            auto &f = related_table_info.foreign_fields[i];
            if (f.foreign_table_id != info.id) continue;
            SelectStmt stmt;
            bool has_null = false;
            stmt.table_names.emplace_back(related_table_info.name);
            for (std::size_t c = 0; c < related_table_info.field_count; ++c) {
                if ((f.column_bitmap >> c) & 1) {
                    if (values[c].isNull()) {
                        has_null = true;
                        break;
                    }
                    WhereClause clause;
                    Column column;
                    Expr expr;
                    column.table_name = related_table_info.name;
                    column.column_name = related_table_info.fields[c].column_name;
                    clause.type = WhereClause::OP_EXPR;
                    clause.column = column;
                    clause.op_type = WhereClause::EQ;
                    expr.type = Expr::VALUE;
                    expr.value = values[f.foreign_column_ids[c]];
                    expr.column = column;
                    clause.expr = expr;
                    stmt.where_clauses.push_back(clause);
                }
            }
            if (!has_null && !qs->search(stmt).record.empty()) {
                return DELETE_FOREIGN_RESTRICTION_FAIL;
            }
        }
    }
    return Error::DELETE_NONE;
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
                    case Field::FLOAT:
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
            case Value::FLOAT:
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
                case Field::FLOAT:
                    actual_type = Value::FLOAT;
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
            case Value::FLOAT:
                *(float *) (buffer + fixed_pos) = v.asFloat();
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
            case Field::FLOAT: {
                float value = *(float *) (buffer + fixed_pos);
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

bool ManageSystem::is_column_exist(const std::string &table_name, const std::string &column_name) {
    std::size_t table_id = find_table_by_name(table_name);
    return find_column_by_name(table_id, column_name, true) !=
           table_mapping_map[current_db.id].mapping[table_id].field_count;
}

Field ManageSystem::get_column_info(const std::string &table_name, const std::string &column_name) {
    std::size_t table_id = find_table_by_name(table_name);
    std::size_t column_id = find_column_by_name(table_id, column_name);
    auto info = table_mapping_map[current_db.id].mapping[table_id].fields[column_id];
    return {
            /* name */      info.column_name,
            /* type */      info.type,
            /* str_len */   info.str_len,
            /* nullable */  info.nullable,
            /* has_def */   info.has_def,
            /* def_int */   info.def_int,
            /* def_float */ info.def_float,
            /* def_str */   info.def_str,
    };
}

bool ManageSystem::is_table_exist(const std::string &table_name) {
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
    auto f = record_files.find(file_path);
    return f != record_files.end() ? f->second : record_files[file_path] = rs.open_file(file_path.c_str());
}

IndexFile *ManageSystem::get_index_file(const std::string &table_name, const std::string &column_name) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    auto index_file_path = current_db.dir;
    std::size_t column_id = find_column_by_name(table_id, column_name);
    assert(info.fields[column_id].indexed);
    index_file_path.append("idx_" + std::to_string(info.id) + "_" + info.fields[column_id].column_name + ".txt");
    auto f = index_files.find(index_file_path);
    return f != index_files.end() ? f->second : index_files[index_file_path] = is.open_file(index_file_path.c_str());
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
            case Field::FLOAT:
                length += 4;
                break;
            default:
                assert(false);
        }
    }
    return length;
}

std::size_t ManageSystem::get_column_num(const std::string &table_name) {
    std::size_t table_id = find_table_by_name(table_name);
    auto &info = table_mapping_map[current_db.id].mapping[table_id];
    return info.field_count;
}
