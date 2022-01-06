#pragma once

#include "../fs/Filesystem.h"
#include "IndexFile.h"

/**
 * 是整个索引管理系统的入口类，包含一个文件系统 Filesystem 对象。
 * <br/>
 * 所有索引文件都必须通过该类创建，因为通过这种方式创建的文件会包含一些索引文件的元信息。
 */
class IndexSystem {
    Filesystem fs;

    int btree_m;

public:
    IndexSystem();

    IndexSystem(int m);

    /**
     * 创建一个索引文件，并维护该文件的元信息。
     *
     * @param filename    待创建文件的文件名
     */
    void create_file(const char *filename);

    /**
     * 删除一个索引文件。等价于从文件系统中直接删除文件。
     *
     * @param filename  待删除文件的文件名
     */
    static void remove_file(const char *filename);

    /**
     * 打开一个索引文件，创建一个 IndexFile 对象，并返回它的指针。
     * <br/>
     * 调用者需自行保证该文件是一个索引文件。
     *
     * @param filename  待打开文件的文件名
     * @return          对应打开后的文件的 IndexFile 对象。
     */
    IndexFile *open_file(const char *filename);

    ~IndexSystem() = default;
};