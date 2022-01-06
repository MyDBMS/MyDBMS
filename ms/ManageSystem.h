#pragma once

#include <filesystem>
#include "../rs/RecordSystem.h"
#include "DatabaseMapping.h"
#include "TableMapping.h"
#include "Field.h"
#include "Value.h"
#include "../utils/error.h"

class ManageSystem {
    /**
     * 数据库系统根目录，包含 base 和 global 两个目录。
     */
    const std::filesystem::path system_root;

    /**
     * 数据库系统 global 目录。
     */
    const std::filesystem::path global_root;

    /**
     * 数据库系统 base 目录。
     */
    const std::filesystem::path base_root;

    /**
     * db_mapping 文件目录。
     */
    const std::filesystem::path db_mapping_path;

    /**
     * 数据库名称与 ID 的映射关系。
     */
    DatabaseMapping db_mapping;

    /**
     * 表名称与 ID 的映射关系。
     */
    std::map<std::size_t, TableMapping> table_mapping_map;

    /**
     * 当前打开的数据库的信息。
     */
    struct {
        bool valid;                 // 未打开或非法时该字段均为假
        std::size_t loc;            // 下标
        std::size_t id;             // 编号
        std::filesystem::path dir;  // 目录
    } current_db;

    RecordSystem rs;

    explicit ManageSystem(const std::string &root_dir);

    void load_db_mapping_file();

    void update_db_mapping_file();

    void load_table_mapping_file(std::size_t db_id);

    void update_table_mapping_file(std::size_t db_id);

    std::size_t find_table_by_name(const std::string &table_name);

    static void issue();

public:

    static ManageSystem load_system(const std::string &root_dir);

    void create_db(const std::string &db_name);

    void use_db(const std::string &db_name);

    void create_table(const std::string &table_name, const std::vector<Field> &field_list);

    Error::InsertError validate_insert_data(const std::string &table_name, const std::vector<Value> &values);

    char *from_record_to_bytes(const std::string &table_name, const std::vector<Value> &values, std::size_t &length);

    std::vector<Value> from_bytes_to_record(const std::string &table_name, char *buffer, std::size_t length);

    std::string get_column_name(const std::string &table_name, std::size_t column_id);

    bool is_table_exist(const std::string &table_name);

    RecordFile *get_record_file(const std::string &table_name);

    std::size_t get_record_length_limit(const std::string &table_name);
};
