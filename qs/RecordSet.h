#pragma once

#include "../ms/Value.h"
#include "../rs/RecordSystem.h"
#include "Column.h"

#include <vector>

struct RecordData {
public:
    RID rid;
    std::vector<Value> values;

    RecordData();
};

struct RecordSet {
public:
    std::vector<Column> columns;
    std::vector<RecordData> record;

    RecordSet();
};