#pragma once

#include "../fs/File.h"
#include "../rs/RecordFile.h"
#include "IndexFileMeta.h"
#include "IndexPageHeader.h"

#include <iostream>

/**
 * 用于使用页编号和关键码编号在索引文件中唯一确定一条索引。
 */
struct IID {
    std::size_t page_id{};
    u_int8_t key_id{};
};

/**
 * 进行查找操作时得到的类似迭代器的封装类
 */
class IndexScan {
    File *file;

    int upper_bound;  //  键值上界

    IID now_iid;  //  当前索引

public:

    IndexScan() = default;

    IndexScan(File *file, int upper_bound, IID iid);

    IID RID_iid;
    
    /**
     * 获取记录。
     * @param rid     获取到的记录唯一标识
     * @return        是否获取成功，true表示成功
     */
    bool get_next_entry(RID &rid);
};

/**
 * 在 File 类的基础上进一步封装，提供了一些读写索引相关的方法。
 * <br/>
 * 在更上层的模块中，一个 IndexFile 对象将会对应关于一张表的一个整型列所创建的索引。
 */
class IndexFile {
    File *file;

    IndexFileMeta meta{};

    u_int32_t get_key(BufferPage *page, u_int8_t key_id);

    void set_key(BufferPage *page, u_int8_t key_id, u_int32_t key);

    u_int32_t get_value(BufferPage *page, u_int8_t key_id);

    void get_rid_value(BufferPage *page, u_int8_t key_id, RID &rid);

    void set_value(BufferPage *page, u_int8_t key_id, u_int32_t value);

    void set_rid_value(BufferPage *page, u_int8_t key_id, RID rid);

    u_int8_t get_pre_key_id(BufferPage *page, u_int8_t key_id);

    void set_pre_key_id(BufferPage *page, u_int8_t key_id, u_int8_t pre_key_id);

    u_int8_t get_suc_key_id(BufferPage *page, u_int8_t key_id);

    void set_suc_key_id(BufferPage *page, u_int8_t key_id, u_int8_t suc_key_id);

    IID location_at(int key);

    bool insert_key_leaf(std::size_t page_id, u_int8_t suc_key_id, u_int32_t key, const RID &rid);

    void insert_key(std::size_t page_id, u_int8_t suc_key_id, u_int32_t key, u_int32_t value);

    bool insert_key_with_lrvalue(std::size_t page_id, u_int32_t key, u_int32_t lvalue, u_int32_t rvalue);

    std::size_t overflow(std::size_t page_id, std::size_t &new_page_id, u_int32_t &over_key);

    void delete_key(std::size_t page_id, u_int8_t key_id);

    bool underflow(std::size_t page_id, u_int8_t key_id, u_int8_t &under_key_id, std::size_t &merge_page_id);

    bool delete_range_key(std::size_t page_id, int lower_bound, int upper_bound);
    
    bool delete_key_leaf(std::size_t page_id, std::size_t key_id);

public:

    explicit IndexFile(File *file);

    /**
     * 创建一个新的B树节点，默认为叶子，并添加正无穷关键码，返回页编号
     * @return      页编号
     */
    std::size_t create_node(int is_leaf = 1, int parent = 0, int pre_leaf = 0, int suc_leaf = 0);

    /**
     * 查找某个范围内的记录，结果通过迭代器访问。注意如果修改了索引文件，迭代器会失效，调用者应在此之前使用完
     * @param lower_bound     索引值的下界
     * @param upper_bound     索引值的上界
     * @param index_scan      结果
     */
    void search(int lower_bound, int upper_bound, IndexScan &index_scan);

    /**
     * 往索引文件里插入一条记录，需保证之前不存在对应键值的索引记录
     * @param key     记录的键值
     * @param rid     记录的唯一标识
     */
    void insert_record(int key, const RID &rid);

    void delete_record(int key, const RID &rid);

    /**
     * 从索引文件里删除一个区间的索引记录
     * @param lower_bound     索引值的下界
     * @param upper_bound     索引值的上界
     */
    void delete_record_range(int lower_bound, int upper_bound);

    void update_record(int old_key, const RID &old_rid, int new_key, const RID &new_rid);

    void close();
};