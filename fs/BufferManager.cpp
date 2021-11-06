#include "BufferManager.h"

BufferPage *BufferManager::alloc_buffer(std::size_t file_id, std::size_t page_id) {
    if (pages.size() == BUFFER_MAX) {
        auto it = --pages.end();
        auto buffer_page = *it;
        buffer_map.erase(buffer_map.find(std::make_pair(buffer_page->file_id, buffer_page->page_id)));
        pages.erase(it);
        buffer_page->fresh = true;
        buffer_page->fresh_file_id = file_id;
        buffer_page->fresh_page_id = page_id;
        return buffer_page;
    } else {
        return new BufferPage(file_id, page_id);
    }
}

BufferPage *BufferManager::get_buffer(std::size_t file_id, std::size_t page_id, bool bypass_search) {
    BufferPage *buffer_page;
    if (bypass_search) {
        buffer_page = alloc_buffer(file_id, page_id);
    } else {
        auto map_result = buffer_map.find(std::make_pair(file_id, page_id));
        if (map_result == buffer_map.end()) {
            buffer_page = alloc_buffer(file_id, page_id);
        } else {
            auto it = map_result->second;
            buffer_page = *it;
            pages.erase(it);
        }
    }
    pages.insert(pages.begin(), buffer_page);
    buffer_map[std::make_pair(file_id, page_id)] = pages.begin();
    return buffer_page;
}

BufferPage *BufferManager::release_buffer(std::size_t file_id, std::size_t page_id) {
    auto map_result = buffer_map.find(std::make_pair(file_id, page_id));
    if (map_result != buffer_map.end()) {
        auto it = map_result->second;
        auto buffer_page = *it;
        buffer_map.erase(map_result);
        pages.erase(it);
        return buffer_page;
    } else {
        return nullptr;
    }
}
