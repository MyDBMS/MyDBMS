#include "Column.h"

Column::Column(){
    table_name = "";
    column_name = "";
}

bool Column::operator < (const Column& other){
    return table_name < other.table_name || table_name == other.table_name && column_name < other.column_name;
}

bool Column::operator == (const Column& other){
    return table_name == other.table_name && column_name == other.column_name;
}