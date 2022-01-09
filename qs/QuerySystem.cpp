#include "QuerySystem.h"

#include <regex>

QuerySystem::QuerySystem(ManageSystem &ms): ms(ms) {
}

void QuerySystem::QS_error(const std::string &msg){
    if (flag){
        ms.frontend->error(msg);
        flag = false;
    }
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
    //  处理无法合并的情况
    if (a.record.size() != b.record.size()){
        QS_error("Internal warning: the arguements of merge_Recordset don't contain the same number of rows");
        return a;
    }
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

RecordSet QuerySystem::add_RecordSet(RecordSet a, RecordSet b){
    RecordSet c = a;
    for(auto record : b.record)
        c.record.push_back(record);
    return c;
}

bool QuerySystem::compare_Value(Value a, Value b, WhereClause::OP_Type op_type){
    if (a.type == Value::Type::NUL && b.type == Value::Type::NUL){ //  两 null 比较
        /* return op_type == WhereClause::OP_Type::EQ;  */ 
        return false;
    }
    if (a.type == Value::Type::NUL || b.type == Value::Type::NUL){  //  null 和正常类型比较
        /* QS_error("Null value cannot participate in comparison"); */
        return false;
    }
    //  两者皆非 null
    if (a.type != b.type){  //  类型不一致
        QS_error("The two compared value types do not match");
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
        QS_error("column value is not exist");
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
        QS_error("column value is not exist");
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
            QS_error("update value's type is not expected");
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
    //  如果一个域字段定义为 FLOAT ，要把解析出的整数转换过去
    for(int i = 0; i < values.size(); i ++){
        auto column_name = ms.get_column_name(table_name, i);
        if (values[i].type == Value::INT && ms.get_column_info(table_name, column_name).type == Field::Type::FLOAT){
            auto value = Value::make_value(float(values[i].asInt()));
            values[i] = value;
        }
    }
    switch (ms.validate_insert_data(table_name, values)) {
        case Error::NONE:
            break;
        case Error::VALUE_COUNT_MISMATCH:
            QS_error("Field count mismatch!");
            return;
        case Error::TYPE_MISMATCH:
            QS_error("Field type mismatch!");
            return;
        case Error::STR_TOO_LONG:
            QS_error("String value is too long!");
            return;
        case Error::FIELD_CANNOT_BE_NULL:
            QS_error("Field cannot be null!");
            return;
        case Error::UNIQUE_RESTRICTION_FAIL:
            QS_error("Unique constraint failed!");
            return;
        case Error::PRIMARY_RESTRICTION_FAIL:
            QS_error("Primary constraint failed!");
            return;
        case Error::INSERT_FOREIGN_RESTRICTION_FAIL:
            QS_error("Foreign constraint failed!");
            return;
        case Error::TABLE_DOES_NOT_EXIST:
            QS_error("Table does not exist!");
            return;
        default:
            QS_error("Internal warning: insertion failed due to unhandled reason.");
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
            auto index_file = ms.get_index_file(table_name, column_name);
            index_file->insert_record(key, rid);
        }
    }
    delete [] buffer;
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

//  已有一个表的结果，用其驱动得到两表 join 后根据 where_clause 筛选的结果
//  其中 where_clause 一定是 another_table.column1 = table_name.column2
/// 并且保证 table_name 在 column2 上建有索引
RecordSet QuerySystem::search_by_another_table(RecordSet input_result, std::string table_name, Column column1, Column column2){
    std::map<Value, RecordSet> rs_map;
    rs_map.clear();
    for(auto record : input_result.record){
        auto record_val = get_column_value(input_result.columns, record, column1);
        if (rs_map.find(record_val) == rs_map.end()){
            RecordSet rs;
            rs.columns = input_result.columns;
            rs.record.push_back(record);
            rs_map[record_val] = rs;
        }
        else
            rs_map[record_val].record.push_back(record);
    }
    RecordSet result;
    for(auto ite : rs_map){
        auto rs = ite.second;
        int key = ite.first.asInt();
        auto search_result = search_by_index(table_name, column2.column_name, key, key);
        auto join_result = join_Recordset(rs, search_result);
        result = add_RecordSet(join_result, result);
    }
    return result;
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
                auto cmp_val = get_column_value(input_result.columns, data, where_clause.expr.column);
                if (compare_Value(col_val, cmp_val, where_clause.op_type)){
                    result.record.push_back(data);
                }
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
            QS_error("select where column = select_table, select_table's size isn't 1");
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
            QS_error("select where column in select_table, select_table's isn't a list");
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
            if (val_map.find(col_val) != val_map.end()) 
                result.record.push_back(data);
        }
        return result;
    }
    else if (where_clause.type == WhereClause::Type::LIKE_STR){  //  COL LIKE STR
        /* ms.frontend->warning("[QuerySystem] search where (COL LIKE STR) hasn't been finished yet"); */
        auto column = where_clause.column;
        //  判断这一列是不是字符串类型
        if (ms.get_column_info(column.table_name, column.column_name).type != Field::Type::STR){
            QS_error("select COL LIKE STR, field type must be string");
            return input_result;
        }
        std::string str = where_clause.str;
        str = str.substr(1, str.length() - 2);
        std::string regex_str = "";
        for(auto ch : str){
            if (ch == '%')
                regex_str += ".*";
            else if (ch == '_')
                regex_str += ".";
            else regex_str += ch;
        }
        RecordSet result;
        result.columns = input_result.columns;
        result.record.clear();
        for(auto data : input_result.record){
            auto col_val = get_column_value(input_result.columns, data, column);
            if (col_val.type == Value::Type::NUL) continue;  //  忽略 NULL 值，必须使用 COL IS NULL
            if (std::regex_match(col_val.asString(), std::regex(regex_str))) 
                result.record.push_back(data);
        }
        return result;
    }
    else{
        QS_error("[QuerySystem] search where doesn't match any rule");
        return input_result;
    }
}

RecordSet QuerySystem::search_where_clauses(std::vector<std::string> table_names, std::vector<WhereClause> where_clauses){
    //  判断每个表是否都存在
    for(auto table_name : table_names)
        if (!ms.is_table_exist(table_name)){
            QS_error("table name does not exist");
            RecordSet record_set;
            return record_set;
        }
    //  for debug
    /* {
        RecordSet init_result;  //  初始没有列，有一条记录（为了 join )
        RecordData rd;  //  空的
        init_result.record.push_back(rd);
        for(auto table_name : table_names)
            init_result = join_Recordset(init_result, search_whole_table(table_name));
        auto result = init_result;
        //  施加其他选择子
        for(auto where_clause : where_clauses){
            result = search_where_clause(result, where_clause);
        }
        return result;
    } */
    //  为每个表名判断是否可以利用索引，可以的话直接将索引的wc和其他非join型wc消耗掉
    std::map<std::string, int> lb;  //  列名关键码范围的lower bound
    std::map<std::string, int> ub;  //  列名关键码范围的upper bound
    lb.clear();
    ub.clear();
    //  先求出每个列的查询范围
    for(auto where_clause : where_clauses){
        if (where_clause.type == WhereClause::Type::OP_EXPR){  //  是 COL OP EXPR 型
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
    std::map<std::string, bool> is_use_index;
    is_use_index.clear();
    std::map<std::string, int> l_val;
    l_val.clear();
    std::map<std::string, int> r_val;
    r_val.clear();
    std::map<std::string, Column> index_column;
    index_column.clear();
    // 对每个表任找出一个范围合法的作按索引查询
    for(auto where_clause : where_clauses){
        auto column = where_clause.column;
        if (lb.find(column.table_name + "_" + column.column_name) != lb.end()){
            int l = lb[column.table_name + "_" + column.column_name];
            int r = ub[column.table_name + "_" + column.column_name];
            if (l <= r){
                /* continue;  //  for debug */
                is_use_index[column.table_name] = true;
                l_val[column.table_name] = l;
                r_val[column.table_name] = r;
                index_column[column.table_name] = column;
                break;
            }
        }
    }
    std::vector<RecordSet> result_vec;
    result_vec.clear();
    std::map<std::string, int> vec_index;
    vec_index.clear();
    for(auto table_name : table_names){
        vec_index[table_name] = -1;
        if (is_use_index.find(table_name) != is_use_index.end() && is_use_index[table_name]){
            auto index_result = search_by_index(table_name, index_column[table_name].column_name, l_val[table_name], r_val[table_name]);
            //  施加其他同为 table_name 的选择子
            for(int i = 0; i < where_clauses.size(); i ++){
                auto where_clause = where_clauses[i];
                //  跳过施加了索引的
                if (where_clause.type == WhereClause::Type::OP_EXPR){
                    auto column = where_clause.column;
                    auto expr = where_clause.expr;
                    if (column == index_column[table_name] && expr.type == Expr::Type::VALUE){
                        where_clauses[i].is_solved = true;
                        continue;
                    }
                }
                if (where_clause.column.table_name == table_name){
                    //  如果是和别的表有关联，则跳过
                    if (where_clause.type == WhereClause::Type::OP_EXPR 
                        && where_clause.expr.type == Expr::Type::COLUMN
                        && where_clause.expr.column.table_name != table_name) continue;
                    //  否则，消耗掉
                    index_result = search_where_clause(index_result, where_clause);
                    where_clauses[i].is_solved = true;
                }
            }
            vec_index[table_name] = result_vec.size();
            result_vec.push_back(index_result);
        }
    }
    //  TODO: 用驱动法 join 
    /* for(auto table_name : table_names){
        if (vec_index[table_name] == -1){
            auto whole_result = search_whole_table(table_name);
            vec_index[table_name] = result_vec.size();
            result_vec.push_back(whole_result);
        }
    } */
    //  每次找到可以被驱动的表，否则任找一个
    while (true){
        bool bz = false;
        //  驱动笛卡尔积
        for(int i = 0; i < where_clauses.size(); i ++){
            auto where_clause = where_clauses[i];
            if (where_clause.is_solved) continue;
            if (where_clause.type == WhereClause::Type::OP_EXPR && where_clause.expr.type == Expr::Type::COLUMN){
                auto column1 = where_clause.column;
                auto column2 = where_clause.expr.column;
                auto table1 = where_clause.column.table_name;
                auto table2 = where_clause.expr.column.table_name;
                if (table1 == table2) continue;
                if (vec_index[table1] == -1 && vec_index[table2] == -1) continue;
                if (vec_index[table1] != -1 && vec_index[table2] != -1) continue;
                if (vec_index[table1] == -1) std::swap(column1, column2);
                if (ms.is_index_exist(column2.table_name, column2.column_name)){
                    bz = true;
                    int loc = vec_index[column1.table_name];
                    result_vec[loc] = search_by_another_table(result_vec[loc], column2.table_name, column1, column2);
                    vec_index[column2.table_name] = loc;
                    where_clauses[i].is_solved = true;
                    break;
                }
            }
        }
        //  随便找一个
        if (!bz){
            for(auto table_name : table_names)
                if (vec_index[table_name] == -1){
                    bz = true;
                    vec_index[table_name] = result_vec.size();
                    result_vec.push_back(search_whole_table(table_name));
                    break;
                }
            if (!bz) break;
        }
    }
    //  将结果列表里的全部卷起来
    RecordSet init_result;  //  初始没有列，有一条记录（为了 join )
    RecordData rd;  //  空的
    init_result.record.push_back(rd);
    for(auto rt : result_vec)
        init_result = join_Recordset(init_result, rt);
    auto result = init_result;
    //  施加其他选择子
    for(auto where_clause : where_clauses){
        if (where_clause.is_solved) continue;
        result = search_where_clause(result, where_clause);
    }
    return result;
}

RecordSet QuerySystem::search_selector(RecordSet input_result, Selector selector, GroupBy group_by){
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
            QS_error("table.column name does not exist");
            return input_result;
        }
        RecordSet result;
        result.columns.push_back(column);
        //  处理有 GROUP BY 的情况
        if (!group_by.is_empty){
            if (selector.col != group_by.column){
                QS_error("select column name and group by column name are different");
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
            QS_error("String type can only aggregate by Count()");
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
            std::map<Value, bool> exist_map;
            calc_map.clear();
            count_map.clear();
            exist_map.clear();
            for(auto record : input_result.record){
                RecordData rd;
                Value group_by_val;
                //  没有 group by 的话，全部聚合到一起
                if (group_by.is_empty) group_by_val = Value::make_value();
                else group_by_val = get_column_value(input_result.columns, record, group_by.column);
                exist_map[group_by_val] = true;
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
            for(auto ite : exist_map){
                RecordData rd;
                //  处理全 null
                if (count_map.find(ite.first) == count_map.end() || count_map[ite.first] == 0){
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
                Value result_val = calc_map[ite.first];
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
        QS_error("[QuerySystem] search selector doesn't match any rule");
        return input_result;
    }
}

RecordSet QuerySystem::search_selectors(RecordSet input_result, std::vector<Selector> selectors, GroupBy group_by){
    if (selectors.size() == 0) return input_result;  //  * 的情况
    RecordSet result;  //  空的
    bool is_empty = true;
    for(auto selector : selectors){
        if (selector.type == Selector::Type::CT){  //  这种情况只应该有一个投影子，返回一列一个数据表示数据量
            if (selectors.size() > 1){
                QS_error("Count(*) must be the only selector");
                return input_result;
            }
            return search_selector(input_result, selector, group_by);  //  唯一，直接返回
        }
        auto output_result = search_selector(input_result, selector, group_by);
        if (is_empty){
            result = output_result;
            is_empty = false;
        }
        else result = merge_Recordset(result, output_result);
    }
    return result;
}

RecordSet QuerySystem::search_limit_offset(RecordSet input_result, int limit, int offset){
    if (offset == 0 && limit >= input_result.record.size()) return input_result;
    RecordSet result;
    result.columns = input_result.columns;
    result.record.clear();
    int index = 0;
    int cnt = 0;
    for(auto record : input_result.record){
        if (index >= offset){
            cnt ++;
            if (cnt <= limit) result.record.push_back(record);
        }
        index ++;
    }
    return result;
}

RecordSet QuerySystem::search(SelectStmt select_stmt){
    /* printf("search!\n");
    select_stmt.print_stmt(); */
    if (!ms.ensure_db_valid()){
        flag = false;
        return RecordSet();
    }
    //  检查表名是否有重复
    std::map<std::string, bool> ha_map;
    ha_map.clear();
    for(auto table_name : select_stmt.table_names){
        if (ha_map.find(table_name) != ha_map.end()){
            QS_error("Not unique table");
            return RecordSet();
        }
        ha_map[table_name] = true;
    }
    auto where_result = search_where_clauses(select_stmt.table_names, select_stmt.where_clauses);  //  先处理选择子，因为可以用索引加速
    auto selector_result = search_selectors(where_result, select_stmt.selectors, select_stmt.group_by);  //  再处理投影子
    auto result = search_limit_offset(selector_result, select_stmt.limit, select_stmt.offset);
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
        switch (ms.validate_delete_data(table_name, values)) {
            case Error::DELETE_NONE:
                break;
            case Error::DELETE_TABLE_DOES_NOT_EXIST:
                QS_error("Table does not exist!");
                continue;
            case Error::DELETE_FOREIGN_RESTRICTION_FAIL:
                QS_error("Foreign constraint failed.");
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
    //  如果一个域字段定义为 FLOAT ，要把解析出的整数转换过去
    for(int i = 0; i < update_values.size(); i ++){
        auto column_name = column_names[i];
        if (update_values[i].type == Value::INT && ms.get_column_info(table_name, column_name).type == Field::Type::FLOAT){
            auto value = Value::make_value(float(update_values[i].asInt()));
            update_values[i] = value;
        }
    }
    //  构造查找语句，查询所有满足更改条件的记录
    SelectStmt select_stmt;
    select_stmt.table_names.push_back(table_name);
    select_stmt.where_clauses = where_clauses;
    auto result = search(select_stmt);
    //  删除原先的记录
    delete_record(table_name, where_clauses);
    return;
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
    if (!ms.ensure_db_valid()){
        flag = false;
        return;
    }
    auto result = search(select_stmt);
    if (flag) ms.frontend->print_table(from_RecordSet_to_Table(result));
}