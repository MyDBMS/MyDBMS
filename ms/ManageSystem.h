#pragma once

#include <filesystem>
#include "../rs/RecordSystem.h"
#include "../is/IndexSystem.h"
#include "DatabaseMapping.h"
#include "TableMapping.h"
#include "Field.h"
#include "Value.h"
#include "../utils/Error.h"
#include "../utils/Frontend.h"

class QuerySystem;

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

    /**
     * 保存已经打开的记录文件。
     */
    std::map<std::string, RecordFile *> record_files;

    /**
     * 保存已经打开的索引文件。
     */
    std::map<std::string, IndexFile *> index_files;

    RecordSystem rs;

    IndexSystem is;

    explicit ManageSystem(const std::string &root_dir, const Frontend *frontend);

    void load_db_mapping_file();

    void update_db_mapping_file();

    void load_table_mapping_file(std::size_t db_id);

    void update_table_mapping_file(std::size_t db_id);

    std::size_t find_table_by_name(const std::string &table_name, bool no_assert = false);

    std::size_t find_table_by_id(std::size_t table_id, bool no_assert = false);

    std::size_t find_column_by_name(std::size_t table_loc, const std::string &column_name, bool no_assert = false);

    std::string find_column_by_id(std::size_t table_loc, std::size_t column_id);

    void add_index(std::size_t table_loc, std::size_t column_id, bool need_add_data);

    bool add_primary_key(std::size_t table_loc, const PrimaryField &f, bool need_add_data_to_index);

    bool add_foreign_key(std::size_t table_loc, const ForeignField &f);

    bool add_unique(std::size_t table_loc, const std::vector<std::string> &columns);

public:

    const Frontend *frontend;

    QuerySystem *qs;

    static ManageSystem load_system(const std::string &root_dir, const Frontend *frontend);

    bool ensure_db_valid() const;

    void create_db(const std::string &db_name);

    void drop_db(const std::string &db_name);

    void show_dbs();

    void show_tables();

    void show_indexes() const;

    void use_db(const std::string &db_name);

    void create_table(const std::string &table_name, const std::vector<Field> &field_list,
                      const std::vector<PrimaryField> &primary_field_list,
                      const std::vector<ForeignField> &foreign_field_list);

    void drop_table(const std::string &table_name);

    void describe_table(const std::string &table_name);

    void create_index(const std::string &table_name, const std::vector<std::string> &column_list);

    void drop_index(const std::string &table_name, const std::vector<std::string> &column_list);

    void add_primary_key(const std::string &table_name, const PrimaryField &primary_restriction);

    void drop_primary_key(const std::string &table_name, const std::string &restriction_name);

    void add_foreign_key(const std::string &table_name, const ForeignField &foreign_restriction);

    void drop_foreign_key(const std::string &table_name, const std::string &restriction_name);

    void add_unique(const std::string &table_name, const std::vector<std::string> &column_list);

    Error::InsertError validate_insert_data(const std::string &table_name, const std::vector<Value> &values);

    Error::DeleteError validate_delete_data(const std::string &table_name, const std::vector<Value> &values);

    char *from_record_to_bytes(const std::string &table_name, const std::vector<Value> &values, std::size_t &length);

    std::vector<Value> from_bytes_to_record(const std::string &table_name, char *buffer, std::size_t length);

    std::vector<std::size_t> get_index_ids(const std::string &table_name);

    bool is_index_exist(const std::string &table_name, const std::string &column_name);

    std::string get_column_name(const std::string &table_name, std::size_t column_id);

    bool is_column_exist(const std::string &table_name, const std::string &column_name);

    Field get_column_info(const std::string &table_name, const std::string &column_name);

    bool is_table_exist(const std::string &table_name);

    RecordFile *get_record_file(const std::string &table_name);

    IndexFile *get_index_file(const std::string &table_name, const std::string &column_name);

    std::size_t get_record_length_limit(const std::string &table_name);

    std::size_t get_column_num(const std::string &table_name);
};
