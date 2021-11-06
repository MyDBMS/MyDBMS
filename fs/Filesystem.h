#pragma once

#include <vector>

#include "File.h"
#include "BufferManager.h"

/**
 * 整个页式文件管理系统的入口类，负责管理文件系统中用到的所有 File 对象，并维护了一个缓存管理器 BufferManager。
 * <br/>
 * 所有 File 共享一个 BufferManager 的引用。
 */
class Filesystem {
    std::vector<File *> files;

    BufferManager buffer_manager{};

public:
    Filesystem() = default;

    /**
     * 在磁盘中创建一个文件。若文件名已存在，则无事发生。
     * @param filename  待创建文件的文件名
     */
    static void create_file(const char *filename);

    /**
     * 从磁盘中删除一个文件。
     * @param filename  待删除文件的文件名
     */
    static void remove_file(const char *filename);

    /**
     * 打开一个文件，创建一个 File 对象，并返回它的指针。
     * @param filename  待打开文件的文件名
     * @return          对应打开后的文件的 File 对象。
     */
    File *open_file(const char *filename);

    ~Filesystem();
};
