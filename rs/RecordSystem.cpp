#include <cstring>
#include "RecordSystem.h"

void RecordSystem::create_file(const char *filename, std::size_t fixed_size, std::size_t var_cnt) {
    RecordFileMeta meta{
            /* page_cnt: */    1,
            /* fixed_size: */  fixed_size,
            /* var_cnt: */     var_cnt,
    };
    Filesystem::create_file(filename);
    auto file = fs.open_file(filename);
    auto page = file->get_page(0, true);
    memcpy(page->data, &meta, sizeof meta);
    page->dirty = true;
    file->close();
}

void RecordSystem::remove_file(const char *filename) {
    Filesystem::remove_file(filename);
}

RecordFile *RecordSystem::open_file(const char *filename) {
    auto file = fs.open_file(filename);
    return new RecordFile(file);
}
