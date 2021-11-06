#pragma once

#include <cassert>
#include <cstdio>
#include <set>

#include "BufferManager.h"

/**
 * 封装了文件读写相关系统函数的类。
 */
class File {
    friend class Filesystem;

    FILE *fd{};

    /**
     * 文件编号，由 Filesystem 分配。
     */
    std::size_t index{};

    /**
     * 缓存管理器，由 Filesystem 提供。
     */
    BufferManager *bm;

    /**
     * 维护用过的页号，关闭文件时写回脏页有用。
     */
    std::set<std::size_t> used_pages;

    explicit File(BufferManager *bm);

    /**
     * 在磁盘中创建一个文件。若文件名已存在，则无事发生。
     * @param filename  待创建文件的文件名
     */
    static void create(const char *filename);

    /**
     * 从磁盘中删除一个文件。
     * @param filename  待删除文件的文件名
     */
    static void rm(const char *filename);

    /**
     * 从磁盘打开一个文件。
     * @param filename  待打开文件的文件名
     * @param id        Filesystem 分配的文件编号
     */
    void open(const char *filename, std::size_t id);

    /**
     * 内部方法，用于将文件读入缓存页。
     * @param buffer_page  待读入的缓存页
     */
    void read_page(BufferPage *buffer_page);

    /**
     * 内部方法，用于将缓存页写回文件。
     * @param buffer_page  待写回的缓存页
     */
    void write_page(BufferPage *buffer_page);

public:

    /**
     * 获取一个缓存页。
     * <br/>
     * 如果需要对获取到的缓存页进行写操作，用户需自行将 BufferPage::dirty 字段改为 <code>true</code>；当这片缓存被交换或文件被关闭时，脏缓存页的内容会被写回。
     * <br/>
     * 一旦通过该方法获得了一个缓存页的指针，调用者应尽快使用，以防该页因为被交换出缓存队列而失效。
     * @param page_id        页编号
     * @param bypass_search  是否跳过缓存页的查找；只有当调用者确信指定页不在缓存中时才应将该参数置为 <code>true</code>
     * @return               获取到的缓存页。
     */
    BufferPage *get_page(std::size_t page_id, bool bypass_search = false);

    /**
     * 关闭文件。
     * <br/>
     * 如果缓存中有该文件的脏页，则将脏页写回磁盘。
     */
    void close();
};