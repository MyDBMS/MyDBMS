#pragma once

#include <cstdlib>

#define INDEX_PAGE_HEADER_SIZE 64

#define INDEX_KEY_MAX 100000000

/**
 * 用于描述索引文件中数据页的页头。
 */
struct IndexPageHeader {
    std::size_t is_leaf{};    // 是否是叶子节点
    std::size_t key_num{};    // 当前总共含有多少有效关键码（即不包括编号为0的关键吗），等于关键码编号的最大值
    u_int8_t first_key_id{};    //  最小的关键码的编号
    std::size_t parent{};    // 父亲节点的页号，为 0 代表是根节点
    std::size_t pre_leaf{};    // 如果是叶子节点，维护叶子链表的前一项，即前一个叶子的页号，为 0 代表其已经是最前一个叶子
    std::size_t suc_leaf{};    // 如果是叶子节点，维护叶子链表的后一项，即后一个叶子的页号，为 0 代表其已经是最后一个叶子
};