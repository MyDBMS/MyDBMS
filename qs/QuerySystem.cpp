#include "QuerySystem.h"

QuerySystem::QuerySystem(ManageSystem &ms): ms(ms) {
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
        int key = values[column_id].asInt();
        auto index_file = ms.get_index_file(table_name, column_name);
        index_file->insert_record(key, rid);
    }
}

void QuerySystem::search(std::string table_name, std::string column_name, int key){
    if (!ms.is_table_exist(table_name)){
        printf("Error: table is not exist!\n");
        return;
    }
    if (ms.is_index_exist(table_name, column_name)){
        auto index_file = ms.get_index_file(table_name, column_name);
        RID rid;
        IndexScan index_scan;
        index_file->search(key, key, index_scan);
        if (index_scan.get_next_entry(rid)){
            //  从记录系统里查找
            auto record_file = ms.get_record_file(table_name);
            // 向管理系统要一下长度上限
            std::size_t max_length = ms.get_record_length_limit(table_name);
            char* buffer = new char[max_length];
            std::size_t length = record_file->get_record(rid, buffer);
            auto values = ms.from_bytes_to_record(table_name, buffer, length);
            for(auto value : values){
                switch (value.type){
                    case Value::Type::NUL:
                        std::cout << "NULL ";
                        break;
                    case Value::Type::STR:
                        std::cout << value.asString();
                        break;
                    case Value::Type::INT:
                        std::cout << value.asInt();
                        break;
                }
            }
            printf("\n");
        }
    }
    else{
        // TODO: 没有索引则暴力扫描
    }
}