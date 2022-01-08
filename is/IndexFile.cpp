#include <cstring>

#include "IndexFile.h"

IndexScan::IndexScan(File *_file, int _upper_bound, IID _iid): file(_file), upper_bound(_upper_bound), now_iid(_iid){}

bool IndexScan::get_next_entry(RID &rid){
    if (now_iid.page_id == 0) return false;
    auto page = file->get_page(now_iid.page_id);
    int key = ((u_int32_t *) page->data)[(INDEX_PAGE_HEADER_SIZE + now_iid.key_id * 8) >> 2];
    if (key > upper_bound) return false;
    memcpy(&rid, page->data + INDEX_PAGE_HEADER_SIZE + now_iid.key_id * 8 + 4, 4);
    RID_iid = now_iid;  //  rid 对应的 iid
    now_iid.key_id = page->data[PAGE_SIZE - now_iid.key_id * 2 - 2 + 1];
    if (now_iid.key_id == 0){
        auto header = (IndexPageHeader *) page->data;
        now_iid.page_id = header->suc_leaf;
        if (now_iid.page_id != 0){
            auto new_page = file->get_page(now_iid.page_id);
            auto new_header = (IndexPageHeader *) new_page->data;
            now_iid.key_id = new_header->first_key_id;
        }
    }
    return true;
}

u_int32_t IndexFile::get_key(BufferPage *page, u_int8_t key_id){
    return ((u_int32_t *) page->data)[(INDEX_PAGE_HEADER_SIZE + key_id * 8) >> 2];
}

void IndexFile::set_key(BufferPage *page, u_int8_t key_id, u_int32_t key){
    /* memcpy(page->data + INDEX_PAGE_HEADER_SIZE + key_id * 8, &key, 4); */
    ((u_int32_t *) page->data)[(INDEX_PAGE_HEADER_SIZE + key_id * 8) >> 2] = key;
    page->dirty = true;
}

u_int32_t IndexFile::get_value(BufferPage *page, u_int8_t key_id){
    /* memcpy(&value, page->data + INDEX_PAGE_HEADER_SIZE + key_id * 8 + 4, 4); */
    return ((u_int32_t *) page->data)[(INDEX_PAGE_HEADER_SIZE + key_id * 8 + 4) >> 2];
}

void IndexFile::get_rid_value(BufferPage *page, u_int8_t key_id, RID &rid){
    memcpy(&rid, page->data + INDEX_PAGE_HEADER_SIZE + key_id * 8 + 4, 4);
}

void IndexFile::set_value(BufferPage *page, u_int8_t key_id, u_int32_t value){
    /* memcpy(page->data + INDEX_PAGE_HEADER_SIZE + key_id * 8 + 4, &value, 4); */
    ((u_int32_t *) page->data)[(INDEX_PAGE_HEADER_SIZE + key_id * 8 + 4) >> 2] = value;
    page->dirty = true;
}

void IndexFile::set_rid_value(BufferPage *page, u_int8_t key_id, RID rid){
    memcpy(page->data + INDEX_PAGE_HEADER_SIZE + key_id * 8 + 4, &rid, 4);
    page->dirty = true;
}

u_int8_t IndexFile::get_pre_key_id(BufferPage *page, u_int8_t key_id){
    /* memcpy(page->data + PAGE_SIZE - key_id * 2 - 2, &pre_key_id, 1); */
    return page->data[PAGE_SIZE - key_id * 2 - 2];
}

void IndexFile::set_pre_key_id(BufferPage *page, u_int8_t key_id, u_int8_t pre_key_id){
    /* memcpy(&pre_key_id, page->data + PAGE_SIZE - key_id * 2 - 2, 1); */
    page->data[PAGE_SIZE - key_id * 2 - 2] = pre_key_id;
    page->dirty = true;
}

u_int8_t IndexFile::get_suc_key_id(BufferPage *page, u_int8_t key_id){
    return page->data[PAGE_SIZE - key_id * 2 - 2 + 1];
}

void IndexFile::set_suc_key_id(BufferPage *page, u_int8_t key_id, u_int8_t suc_key_id){
    page->data[PAGE_SIZE - key_id * 2 - 2 + 1] = suc_key_id;
    page->dirty = true;
}

/**
* 定位大于等于key的索引位置
* @param key    索引值
* @return       索引位置，用唯一标识表示
*/
IID IndexFile::location_at(int key){
    //  从根节点开始循环向下
    auto meta_page = file->get_page(0);
    auto meta_data = (IndexFileMeta *) meta_page->data;
    std::size_t root_page_id = meta_data->root_page;
    std::size_t now_page_id = root_page_id;
    while (true) {
        assert(now_page_id <= meta_data->node_num);
        /* printf("location at, now_page_id is %d\n", now_page_id); */
        auto page = file->get_page(now_page_id);
        auto header = (IndexPageHeader *) page->data;
        if (header->is_leaf){  //  直接开始遍历key对应的位置
            u_int8_t now_key_id = header->first_key_id;  //  从开头开始
            /* printf("location at, now_key_id is %d\n", now_key_id); */
            while (get_key(page, now_key_id) < key){
                now_key_id = get_suc_key_id(page, now_key_id);
                /* printf("location at, now_key_id is %d\n", now_key_id); */
            }
            return {now_page_id, now_key_id};
        }
        else {  //  开始遍历应该往哪个儿子走
            u_int8_t now_key_id = header->first_key_id;  //  从开头开始
            /* printf("location at, now_key_id is %d\n", now_key_id); */
            while (get_key(page, now_key_id) < key){
                now_key_id = get_suc_key_id(page, now_key_id);
                /* printf("location at, now_key_id is %d\n", now_key_id); */
            }
            now_page_id = get_value(page, now_key_id);
        }
    }
}

/**
* 往一个叶子结点插入新的 key-value 键值对
* @param page_id        叶子结点的页号
* @param suc_key_id     新的索引插在编号为 suc_key_id 的关键码前面
* @param key            新的关键码值
* @param rid            对应的 rid 值
* @return               插入后是否需要上溢，true表示需要
*/
bool IndexFile::insert_key_leaf(std::size_t page_id, u_int8_t suc_key_id, u_int32_t key, const RID &rid){
    /* printf("insert key leaf %d %d %d\n", page_id, suc_key_id, key); */
    auto page = file->get_page(page_id);
    //  更新页头数据
    auto header = (IndexPageHeader *) page->data;
    header->key_num += 1;
    page->dirty = true;
    u_int8_t new_key_id = header->key_num;
    //  写入新索引记录
    set_key(page, new_key_id, key);
    set_rid_value(page, new_key_id, rid);
    //  更新链表
    set_suc_key_id(page, new_key_id, suc_key_id);
    u_int8_t pre_key_id = get_pre_key_id(page, suc_key_id);
    if (pre_key_id != 0){
        set_pre_key_id(page, new_key_id, pre_key_id);
        set_suc_key_id(page, pre_key_id, new_key_id);
    }
    else{
        header->first_key_id = new_key_id;
        set_pre_key_id(page, new_key_id, 0);
    }
    set_pre_key_id(page, suc_key_id, new_key_id);
    return header->key_num > meta.btree_m - 1;
}

/**
* 往一个非叶子结点插入新的 key-value 键值对
* @param page_id        非叶子结点的页号
* @param suc_key_id     新的索引插在编号为 suc_key_id 的关键码前面
* @param key            新的关键码值
* @param value          对应的 value 值
*/
void IndexFile::insert_key(std::size_t page_id, u_int8_t suc_key_id, u_int32_t key, u_int32_t value){
    auto page = file->get_page(page_id);
    //  更新页头数据
    auto header = (IndexPageHeader *) page->data;
    header->key_num += 1;
    page->dirty = true;
    u_int8_t new_key_id = header->key_num;
    //  写入新索引记录
    set_key(page, new_key_id, key);
    set_value(page, new_key_id, value);
    //  更新链表
    set_suc_key_id(page, new_key_id, suc_key_id);
    u_int8_t pre_key_id = get_pre_key_id(page, suc_key_id);
    if (pre_key_id != 0){
        set_pre_key_id(page, new_key_id, pre_key_id);
        set_suc_key_id(page, pre_key_id, new_key_id);
    }
    else{
        header->first_key_id = new_key_id;
        set_pre_key_id(page, new_key_id, 0);
    }
    set_pre_key_id(page, suc_key_id, new_key_id);
}

/**
* 往一个非叶子结点插入新的 key-lvalue 键值对 ，并将其后一个关键码的 value 更新为 rvalue
* @param page_id        叶子结点的页号
* @param key            新的关键码值
* @param lvalue         对应的左子树 value 值
* @param rvalue         对应的右子树 value 值
* @return               插入后是否需要上溢，true表示需要
*/
bool IndexFile::insert_key_with_lrvalue(std::size_t page_id, u_int32_t key, u_int32_t lvalue, u_int32_t rvalue){
    auto page = file->get_page(page_id);
    auto header = (IndexPageHeader *) page->data;
    auto now_key_id = header->first_key_id;
    while (key > get_key(page, now_key_id))
        now_key_id = get_suc_key_id(page, now_key_id);
    auto suc_key_id = now_key_id;
    insert_key(page_id, suc_key_id, key, lvalue);
    //  右子树等于下一个关键码的左子树
    set_value(page, suc_key_id, rvalue);
    return header->key_num > meta.btree_m - 1;
}

/**
* 对一个结点处理上溢，分裂出新的结点，并将一个key往父亲处插入。需要保证确实需要上溢
* @param page_id        结点的页号
* @param new_page_id    新分裂出的页号
* @param over_key       往父亲处插入的key值
* @return               该结点的父亲页编号
*/
std::size_t IndexFile::overflow(std::size_t page_id, std::size_t &new_page_id, u_int32_t &over_key){
    auto page = file->get_page(page_id);
    auto header = (IndexPageHeader *) page->data;
    bool is_leaf = header->is_leaf;
    auto now_key_id = header->first_key_id;
    //  找到第 m / 2 上取整个关键码，从其开始分裂，并将其上溢
    int cnt = 0;
    while (cnt < (meta.btree_m + 1) / 2 - 1){
        cnt ++;
        now_key_id = get_suc_key_id(page, now_key_id);
    }
    auto mid_key_id = now_key_id;
    over_key = get_key(page, mid_key_id);
    //  将 mid_key_id 后的内容写入新页,如果是非叶子把编号为 0 处的值赋值给新页编号为 0 处
    new_page_id = create_node(is_leaf, header->parent);
    now_key_id = get_suc_key_id(page, mid_key_id);
    while (true){
        auto now_key = get_key(page, now_key_id);
        if (is_leaf){
            if (now_key_id == 0) break;
            RID now_rid;
            get_rid_value(page, now_key_id, now_rid);
            //  插入新页
            insert_key_leaf(new_page_id, 0, now_key, now_rid);
        }
        else{
            auto now_value = get_value(page, now_key_id);
            //  插入新页
            if (now_key_id != 0){
                insert_key(new_page_id, 0, now_key, now_value);
            }
            else{
                auto new_page = file->get_page(new_page_id);
                set_value(new_page, 0, now_value);
                break;
            }
        }
        now_key_id = get_suc_key_id(page, now_key_id);
    }
    //  将 mid_key_id 之前的内容写入新页,如果是叶子节点的上溢，mid_key_id也需要写入
    auto new_page_id2 = create_node(is_leaf, header->parent, header->pre_leaf, header->suc_leaf);
    now_key_id = header->first_key_id;
    while (true){
        if (!is_leaf && now_key_id == mid_key_id) break;
        auto now_key = get_key(page, now_key_id);
        if (is_leaf){
            RID now_rid;
            get_rid_value(page, now_key_id, now_rid);
            //  插入新页
            insert_key_leaf(new_page_id2, 0, now_key, now_rid);
        }
        else{
            auto now_value = get_value(page, now_key_id);
            //  插入新页
            insert_key(new_page_id2, 0, now_key, now_value);
        }
        if (now_key_id == mid_key_id) break;
        now_key_id = get_suc_key_id(page, now_key_id);
    }
    auto new_page2 = file->get_page(new_page_id2);
    //  非叶子节点上溢，还要把 mid_key_id 的左子树保留下来
    if (!is_leaf){
        auto mid_value = get_value(page, mid_key_id);
        set_value(new_page2, 0, mid_value);
    }
    //  将第二个新页的内容倒入原页中
    memcpy(page->data, new_page2->data, PAGE_SIZE);
    //  删去第二个新页
    auto meta_page = file->get_page(0);
    auto meta_data = (IndexFileMeta *) meta_page->data;
    meta_data->node_num -= 1;
    meta.node_num -= 1;

    header = (IndexPageHeader *) page->data;
    //  如果从叶子节点分裂成两个叶子，再维护下页头里的叶子链表
    if (is_leaf){
        auto new_page = file->get_page(new_page_id);
        auto new_header = (IndexPageHeader *) new_page->data;
        new_header->suc_leaf = header->suc_leaf;
        new_header->pre_leaf = page_id;
        header->suc_leaf = new_page_id;
        page->dirty = true;
        new_page->dirty = true;
    }
    return header->parent;
}

/**
* 删除一个结点的一个关键码
* @param page_id        结点的页号
* @param key_id         需要删除的关键码编号
*/
void IndexFile::delete_key(std::size_t page_id, u_int8_t key_id) {
    auto page = file->get_page(page_id);
    auto header = (IndexPageHeader *) page->data;
    //  从链表中删除
    auto pre_key_id = get_pre_key_id(page, key_id);
    auto suc_key_id = get_suc_key_id(page, key_id);
    if (pre_key_id != 0) set_suc_key_id(page, pre_key_id, suc_key_id);
    else header->first_key_id = suc_key_id;  //  更新链表头
    set_pre_key_id(page, suc_key_id, pre_key_id);
    //  将最大编号的关键码更改编号为 key_id
    auto max_key_id = header->key_num;
    set_key(page, key_id, get_key(page, max_key_id));
    set_value(page, key_id, get_value(page, max_key_id));
    set_pre_key_id(page, key_id, get_pre_key_id(page, max_key_id));
    set_suc_key_id(page, key_id, get_suc_key_id(page, max_key_id));
    pre_key_id = get_pre_key_id(page, key_id);
    suc_key_id = get_suc_key_id(page, key_id);
    if (pre_key_id != 0) set_suc_key_id(page, pre_key_id, key_id);
    else header->first_key_id = key_id;  //  更新链表头
    set_pre_key_id(page, suc_key_id, key_id);
    //  更新页头数据
    header->key_num -= 1;
    page->dirty = true;
}

/**
* 处理一个结点的下溢，如果是叶子，整个结点被删，如果是非叶子，key_id 被删，返回是否需要进一步下溢
* @param page_id        结点的页号
* @param key_id         如果是非叶子，删去的关键码编号
* @param under_key_id   如果进一步下溢，父亲应该删去的关键码编号
* @param merge_page_id  如果发生合并，合并后的页号
* @return               是否需要进一步下溢，true表示需要
*/
bool IndexFile::underflow(std::size_t page_id, u_int8_t key_id, u_int8_t &under_key_id, std::size_t &merge_page_id){
    auto page = file->get_page(page_id);
    auto header = (IndexPageHeader *) page->data;
    bool is_leaf = header->is_leaf;
    u_int32_t key;
    if (!is_leaf) key = get_key(page, key_id);
    else key = get_key(page, get_pre_key_id(page, 0));  //  叶子用最大键值
    //  定位其属于父亲哪一个关键码的左子树
    auto parent_page = file->get_page(header->parent);
    auto parent_header = (IndexPageHeader *) parent_page->data;
    auto parent_key_id = parent_header->first_key_id;
    while (key > get_key(parent_page, parent_key_id)) parent_key_id = get_suc_key_id(parent_page, parent_key_id);
    under_key_id = parent_key_id;
    if (is_leaf){  //  删去整个叶子
        //  尝试过接右兄弟
        if (parent_key_id != 0){
            auto right_bro_page_id = get_value(parent_page, get_suc_key_id(parent_page, parent_key_id));
            auto right_bro_page = file->get_page(right_bro_page_id);
            auto right_bro_header = (IndexPageHeader *) right_bro_page->data;
            if (right_bro_header->key_num > 1){  //  可以过接
                //  将 key-value 拿到
                auto min_key_id = right_bro_header->first_key_id;
                auto min_key = get_key(right_bro_page, min_key_id);
                auto min_value = get_value(right_bro_page, min_key_id);
                //  将原页变成关键码为1
                header->key_num = 1;
                header->first_key_id = 1;
                set_key(page, 1, min_key);
                set_value(page, 1, min_value);
                set_pre_key_id(page, 1, 0);
                set_suc_key_id(page, 1, 0);
                set_pre_key_id(page, 0, 1);
                //  将兄弟页删去对应关键码
                delete_key(right_bro_page_id, min_key_id);
                //  更改父亲处的关键码值
                set_key(parent_page, parent_key_id, min_key);
                return false;
            }
        }
        //  合并，继续下溢
        //  先更改叶子链表
        auto pre_leaf = header->pre_leaf;
        auto suc_leaf = header->suc_leaf;
        if (pre_leaf != 0){
            auto pre_leaf_page = file->get_page(pre_leaf);
            auto pre_leaf_header = (IndexPageHeader *) pre_leaf_page->data;
            pre_leaf_header->suc_leaf = suc_leaf;
            pre_leaf_page->dirty = true;
        }
        if (suc_leaf != 0){
            auto suc_leaf_page = file->get_page(suc_leaf);
            auto suc_leaf_header = (IndexPageHeader *) suc_leaf_page->data;
            suc_leaf_header->pre_leaf = pre_leaf;
            suc_leaf_page->dirty = true;
        }
        merge_page_id = suc_leaf;
        if (parent_key_id == 0){  //  和左兄弟合并
            merge_page_id = pre_leaf;
            auto pre_parent_key_id = get_pre_key_id(parent_page, 0);
            auto pre_value = get_value(parent_page, pre_parent_key_id);
            set_value(parent_page, 0, pre_value);
            under_key_id = pre_parent_key_id;
        }
        return true;
    }
    else{
        //  先将 key_id 删掉
        delete_key(page_id, key_id);
        header = (IndexPageHeader *) page->data;
        if (header->key_num >= (meta.btree_m + 1) / 2 - 1) return false;
        //  尝试过接右兄弟,若不行则与右兄弟合并
        if (parent_key_id != 0){
            auto bro_page_id = get_value(parent_page, get_suc_key_id(parent_page, parent_key_id));
            merge_page_id = page_id;
            auto bro_page = file->get_page(bro_page_id);
            auto bro_header = (IndexPageHeader *) bro_page->data;
            auto x = get_key(parent_page, parent_key_id);
            if (bro_header->key_num - 1 >= (meta.btree_m + 1) / 2 - 1){  //  可以过接
                //  将 key-value 拿到
                auto min_key_id = bro_header->first_key_id;
                auto min_key = get_key(bro_page, min_key_id);
                auto min_value = get_value(bro_page, min_key_id);
                //  往原页最后插入
                auto max_value = get_value(page, 0);
                insert_key(page_id, 0, x, max_value);
                set_value(page, 0, min_value);
                //  将兄弟页删去对应关键码
                delete_key(bro_page_id, min_key_id);
                //  更改父亲处的关键码值
                set_key(parent_page, parent_key_id, min_key);
                return false;
            }
            else{  //  合并
                auto max_value = get_value(page, 0);
                insert_key(page_id, 0, x, max_value);
                auto bro_key_id = bro_header->first_key_id;
                while (bro_key_id != 0){
                    auto bro_key = get_key(bro_page, bro_key_id);
                    auto bro_value = get_value(bro_page, bro_key_id);
                    insert_key(page_id, 0, bro_key, bro_value);
                    bro_key_id = get_suc_key_id(bro_page, bro_key_id);
                }
                set_value(page, 0, get_value(bro_page, 0));
                set_value(parent_page, get_suc_key_id(parent_page, parent_key_id), page_id);
                return true;  //  需要删掉 parent_key_id
            }
        }
        else{  //  尝试过接左兄弟,若不行则与左兄弟合并
            std::size_t bro_page_id = get_value(parent_page, get_pre_key_id(parent_page, parent_key_id));
            merge_page_id = bro_page_id;
            auto bro_page = file->get_page(bro_page_id);
            auto bro_header = (IndexPageHeader *) bro_page->data;
            auto x = get_key(parent_page, parent_key_id);
            if (bro_header->key_num - 1 >= (meta.btree_m + 1) / 2 - 1){  //  可以过接
                //  将 key-value 拿到
                auto max_key_id = get_pre_key_id(bro_page, 0);
                auto max_key = get_key(bro_page, max_key_id);
                auto max_value = get_value(bro_page, max_key_id);
                //  往原页最前插入
                auto max_0_value = get_value(bro_page, 0);
                insert_key(page_id, header->first_key_id, x, max_0_value);
                //  将兄弟页删去对应关键码
                set_value(bro_page, 0, max_value);
                delete_key(bro_page_id, max_key_id);
                //  更改父亲处的关键码值
                set_key(parent_page, parent_key_id, max_key);
                return false;
            }
            else{  //  合并
                //  看成右兄弟合并
                parent_key_id = get_pre_key_id(parent_page, parent_key_id);
                under_key_id = parent_key_id;
                std::swap(page_id, bro_page_id);
                std::swap(page, bro_page);
                std::swap(header, bro_header);
                //  执行右兄弟合并
                auto max_value = get_value(page, 0);
                insert_key(page_id, 0, x, max_value);
                auto bro_key_id = bro_header->first_key_id;
                while (bro_key_id != 0){
                    auto bro_key = get_key(bro_page, bro_key_id);
                    auto bro_value = get_value(bro_page, bro_key_id);
                    insert_key(page_id, 0, bro_key, bro_value);
                    bro_key_id = get_suc_key_id(bro_page, bro_key_id);
                }
                set_value(page, 0, get_value(bro_page, 0));
                set_value(parent_page, get_suc_key_id(parent_page, parent_key_id), page_id);
                return true;  //  需要删掉 parent_key_id
            }
        }
    }
}

/**
* 删除一个叶子结点处于一个区间内的关键码，如果整个叶子被删则循环处理下溢，返回是否将最大值删去
* @param page_id        叶子结点的页号
* @param lower_bound    键值下界
* @param upper_bound    键值上界
* @return               最大关键码是否也被删去
*/
bool IndexFile::delete_range_key(std::size_t page_id, int lower_bound, int upper_bound){
    auto page = file->get_page(page_id);
    auto header = (IndexPageHeader *) page->data;
    //  查询最小值和最大值
    auto min_key = get_key(page, header->first_key_id);
    auto max_key = get_key(page, get_pre_key_id(page, 0));
    if (min_key > upper_bound || max_key < lower_bound) return false;
    if (min_key >= lower_bound && max_key <= upper_bound){  //  整个叶子都删掉
        auto now_page_id = page_id;
        u_int8_t key_id;
        while (true){
            u_int8_t new_key_id;
            auto now_page = file->get_page(now_page_id);
            auto now_header = (IndexPageHeader *) now_page->data;
            auto parent_page_id = now_header->parent;
            std::size_t merge_page_id;
            if (!underflow(now_page_id, key_id, new_key_id, merge_page_id)) break;
            //  继续考虑下溢，注意特判下溢到根节点的情况
            key_id = new_key_id;
            if (parent_page_id == meta.root_page){
                auto root_page = file->get_page(parent_page_id);
                auto root_header = (IndexPageHeader *) root_page->data;
                if (root_header->key_num > 1) delete_key(parent_page_id, key_id);
                else{  //  删掉根节点
                    //  更新元信息页与meta
                    auto meta_page = file->get_page(0);
                    auto meta_data = (IndexFileMeta *) meta_page->data;
                    meta_data->root_page = merge_page_id;
                    meta_page->dirty = true;
                    meta.root_page = merge_page_id;
                    //  更新 now_page_id 页的 parent
                    now_header->parent = 0;
                    now_page->dirty = true;
                }
                break;
            }
            now_page_id = parent_page_id;
        }
        return true;
    }
    auto now_key_id = header->first_key_id;
    while (now_key_id != 0){
        auto next_key_id = get_suc_key_id(page, now_key_id);
        auto now_key = get_key(page, now_key_id);
        if (now_key >= lower_bound && now_key <= upper_bound){
            if (next_key_id == header->key_num) next_key_id = now_key_id;  //  删去 now 后会把最大编号直接改编号到 now 上
            delete_key(page_id, now_key_id);
        }
        now_key_id = next_key_id;
    }
    return max_key >= lower_bound && max_key <= upper_bound;
}

bool IndexFile::delete_key_leaf(std::size_t page_id, std::size_t key_id){
    auto page = file->get_page(page_id);
    auto header = (IndexPageHeader *) page->data;
    if (header->key_num == 1){  //  整个叶子都删掉
        auto now_page_id = page_id;
        u_int8_t key_id;
        while (true){
            u_int8_t new_key_id;
            auto now_page = file->get_page(now_page_id);
            auto now_header = (IndexPageHeader *) now_page->data;
            auto parent_page_id = now_header->parent;
            std::size_t merge_page_id;
            if (!underflow(now_page_id, key_id, new_key_id, merge_page_id)) break;
            //  继续考虑下溢，注意特判下溢到根节点的情况
            key_id = new_key_id;
            if (parent_page_id == meta.root_page){
                auto root_page = file->get_page(parent_page_id);
                auto root_header = (IndexPageHeader *) root_page->data;
                if (root_header->key_num > 1) delete_key(parent_page_id, key_id);
                else{  //  删掉根节点
                    //  更新元信息页与meta
                    auto meta_page = file->get_page(0);
                    auto meta_data = (IndexFileMeta *) meta_page->data;
                    meta_data->root_page = merge_page_id;
                    meta_page->dirty = true;
                    meta.root_page = merge_page_id;
                    //  更新 now_page_id 页的 parent
                    now_header->parent = 0;
                    now_page->dirty = true;
                }
                break;
            }
            now_page_id = parent_page_id;
        }
        return true;
    }
    auto now_key_id = key_id;
    auto now_key = get_key(page, now_key_id);
    delete_key(page_id, now_key_id);
    return false;
}

IndexFile::IndexFile(File *file) : file(file) {
    auto page = file->get_page(0);
    memcpy(&meta, page->data, sizeof meta);
    assert(meta.root_page > 0);
}

std::size_t IndexFile::create_node(int is_leaf, int parent, int pre_leaf, int suc_leaf){
    //  更新元信息页与meta
    auto meta_page = file->get_page(0);
    auto meta_data = (IndexFileMeta *) meta_page->data;
    meta_data->node_num += 1;
    meta_page->dirty = true;
    meta.node_num += 1;
    //  维护新页页头以及初始值
    auto page = file->get_page(meta_data->node_num, true);
    auto header = (IndexPageHeader *) page->data;
    header->is_leaf = is_leaf;
    header->key_num = 0;
    header->first_key_id = 0;
    header->parent = parent;
    header->pre_leaf = pre_leaf;
    header->suc_leaf = suc_leaf;
    page->dirty = true;
    int inf = INDEX_KEY_MAX;
    set_key(page, 0, inf);
    set_pre_key_id(page, 0, 0);
    set_suc_key_id(page, 0, 0);
    return meta.node_num;
}

void IndexFile::search(int lower_bound, int upper_bound, IndexScan &index_scan){
    /* printf("search %d %d\n", lower_bound, upper_bound); */
    IID iid = location_at(lower_bound);
    /* printf("IID is {%d %d}\n", iid.page_id, iid.key_id); */
    index_scan = IndexScan(file, upper_bound, iid);
}

void IndexFile::insert_record(int key, const RID &rid){
    /* printf("Insert record %d\n", key); */
    IID iid = location_at(key);
    /* printf("IID is {%d , %d}\n", iid.page_id, iid.key_id);
    printf("RID is {%d , %d}\n", rid.page_id, rid.slot_id); */
    //  插入并检测是否需要上溢
    if (insert_key_leaf(iid.page_id, iid.key_id, key, rid)){  //  需要上溢
        std::size_t now_page_id;
        std::size_t new_page_id;
        u_int32_t over_key;
        std::size_t parent_page_id = iid.page_id;  //  为了边界情况下可以正常循环
        do{
            now_page_id = parent_page_id;
            parent_page_id = overflow(now_page_id, new_page_id, over_key);  //  将该节点上溢
            if (now_page_id == meta.root_page){
                //  新建一个页作根节点
                parent_page_id = create_node(0);
                //  更新元信息页与meta
                auto meta_page = file->get_page(0);
                auto meta_data = (IndexFileMeta *) meta_page->data;
                meta_data->root_page = parent_page_id;
                meta_page->dirty = true;
                meta.root_page = parent_page_id;
                //  更新两个节点的 parent
                auto now_page = file->get_page(now_page_id);
                auto now_header = (IndexPageHeader *) now_page->data;
                now_header->parent = meta.root_page;
                now_page->dirty = true;
                auto new_page = file->get_page(new_page_id);
                auto new_header = (IndexPageHeader *) new_page->data;
                new_header->parent = meta.root_page;
                new_page->dirty = true;
            }
            /* printf("overflow %d %d\n", parent_page_id, over_key); */
        }while (insert_key_with_lrvalue(parent_page_id, over_key, now_page_id, new_page_id));
    }
}

void IndexFile::delete_record(int key, const RID &old_rid){
    IndexScan index_scan;
    search(key, key, index_scan);
    RID rid;
    while (index_scan.get_next_entry(rid)){
        if (rid.page_id == old_rid.page_id && rid.slot_id == old_rid.slot_id){
            delete_key_leaf(index_scan.RID_iid.page_id, index_scan.RID_iid.key_id);
            break;
        }
    }
}

void IndexFile::delete_record_range(int lower_bound, int upper_bound){
    IID iid = location_at(lower_bound);
    auto now_leaf_id = iid.page_id;
    while (true){
        auto now_page = file->get_page(now_leaf_id);
        auto now_header = (IndexPageHeader *) now_page->data;
        auto next_leaf_id = now_header->suc_leaf;
        if (!delete_range_key(now_leaf_id, lower_bound, upper_bound)) break;
        now_leaf_id = next_leaf_id;
    }
}

void IndexFile::update_record(int old_key, const RID &old_rid, int new_key, const RID &new_rid){
    delete_record(old_key, old_rid);
    insert_record(new_key, new_rid);
}

void IndexFile::close() {
    if (file) file->close();
    file = nullptr;
}