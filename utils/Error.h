#pragma once

namespace Error {
    enum InsertError {
        NONE,
        VALUE_COUNT_MISMATCH,
        TYPE_MISMATCH,
        STR_TOO_LONG,
        FIELD_CANNOT_BE_NULL,
        PRIMARY_RESTRICTION_FAIL,
    };
}