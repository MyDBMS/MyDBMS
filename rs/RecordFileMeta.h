#pragma once

#define MAX_FSP 63

/**
 * 用于描述记录文件的元信息。
 * <br/>
 * <code>fsp</code> 相关字段的含义参见 <code>README.md</code> 中空闲空间信息维护部分的相关说明。
 */
struct RecordFileMeta {
    std::size_t page_cnt{};      // 页总数
    std::size_t fixed_size{};    // 记录定长部分长度，注意需要包含可变列偏移数组
    std::size_t var_cnt{};       // 记录可变列个数
    u_char fsp_cnt{};            // 空闲空间信息页个数
    u_char fsp_data[MAX_FSP]{};  // 空闲空间信息页数值
};