#pragma once

#include <cstdlib>

#define BUFFER_MAX 60000

#define PAGE_SIZE 8192

#define PAGE_SIZE_IDX 13

/**
 * 用于描述缓存页。
 * <br/>
 * 若要对缓存页进行写操作，用户需自行将 BufferPage::dirty 字段改为 <code>true</code>；当这片缓存被交换或文件被关闭时，脏缓存页的内容会被写回。
 */
struct BufferPage {
    std::size_t file_id;
    std::size_t page_id;
    bool dirty;
    u_char *data;

private:
    friend class BufferManager;

    friend class File;

    BufferPage(std::size_t file_id, std::size_t page_id);

    bool fresh;
    std::size_t fresh_file_id;
    std::size_t fresh_page_id;

    ~BufferPage();
};