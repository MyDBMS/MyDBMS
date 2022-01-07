#include "QuerySystem.h"

QuerySystem::QuerySystem(ManageSystem &ms): ms(ms) {
}

std::string QuerySystem::from_Value_to_string(Value value){
    switch (value.type){
        case Value::Type::NUL:
            return "NULL";
            break;
        case Value::Type::STR:
            return value.asString();
            break;
        case Value::Type::INT:
            return std::to_string(value.asInt());
            break;
    }
}

Frontend::Table QuerySystem::from_RecordSet_to_Table(RecordSet record_set){
    Frontend::Table table;
    table.clear();
    int cnt = 0;
    for(auto name : record_set.column_name){
        Frontend::Column column;
        column.name = name;
        column.values.clear();
        for(auto record : record_set.record)
            column.values.push_back(from_Value_to_string(record[cnt]));
        table.push_back(column);
        cnt ++;
    }
    return table;
}

void QuerySystem::insert_record(std::string table_name, std::vector<Value> values){
    if (!ms.is_table_exist(table_name)){
        printf("Error: table is not exist!\n");
        return;
    }
    //  TODO: 调用系统管理模块接口，处理完整性检查
    std::size_t length;
    auto buffer = ms.from_record_to_bytes(table_name, values, length);
    //  插入记录
    auto record_file = ms.get_record_file(table_name);
    RID rid = record_file->insert_record(length, buffer);
    //  TODO: 处理不够插的情况
    //  枚举所有的索引
    auto indexs = ms.get_index_ids(table_name);
    for(auto column_id : indexs){
        auto column_name = ms.get_column_name(table_name, column_id);
        auto value = values[column_id];
        if (value.type != Value::NUL){
            int key = value.asInt();
            /* std::cout << "[QuerySystem] insert record " << table_name << " " << column_name << " " << key << std::endl; */
            auto index_file = ms.get_index_file(table_name, column_name);
            index_file->insert_record(key, rid);
        }
    }
}

RecordSet QuerySystem::search_by_index(std::string table_name, std::string column_name, int key){
    /* std::cout << "[QuerySystem] search " << table_name << " " << column_name << " " << key << std::endl; */
    RecordSet record_set;
    record_set.column_name.clear();
    int num = ms.get_column_num(table_name);
    for(int i = 0; i < num; i ++)
        record_set.column_name.push_back(ms.get_column_name(table_name, i));
    record_set.record.clear();
    if (ms.is_index_exist(table_name, column_name)){
        auto index_file = ms.get_index_file(table_name, column_name);
        RID rid;
        IndexScan index_scan;
        index_file->search(key, key, index_scan);
        while (index_scan.get_next_entry(rid)){
            //  从记录系统里查找
            auto record_file = ms.get_record_file(table_name);
            // 向管理系统要一下长度上限
            std::size_t max_length = ms.get_record_length_limit(table_name);
            char* buffer = new char[max_length];
            std::size_t length = record_file->get_record(rid, buffer);
            auto values = ms.from_bytes_to_record(table_name, buffer, length);
            record_set.record.push_back(values);
        }
    }
    return record_set;
}

void QuerySystem::search(std::string table_name, std::string column_name, int key){
    if (!ms.is_table_exist(table_name)){
        printf("Error: table is not exist!\n");
        return;
    }
    auto result = search_by_index(table_name, column_name, key);
    ms.frontend->print_table(from_RecordSet_to_Table(result));
}