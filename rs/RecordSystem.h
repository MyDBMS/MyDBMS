#pragma once

#include "../fs/Filesystem.h"
#include "RecordFile.h"

/**
 * 是整个记录管理系统的入口类，包含一个文件系统 Filesystem 对象。
 * <br/>
 * 所有记录文件都必须通过该类创建，因为通过这种方式创建的文件会包含一些记录文件的元信息。
 */
class RecordSystem {
    Filesystem fs;

public:
    RecordSystem() = default;

    /**
     * 创建一个记录文件，并维护该文件的元信息。
     *
     * @param filename    待创建文件的文件名
     * @param fixed_size  该记录文件的记录定长部分长度
     * @param var_cnt     该记录文件的记录可变列个数
     */
    void create_file(const char *filename, std::size_t fixed_size, std::size_t var_cnt);

    /**
     * 删除一个记录文件。等价于从文件系统中直接删除文件。
     *
     * @param filename  待删除文件的文件名
     */
    static void remove_file(const char *filename);

    /**
     * 打开一个记录文件，创建一个 RecordFile 对象，并返回它的指针。
     * <br/>
     * 调用者需自行保证该文件是一个记录文件。
     *
     * @param filename  待打开文件的文件名
     * @return          对应打开后的文件的 RecordFile 对象。
     */
    RecordFile *open_file(const char *filename);

    ~RecordSystem() = default;
};