#pragma once

#include <cstdlib>

#define RECORD_PAGE_HEADER_SIZE 64

/**
 * 用于描述记录文件中数据页的页头。
 */
struct RecordPageHeader {
    std::size_t slot_cnt{};      // 已分配的槽总数，包括有效槽和无效槽；本系统要求最后一个槽必须有效
    std::size_t free_offset{};   // 空闲空间的字节偏移量
    std::size_t free_size{};     // 空闲空间字节数目
    std::size_t free_slot_id{};  // 空闲空间槽号；可能与 slot_cnt 相等，此时插入新纪录需要分配新的槽
};