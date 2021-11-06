#include "BufferPage.h"

BufferPage::BufferPage(std::size_t file_id, std::size_t page_id)
        : file_id(file_id), page_id(page_id), dirty(false), data(new unsigned int[(PAGE_SIZE >> 2)]),
          fresh(true), fresh_file_id(file_id), fresh_page_id(page_id) {}

BufferPage::~BufferPage() {
    delete[] data;
}
