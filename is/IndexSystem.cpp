#include <cstring>
#include "IndexSystem.h"

IndexSystem::IndexSystem(): btree_m(255){
}

IndexSystem::IndexSystem(int m): btree_m(m){
}

void IndexSystem::create_file(const char *filename) {
    //  创建文件，并维护元信息页
    IndexFileMeta meta{
            /* btree_m: */    btree_m,
            /* node_num: */  0,
            /* root_page: */  1,
    };
    Filesystem::create_file(filename);
    auto file = fs.open_file(filename);
    auto page = file->get_page(0, true);
    memcpy(page->data, &meta, sizeof meta);
    page->dirty = true;

    //  创建根数据页
    auto index_file = new IndexFile(file);
    index_file->create_node();

    index_file->close();
}

void IndexSystem::remove_file(const char *filename) {
    Filesystem::remove_file(filename);
}

IndexFile *IndexSystem::open_file(const char *filename) {
    auto file = fs.open_file(filename);
    return new IndexFile(file);
}
