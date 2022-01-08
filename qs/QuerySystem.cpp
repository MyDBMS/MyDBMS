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

RecordSet QuerySystem::join_Recordset(RecordSet a, RecordSet b){
    RecordSet c;
    for(auto col : a.columns)
        c.columns.push_back(col);
    for(auto col : b.columns)
        c.columns.push_back(col);
    for(auto a_data : a.record)
        for(auto b_data : b.record){
            RecordData c_data;
            c_data.rid = b_data.rid;
            c_data.values.clear();
            for(auto a_value : a_data.values)
                c_data.values.push_back(a_value);
            for(auto b_value : b_data.values)
                c_data.values.push_back(b_value);
            c.record.push_back(c_data);
        }
    return c;
}

RecordSet QuerySystem::merge_Recordset(RecordSet a, RecordSet b){
    RecordSet c;
    for(auto col : a.columns)
        c.columns.push_back(col);
    for(auto col : b.columns)
        c.columns.push_back(col);
    int cnt = 0;
    for(auto a_data : a.record){
        RecordData c_data;
        c_data.rid = a_data.rid;
        c_data.values.clear();
        for(auto a_value : a_data.values)
            c_data.values.push_back(a_value);
        auto b_data = b.record[cnt];
        for(auto b_value : b_data.values)
            c_data.values.push_back(b_value);
        c.record.push_back(c_data);
        cnt ++;
    }
    return c;
}

bool QuerySystem::compare_Value(Value a, Value b, WhereClause::OP_Type op_type){
    if (a.type == Value::Type::NUL && b.type == Value::Type::NUL)
        return op_type == WhereClause::OP_Type::EQ;  //  两 null 比较，只有相等是 true
    if (a.type == Value::Type::NUL || b.type == Value::Type::NUL){
        ms.frontend->error("Null value cannot participate in comparison");
        flag = false;
        return false;
    }
    //  两者皆非 null
    if (a.type != b.type){  //  类型不一致
        ms.frontend->error("The two compared value types do not match");
        flag = false;
        return false;
    }
    switch (op_type){
        case WhereClause::OP_Type::EQ :
            return a == b;
            break;
        case WhereClause::OP_Type::LT :
            return a < b;
            break;
        case WhereClause::OP_Type::LE :
            return a <= b;
            break;
        case WhereClause::OP_Type::GT :
            return a > b;
            break;
        case WhereClause::OP_Type::GE :
            return a >= b;
            break;
        case WhereClause::OP_Type::NEQ :
            return a != b;
            break;
        default:
            assert(false);
            break;
    }
}

bool QuerySystem::is_some_column_exist(RecordSet record_set, Column column){
    for(auto col : record_set.columns){
        if (col == column) return true;
    }
    return false;
}

Value QuerySystem::get_column_value(std::vector<Column> columns, RecordData record_data, Column column){
    int cnt = 0;
    bool is_exist = false;
    for(auto col : columns){
        if (col == column){
            is_exist = true;
            break; 
        }
        cnt ++;
    }
    if (!is_exist){
        ms.frontend->error("column value is not exist");
        flag = false;
        return Value::make_value();
    }
    return record_data.values[cnt];
}

Frontend::Table QuerySystem::from_RecordSet_to_Table(RecordSet record_set){
    Frontend::Table table;
    table.clear();
    int cnt = 0;
    for(auto column : record_set.columns){
        Frontend::Column frontend_column;
        frontend_column.name = column.table_name + "." + column.column_name;
        frontend_column.values.clear();
        for(auto record : record_set.record)
            frontend_column.values.push_back(from_Value_to_string(record.values[cnt]));
        table.push_back(frontend_column);
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
            /* std::cout << "[QuerySystem] insert record " << table_name << " " << column_name << " " << key << std::endl;
            printf("RID is { %d , %d }\n", rid.page_id, rid.slot_id); */
            auto index_file = ms.get_index_file(table_name, column_name);
            index_file->insert_record(key, rid);
        }
    }
}

RecordSet QuerySystem::search_whole_table(std::string table_name){
    /* std::cout << "search whole table: "  << table_name << std::endl; */
    RecordSet record_set;
    int num = ms.get_column_num(table_name);
    for(int i = 0; i < num; i ++){
        Column col;
        col.table_name = table_name;
        col.column_name = ms.get_column_name(table_name, i);
        record_set.columns.push_back(col);
    }
    auto record_file = ms.get_record_file(table_name);
    // 向管理系统要一下长度上限
    std::size_t max_length = ms.get_record_length_limit(table_name);
    //  找到第一个
    RID rid = record_file->find_first();
    //  遍历
    while (true){
        /* printf("RID is { %d , %d }\n", rid.page_id, rid.slot_id); */
        if (rid.page_id == 0) break;  //  遍历完成
        char* buffer = new char[max_length];
        std::size_t length = record_file->get_record(rid, buffer);
        auto values = ms.from_bytes_to_record(table_name, buffer, length);
        RecordData rd;
        rd.rid = rid;
        rd.values = values;
        /* record_set.record.push_back(values); */
        record_set.record.push_back(rd);
        rid = record_file->find_next(rid);
    }
    return record_set;
}

RecordSet QuerySystem::search_by_index(std::string table_name, std::string column_name, int lower_bound, int upper_bound){
    /* std::cout << "[QuerySystem] search " << table_name << " " << column_name << " " << key << std::endl; */
    RecordSet record_set;
    int num = ms.get_column_num(table_name);
    for(int i = 0; i < num; i ++){
        Column col;
        col.table_name = table_name;
        col.column_name = ms.get_column_name(table_name, i);
        record_set.columns.push_back(col);
    }
    if (ms.is_index_exist(table_name, column_name)){
        auto index_file = ms.get_index_file(table_name, column_name);
        RID rid;
        IndexScan index_scan;
        index_file->search(lower_bound, upper_bound, index_scan);
        while (index_scan.get_next_entry(rid)){
            //  从记录系统里查找
            auto record_file = ms.get_record_file(table_name);
            // 向管理系统要一下长度上限
            std::size_t max_length = ms.get_record_length_limit(table_name);
            char* buffer = new char[max_length];
            std::size_t length = record_file->get_record(rid, buffer);
            auto values = ms.from_bytes_to_record(table_name, buffer, length);
            RecordData rd;
            rd.rid = rid;
            rd.values = values;
            /* record_set.record.push_back(values); */
            record_set.record.push_back(rd);
        }
    }
    return record_set;
}

RecordSet QuerySystem::search_where_clause(RecordSet input_result, WhereClause where_clause){
    if (where_clause.type == WhereClause::Type::OP_EXPR){  //  COL OP EXPR
        /* ms.frontend->warning("[QuerySystem] search where (COL OP EXPR) hasn't been finished yet"); */
        auto column = where_clause.column;
        RecordSet result;
        result.columns = input_result.columns;
        result.record.clear();
        for(auto data : input_result.record){
            auto col_val = get_column_value(input_result.columns, data, column);
            //  在这条产生式里，跳过 Null 值
            if (col_val.type == Value::NUL) continue;
            if (where_clause.expr.type == Expr::Type::VALUE){
                if (compare_Value(col_val, where_clause.expr.value, where_clause.op_type))
                    result.record.push_back(data);
            }
            else if (where_clause.expr.type == Expr::Type::COLUMN){
                auto cmp_val = get_column_value(input_result.columns, data, column);
                if (compare_Value(col_val, cmp_val, where_clause.op_type))
                    result.record.push_back(data);
            }
        }
        return result;
    }
    else if (where_clause.type == WhereClause::Type::OP_SELECT){  //  COL OP SELECT
        /* ms.frontend->warning("[QuerySystem] search where (COL OP SELECT) hasn't been finished yet"); */
        auto column = where_clause.column;
        RecordSet result;
        result.columns = input_result.columns;
        result.record.clear();
        if (where_clause.select_result.columns.size() != 1 || where_clause.select_result.record.size() != 1){
            ms.frontend->error("select where column = select_table, select_table's size isn't 1");
            flag = false;
            return input_result;
        }
        auto cmp_val = where_clause.select_result.record[0].values[0];
        for(auto data : input_result.record){
            auto col_val = get_column_value(input_result.columns, data, column);
            //  在这条产生式里，跳过 Null 值
            if (col_val.type == Value::NUL) continue;
            if (compare_Value(col_val, cmp_val, where_clause.op_type))
                result.record.push_back(data);
        }
        return result;
    }
    else if (where_clause.type == WhereClause::Type::IS_NULL_OR_NOT_NULL){  //  COL IS (NOT) NULL
        ms.frontend->warning("[QuerySystem] search where (COL IS (NOT) NULL) hasn't been finished yet");
        return input_result;
    }
    else if (where_clause.type == WhereClause::Type::IN_VALUE_LIST){  //  COL IN VALUE_LIST
        ms.frontend->warning("[QuerySystem] search where (COL IN VALUE_LIST) hasn't been finished yet");
        return input_result;
    }
    else if (where_clause.type == WhereClause::Type::IN_SELECT){  //  COL IN SELECT
        ms.frontend->warning("[QuerySystem] search where (COL IN SELECT) hasn't been finished yet");
        return input_result;
    }
    else if (where_clause.type == WhereClause::Type::LIKE_STR){  //  COL LIKE STR
        ms.frontend->warning("[QuerySystem] search where (COL LIKE STR) hasn't been finished yet");
        return input_result;
    }
    else{
        ms.frontend->error("[QuerySystem] search where doesn't match any rule");
        flag = false;
        return input_result;
    }
}

RecordSet QuerySystem::search_where_clauses(std::vector<std::string> table_names, std::vector<WhereClause> where_clauses){
    //  判断每个表是否都存在
    for(auto table_name : table_names)
        if (!ms.is_table_exist(table_name)){
            ms.frontend->error("table name does not exist");
            flag = false;
            RecordSet record_set;
            return record_set;
        }
    //  尝试用单表单列索引加速
    std::map<std::string, int> lb;  //  列名关键码范围的lower bound
    std::map<std::string, int> ub;  //  列名关键码范围的upper bound
    lb.clear();
    ub.clear();
    //  先求出每个列的查询范围
    for(auto where_clause : where_clauses){
        if (where_clause.type == WhereClause::Type::OP_EXPR){  //  只有这种形式可以使用索引
            auto column = where_clause.column;
            auto expr = where_clause.expr;
            if (expr.type == Expr::Type::VALUE && expr.value.type == Value::Type::INT){  //  必须是 table.column op key，且 key 是整型
                int x = expr.value.asInt();
                if (ms.is_index_exist(column.table_name, column.column_name)){  //  如果索引存在
                    //  还不在 map 里则要创建
                    if (lb.find(column.table_name + "_" + column.column_name) == lb.end())
                        lb[column.table_name + "_" + column.column_name] = 0;
                    if (ub.find(column.table_name + "_" + column.column_name) == ub.end())
                        ub[column.table_name + "_" + column.column_name] = INDEX_KEY_MAX - 1;
                    switch (where_clause.op_type) {
                        case WhereClause::OP_Type::EQ :
                            lb[column.table_name + "_" + column.column_name] = std::max(lb[column.table_name + "_" + column.column_name], x);
                            ub[column.table_name + "_" + column.column_name] = std::min(ub[column.table_name + "_" + column.column_name], x);
                            break;
                        case WhereClause::OP_Type::LT :
                            ub[column.table_name + "_" + column.column_name] = std::min(ub[column.table_name + "_" + column.column_name], x - 1);
                            break;
                        case WhereClause::OP_Type::LE :
                            ub[column.table_name + "_" + column.column_name] = std::min(ub[column.table_name + "_" + column.column_name], x);
                            break;
                        case WhereClause::OP_Type::GT :
                            lb[column.table_name + "_" + column.column_name] = std::max(ub[column.table_name + "_" + column.column_name], x + 1);
                            break;
                        case WhereClause::OP_Type::GE :
                            lb[column.table_name + "_" + column.column_name] = std::max(ub[column.table_name + "_" + column.column_name], x);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    bool is_use_index = false;
    Column index_column;
    int l;
    int r;
    // 任找出一个范围合法的作按索引查询
    // TODO: 多个合法，找一个表的列数最多的
    for(auto where_clause : where_clauses){
        auto column = where_clause.column;
        if (lb.find(column.table_name + "_" + column.column_name) != lb.end()){
            l = lb[column.table_name + "_" + column.column_name];
            r = ub[column.table_name + "_" + column.column_name];
            if (l <= r){
                is_use_index = true;
                index_column = column;
                break;
            }
        }
    }
    RecordSet init_result;  //  初始没有列，有一条记录（为了 join )
    RecordData rd;  //  空的
    init_result.record.push_back(rd);
    if (is_use_index){
        auto by_index_result = search_by_index(index_column.table_name, index_column.column_name, l, r);
        init_result = by_index_result;
    }
    //  考虑多表 join ，把其他表的全表与其合并
    for(auto table_name : table_names){
        if (!is_use_index || table_name != index_column.table_name){
            auto table_result = search_whole_table(table_name);
            init_result = join_Recordset(init_result, table_result);
        }
    }
    auto result = init_result;
    //  施加其他选择子
    for(auto where_clause : where_clauses){
        //  检查是否可以跳过这个选择子，只有当这个选择子效果和索引一致时
        if (is_use_index && where_clause.type == WhereClause::Type::OP_EXPR){
            auto column = where_clause.column;
            auto expr = where_clause.expr;
            if (column == index_column && expr.type == Expr::Type::VALUE) continue;
        }
        result = search_where_clause(result, where_clause);
    }
    return result;
}

RecordSet QuerySystem::search_selector(RecordSet input_result, Selector selector){
    /* printf("search selector\n"); */
    if (selector.type == Selector::Type::COL){
        auto column = selector.col;
        /* std::cout << "column type, " << column.table_name << " " << column.column_name << std::endl; */
        int cnt = 0;
        bool is_exist = false;
        for(auto col : input_result.columns){
            if (col == column){
                is_exist = true;
                break;
            }
            cnt ++;
        }
        if (!is_exist){
            ms.frontend->error("table.column name does not exist");
            flag = false;
            return input_result;
        }
        RecordSet result;
        result.columns.push_back(column);
        for(auto record : input_result.record){
            RecordData rd;
            rd.rid = record.rid;
            rd.values.push_back(record.values[cnt]);
            result.record.push_back(rd);
        }
        return result;
    }
    else return input_result;  //  TODO： 拓展的两种投影子
}

RecordSet QuerySystem::search_selectors(RecordSet input_result, std::vector<Selector> selectors){
    if (selectors.size() == 0) return input_result;  //  * 的情况
    /* printf("search selectors\n"); */
    RecordSet result;  //  空的
    bool is_empty = true;
    for(auto selector : selectors){
        auto output_result = search_selector(input_result, selector);
        /* ms.frontend->print_table(from_RecordSet_to_Table(output_result)); */
        if (is_empty){
            result = output_result;
            is_empty = false;
        }
        else result = merge_Recordset(result, output_result);
    }
    return result;
}

RecordSet QuerySystem::search(SelectStmt select_stmt){
    auto where_result = search_where_clauses(select_stmt.table_names, select_stmt.where_clauses);  //  先处理选择子，因为可以用索引加速
    auto result = search_selectors(where_result, select_stmt.selectors);  //  再处理投影子
    /* printf("search finished!\n");
    ms.frontend->print_table(from_RecordSet_to_Table(result)); */
    return result;
}

void QuerySystem::delete_record(std::string table_name, std::vector<WhereClause> where_clauses){
    //  构造查找语句，查询所有满足删除条件的记录
    SelectStmt select_stmt;
    select_stmt.table_names.push_back(table_name);
    select_stmt.where_clauses = where_clauses;
    auto result = search(select_stmt);
    //  遍历每一条需要删除的记录
    auto record_file = ms.get_record_file(table_name);
    for(auto record : result.record){
        RID rid = record.rid;
        auto values = record.values;
        //  TODO: 判断删除是否合法
        record_file->delete_record(rid);
        //  枚举所有的索引
        auto indexs = ms.get_index_ids(table_name);
        for(auto column_id : indexs){
            auto column_name = ms.get_column_name(table_name, column_id);
            auto value = values[column_id];
            if (value.type != Value::NUL){
                int key = value.asInt();
                /* std::cout << "[QuerySystem] insert record " << table_name << " " << column_name << " " << key << std::endl;
                printf("RID is { %d , %d }\n", rid.page_id, rid.slot_id); */
                auto index_file = ms.get_index_file(table_name, column_name);
                index_file->delete_record(key, rid);
            }
        }
    }
}

void QuerySystem::search_entry(SelectStmt select_stmt){
    flag = true;
    auto result = search(select_stmt);
    if (flag) ms.frontend->print_table(from_RecordSet_to_Table(result));
}