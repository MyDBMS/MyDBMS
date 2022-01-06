#pragma once

/**
 * 用于描述索引文件的元信息。
 * <br/>
 * 相关字段的含义参见 <code>README.md</code> 中的相关说明。
 */
struct IndexFileMeta {
    std::size_t btree_m{};    // B+ 树的参数 m , 意味着一个非叶节点存储的关键码数量在 m/2 上取整 - 1 到 m - 1 之间
    std::size_t node_num{};    // 当前节点个数
    std::size_t root_page{};    // 当前根节点是哪一页
};