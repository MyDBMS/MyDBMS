#include "SelectStmt.h"

SelectStmt::SelectStmt(){
    table_names.clear();
    selectors.clear();
    where_clauses.clear();
    limit = TABLE_ROW_MAX;
    offset = 0;
}