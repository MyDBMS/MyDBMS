#include <cstring>
#include "File.h"

File::File(BufferManager *bm) : bm(bm) {
}

void File::create(const char *filename) {
    auto file = fopen(filename, "a+");
    assert(file != nullptr);
    fclose(file);
}

void File::rm(const char *filename) {
    int result = remove(filename);
    assert(result == 0);
}

void File::open(const char *filename, std::size_t id) {
    fd = fopen(filename, "rb+");
    assert(fd != nullptr);
    index = id;
}

void File::read_page(BufferPage *buffer_page) {
    size_t offset = (buffer_page->page_id << PAGE_SIZE_IDX);
    size_t error = fseek(fd, (long) offset, SEEK_SET);
    assert(error == 0);
    fread(buffer_page->data, 1, PAGE_SIZE, fd);
}

void File::write_page(BufferPage *buffer_page) {
    size_t offset = (buffer_page->page_id << PAGE_SIZE_IDX);
    size_t error = fseek(fd, (long) offset, SEEK_SET);
    assert(error == 0);
    fwrite(buffer_page->data, 1, PAGE_SIZE, fd);
}

BufferPage *File::get_page(std::size_t page_id, bool bypass_search) {
    used_pages.insert(page_id);
    auto buffer_page = bm->get_buffer(index, page_id, bypass_search);
    if (buffer_page->fresh) {
        // buffer_page 为 fresh 表明这是一片新分配出的缓存页
        if (buffer_page->dirty) {
            // buffer_page 为 dirty 表明这是一片交换得到的缓存页
            // 旧的内容必须写回磁盘
            write_page(buffer_page);
        }

        // 需要将文件编号和页编号置为新分配的
        buffer_page->file_id = buffer_page->fresh_file_id;
        buffer_page->page_id = buffer_page->fresh_page_id;
        read_page(buffer_page);

        // 重置标记位
        buffer_page->dirty = false;
        buffer_page->fresh = false;
    }
    return buffer_page;
}

void File::close() {
    if (fd) {
        for (std::size_t page_id: used_pages) {
            if (auto buffer_page = bm->release_buffer(index, page_id)) {
                if (buffer_page->dirty) {
                    // 有尚未写回的脏页
                    write_page(buffer_page);
                }
                delete buffer_page;
            }
        }
        fclose(fd);
    }
    fd = nullptr;
}