#pragma once

#include "Column.h"

struct Selector {
public:
    enum Type {
        COL,
        AGR_COL,
        CT
    } type;

    enum Aggregator_Type {
        COUNT,
        AVERAGE,
        MAX,
        MIN,
        SUM
    } agr_type;

    Column col;  //  COL 和 AGR_COL 
};