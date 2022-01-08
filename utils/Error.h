#pragma once

namespace Error {
    enum InsertError {
        NONE,
        VALUE_COUNT_MISMATCH,
        TYPE_MISMATCH,
        STR_TOO_LONG,
        FIELD_CANNOT_BE_NULL,
        UNIQUE_RESTRICTION_FAIL,
        PRIMARY_RESTRICTION_FAIL,
        INSERT_FOREIGN_RESTRICTION_FAIL,
        TABLE_DOES_NOT_EXIST,
    };
}