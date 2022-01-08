#pragma once

#include "../ms/Value.h"
#include "Column.h"

#include <vector>

typedef std::vector<Value> RecordData;

struct RecordSet {
public:
    std::vector<Column> columns;
    std::vector<RecordData> record;

    RecordSet();
};