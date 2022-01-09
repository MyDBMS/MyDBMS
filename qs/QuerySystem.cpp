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
        case Value::Type::FLOAT:
            return std::to_string(value.asFloat());
            break;
    }
}

RecordSet QuerySystem::join_Recordset(RecordSet a, RecordSet b){
    /* printf("join A and B\n");
    ms.frontend->print_table(from_RecordSet_to_Table(a));
    ms.frontend->print_table(from_RecordSet_to_Table(b)); */
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
    if (a.type == Value::Type::NUL && b.type == Value::Type::NUL){ //  两 null 比较
        /* return op_type == WhereClause::OP_Type::EQ;  */ 
        return false;
    }
    if (a.type == Value::Type::NUL || b.type == Value::Type::NUL){  //  null 和正常类型比较
        /* ms.frontend->error("Null value cannot participate in comparison");
        flag = false; */
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

void QuerySystem::set_column_value(std::vector<Column> columns, RecordData &record_data, Column column, Value value){
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
        return;
    }
    if (value.type == Value::Type::NUL){
        record_data.values[cnt] = value;
        return;
    }
    else{
        auto exp_type = ms.get_column_info(column.table_name, column.column_name).type;
        bool is_valid = true;
        switch (exp_type){
            case Field::Type::STR :
                if (value.type != Value::Type::STR) is_valid = false;
                break;
            case Field::Type::INT :
                if (value.type != Value::Type::INT) is_valid = false;
                break;
            case Field::Type::FLOAT :
                if (value.type != Value::Type::FLOAT) is_valid = false;
                break;
            default:
                assert(false);
                break;
        }
        if (!is_valid){
            ms.frontend->error("update value's type is not expected");
            flag = false;
            return;
        }
        record_data.values[cnt] = value;
    }
}

Frontend::Table QuerySystem::from_RecordSet_to_Table(RecordSet record_set){
    Frontend::Table table;
    table.clear();
    int cnt = 0;
    for(auto column : record_set.columns){
        Frontend::Column frontend_column;
        if (column.has_table) frontend_column.name = column.table_name + "." + column.column_name;
        else frontend_column.name = column.column_name;
        frontend_column.values.clear();
        for(auto record : record_set.record)
            frontend_column.values.push_back(from_Value_to_string(record.values[cnt]));
        table.push_back(frontend_column);
        cnt ++;
    }
    return table;
}

void QuerySystem::insert_record(std::string table_name, std::vector<Value> values){
    if (!ms.ensure_db_valid()){
        return;
    }
    flag = false;
    switch (ms.validate_insert_data(table_name, values)) {
        case Error::NONE:
            flag = true;
            break;
        case Error::VALUE_COUNT_MISMATCH:
            ms.frontend->error("Field count mismatch!");
            return;
        case Error::TYPE_MISMATCH:
            ms.frontend->error("Field type mismatch!");
            return;
        case Error::STR_TOO_LONG:
            ms.frontend->error("String value is too long!");
            return;
        case Error::FIELD_CANNOT_BE_NULL:
            ms.frontend->error("Field cannot be null!");
            return;
        case Error::UNIQUE_RESTRICTION_FAIL:
            ms.frontend->error("Unique constraint failed!");
            return;
        case Error::PRIMARY_RESTRICTION_FAIL:
            ms.frontend->error("Primary constraint failed!");
            return;
        case Error::INSERT_FOREIGN_RESTRICTION_FAIL:
            ms.frontend->error("Foreign constraint failed!");
            return;
        case Error::TABLE_DOES_NOT_EXIST:
            ms.frontend->error("Table does not exist!");
            return;
        default:
            ms.frontend->error("Internal warning: insertion failed due to unhandled reason.");
            return;
        }
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
        auto column = where_clause.column;
        bool clause_is_null = where_clause.null_or_not_null;
        RecordSet result;
        result.columns = input_result.columns;
        result.record.clear();
        for(auto data : input_result.record){
            auto col_val = get_column_value(input_result.columns, data, column);
            bool col_is_null = false;
            //  在这条产生式里，跳过 Null 值
            if (col_val.type == Value::NUL) col_is_null = true;
            if (col_is_null == clause_is_null){
                result.record.push_back(data);
            }
        }
        return result;
    }
    else if (where_clause.type == WhereClause::Type::IN_VALUE_LIST){  //  COL IN VALUE_LIST
        /* ms.frontend->warning("[QuerySystem] search where (COL IN VALUE_LIST) hasn't been finished yet"); */
        std::map<Value, bool> val_map;
        val_map.clear();
        for(auto val : where_clause.value_list)
            val_map[val] = true;
        auto column = where_clause.column;
        RecordSet result;
        result.columns = input_result.columns;
        result.record.clear();
        for(auto data : input_result.record){
            auto col_val = get_column_value(input_result.columns, data, column);
            if (col_val.type == Value::Type::NUL) continue;  //  忽略 NULL 值，必须使用 COL IS NULL
            if (val_map.find(col_val) != val_map.end()) result.record.push_back(data);
        }
        return result;
    }
    else if (where_clause.type == WhereClause::Type::IN_SELECT){  //  COL IN SELECT
        /* ms.frontend->warning("[QuerySystem] search where (COL IN SELECT) hasn't been finished yet"); */
        std::map<Value, bool> val_map;
        val_map.clear();
        if (where_clause.select_result.columns.size() != 1){  //  不是 value_list
            ms.frontend->error("select where column in select_table, select_table's isn't a list");
            flag = false;
            return input_result;
        }
        for(auto input_record : where_clause.select_result.record){
            auto val = input_record.values[0];
            val_map[val] = true;
        }
        auto column = where_clause.column;
        RecordSet result;
        result.columns = input_result.columns;
        result.record.clear();
        for(auto data : input_result.record){
            auto col_val = get_column_value(input_result.columns, data, column);
            if (col_val.type == Value::Type::NUL) continue;  //  忽略 NULL 值，必须使用 COL IS NULL
            if (val_map.find(col_val) != val_map.end()) result.record.push_back(data);
        }
        return result;
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

RecordSet QuerySystem::search_selector(RecordSet input_result, Selector selector, GroupBy group_by){
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
        //  处理有 GROUP BY 的情况
        if (!group_by.is_empty){
            if (selector.col != group_by.column){
                ms.frontend->error("select column name and group by column name are different");
                flag = false;
                return input_result;
            }
            std::map<Value, bool> calc_map;
            calc_map.clear();
            for(auto record : input_result.record){
                auto group_by_val = get_column_value(input_result.columns, record, group_by.column);
                calc_map[group_by_val] = true;
            }
            for(auto ite : calc_map){
                RecordData rd;
                rd.values.push_back(ite.first);
                result.record.push_back(rd);
            }
            return result;
        }
        for(auto record : input_result.record){
            RecordData rd;
            rd.rid = record.rid;
            rd.values.push_back(record.values[cnt]);
            result.record.push_back(rd);
        }
        return result;
    }
    else if (selector.type == Selector::Type::AGR_COL){
        RecordSet result;
        Column column;
        column.has_table = false;
        //  不允许对字符类型作非Count聚合
        bool is_str_type = ms.get_column_info(selector.col.table_name, selector.col.column_name).type == Field::Type::STR;
        if (is_str_type && selector.agr_type != Selector::Aggregator_Type::COUNT){
            ms.frontend->error("String type can only aggregate by Count()");
            flag = false;
            return input_result;
        }
        switch (selector.agr_type){
            case Selector::Aggregator_Type::COUNT :
                column.column_name = "Count(" + selector.col.table_name + "." + selector.col.column_name + ")";
                break;
            case Selector::Aggregator_Type::AVERAGE :
                column.column_name = "Average(" + selector.col.table_name + "." + selector.col.column_name + ")";
                break;
            case Selector::Aggregator_Type::MAX :
                column.column_name = "Max(" + selector.col.table_name + "." + selector.col.column_name + ")";
                break;
            case Selector::Aggregator_Type::MIN :
                column.column_name = "Min(" + selector.col.table_name + "." + selector.col.column_name + ")";
                break;
            case Selector::Aggregator_Type::SUM :
                column.column_name = "Sum(" + selector.col.table_name + "." + selector.col.column_name + ")";
                break;
            default :
                assert(false);
                break;
        }
        result.columns.push_back(column);
        
        //  处理聚合计算
        {
            int cnt_not_null = 0;
            std::map<Value, Value> calc_map;
            std::map<Value, int> count_map;
            calc_map.clear();
            count_map.clear();
            for(auto record : input_result.record){
                RecordData rd;
                Value group_by_val;
                //  没有 group by 的话，全部聚合到一起
                if (group_by.is_empty) group_by_val = Value::make_value();
                else group_by_val = get_column_value(input_result.columns, record, group_by.column);
                Value record_val = get_column_value(input_result.columns, record, selector.col);
                if (record_val.type == Value::Type::NUL) continue;  //  不对 Null 作聚合
                else cnt_not_null ++;
                if (calc_map.find(group_by_val) == calc_map.end()){  //  创建初值
                    count_map[group_by_val] = 1;
                    switch (selector.agr_type){
                        case Selector::Aggregator_Type::COUNT :
                            calc_map[group_by_val] = Value::make_value(1);
                            break;
                        case Selector::Aggregator_Type::SUM :
                        case Selector::Aggregator_Type::AVERAGE :
                        case Selector::Aggregator_Type::MAX :
                        case Selector::Aggregator_Type::MIN :
                            calc_map[group_by_val] = record_val;
                            break;
                        default :
                            assert(false);
                            break;
                    }
                }
                else{  //  更新值
                    auto val = calc_map[group_by_val];
                    count_map[group_by_val] += 1;
                    switch (selector.agr_type){
                        case Selector::Aggregator_Type::COUNT :
                            calc_map[group_by_val] = val + Value::make_value(1);
                            break;
                        case Selector::Aggregator_Type::SUM :
                        case Selector::Aggregator_Type::AVERAGE :
                            calc_map[group_by_val] = val + record_val;
                            break;
                        case Selector::Aggregator_Type::MAX :
                            if (record_val > val)
                                calc_map[group_by_val] = record_val;
                            break;
                        case Selector::Aggregator_Type::MIN :
                            if (record_val < val)
                                calc_map[group_by_val] = record_val;
                            break;
                        default :
                            assert(false);
                            break;
                    }
                }
            }
            //  空表或没有任何非Null
            /* if (cnt_not_null == 0){
                RecordData rd;
                switch (selector.agr_type){
                    case Selector::Aggregator_Type::COUNT :
                        rd.values.push_back(Value::make_value(0));
                        result.record.push_back(rd);
                        return result;
                        break;
                    case Selector::Aggregator_Type::SUM :
                    case Selector::Aggregator_Type::AVERAGE :
                    case Selector::Aggregator_Type::MAX :
                    case Selector::Aggregator_Type::MIN :
                        rd.values.push_back(Value::make_value());
                        result.record.push_back(rd);
                        return result;
                        break;
                    default :
                        assert(false);
                        break;
                }
            } */
            //  否则，按 group_by_val 的偏序插入 RecordSet
            for(auto ite : calc_map){
                Value result_val = ite.second;
                RecordData rd;
                //  处理全 null
                if (count_map[ite.first] == 0){
                    switch (selector.agr_type){
                        case Selector::Aggregator_Type::COUNT :
                            rd.values.push_back(Value::make_value(0));
                            result.record.push_back(rd);
                            break;
                        case Selector::Aggregator_Type::SUM :
                        case Selector::Aggregator_Type::AVERAGE :
                        case Selector::Aggregator_Type::MAX :
                        case Selector::Aggregator_Type::MIN :
                            rd.values.push_back(Value::make_value());
                            result.record.push_back(rd);
                            break;
                        default :
                            assert(false);
                            break;
                    }
                    continue;
                }
                if (selector.agr_type == Selector::Aggregator_Type::AVERAGE){
                    if (result_val.type == Value::Type::INT)
                        result_val = Value::make_value((float)result_val.asInt() / count_map[ite.first]);
                    else 
                        result_val = Value::make_value(result_val.asFloat() / count_map[ite.first]);
                }
                rd.values.push_back(result_val);
                result.record.push_back(rd);
            }
        }
        return result;
    }
    else if (selector.type == Selector::Type::CT){
        RecordSet result;
        Column column;
        column.has_table = false;
        column.column_name = "Count(*)";
        result.columns.push_back(column);
        RecordData rd;
        rd.values.push_back(Value::make_value(int(input_result.record.size())));
        result.record.push_back(rd);   
        return result;
    }
    else{
        ms.frontend->error("[QuerySystem] search selector doesn't match any rule");
        flag = false;
        return input_result;
    }
}

RecordSet QuerySystem::search_selectors(RecordSet input_result, std::vector<Selector> selectors, GroupBy group_by){
    if (selectors.size() == 0) return input_result;  //  * 的情况
    /* printf("search selectors\n"); */
    RecordSet result;  //  空的
    bool is_empty = true;
    for(auto selector : selectors){
        if (selector.type == Selector::Type::CT){  //  这种情况只应该有一个投影子，返回一列一个数据表示数据量
            if (selectors.size() > 1){
                ms.frontend->error("Count(*) must be the only selector");
                flag = false;
                return input_result;
            }
            return search_selector(input_result, selector, group_by);  //  唯一，直接返回
        }
        auto output_result = search_selector(input_result, selector, group_by);
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
    if (!ms.ensure_db_valid()){
        flag = false;
        return RecordSet();
    }
    //  检查表名是否有重复
    std::map<std::string, bool> ha_map;
    ha_map.clear();
    for(auto table_name : select_stmt.table_names){
        if (ha_map.find(table_name) != ha_map.end()){
            ms.frontend->error("Not unique table");
            flag = false;
            return RecordSet();
        }
        ha_map[table_name] = true;
    }
    auto where_result = search_where_clauses(select_stmt.table_names, select_stmt.where_clauses);  //  先处理选择子，因为可以用索引加速
    auto result = search_selectors(where_result, select_stmt.selectors, select_stmt.group_by);  //  再处理投影子
    /* printf("search finished!\n");
    ms.frontend->print_table(from_RecordSet_to_Table(result)); */
    return result;
}

void QuerySystem::delete_record(std::string table_name, std::vector<WhereClause> where_clauses){
    if (!ms.ensure_db_valid()){
        flag = false;
        return;
    }
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
        flag = false;
        switch (ms.validate_delete_data(table_name, values)) {
            case Error::DELETE_NONE:
                flag = true;
                break;
            case Error::DELETE_TABLE_DOES_NOT_EXIST:
                ms.frontend->error("Table does not exist!");
                continue;
            case Error::DELETE_FOREIGN_RESTRICTION_FAIL:
                ms.frontend->error("Foreign constraint failed.");
                continue;
            default:
                ms.frontend->warning("Internal warning: unhandled invalid deletion.");
                continue;
        }
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

void QuerySystem::update_record(std::string table_name, std::vector<std::string> column_names, std::vector<Value> update_values, std::vector<WhereClause> where_clauses){
    if (!ms.ensure_db_valid()){
        flag = false;
        return;
    }
    //  构造查找语句，查询所有满足更改条件的记录
    SelectStmt select_stmt;
    select_stmt.table_names.push_back(table_name);
    select_stmt.where_clauses = where_clauses;
    auto result = search(select_stmt);
    //  删除原先的记录
    delete_record(table_name, where_clauses);
    //  加入这些记录更改后
    for(auto record : result.record){
        auto new_record = record;
        int cnt = 0;
        for(auto col_name : column_names){
            Column col;
            col.table_name = table_name;
            col.column_name = col_name;
            set_column_value(result.columns, new_record, col, update_values[cnt]);
            cnt ++;
        }
        //  更改完毕，插入
        insert_record(table_name, new_record.values);
    }
}

void QuerySystem::search_entry(SelectStmt select_stmt){
    flag = true;
    if (!ms.ensure_db_valid()){
        flag = false;
        return;
    }
    auto result = search(select_stmt);
    if (flag) ms.frontend->print_table(from_RecordSet_to_Table(result));
}