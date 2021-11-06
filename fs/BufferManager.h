#pragma once

#include <list>
#include <map>
#include "BufferPage.h"

typedef typeof(std::list<BufferPage *>().begin()) BufferPageIterator;

/**
 * 基于 LRU 算法的缓存管理系统，用于对页进行缓存管理。
 * <br/>
 * 每当文件需要读取一个页时，它会首先尝试从该对象中寻找缓存页，若未命中则再进行文件读写。
 */
class BufferManager {
    friend class Filesystem;

    friend class File;

    /**
     * 缓存队列，最大长度为 #BUFFER_MAX。
     */
    std::list<BufferPage *> pages;

    /**
     * 考虑维护从 <code>(file_id, page_id)</code> 到 <code>BufferPage *</code> 的缓存查找系统。
     * <br/>
     * 我们需要一种数据结构，既能维护缓存队列，又能快速从 <code>(file_id, page_id)</code> 查找到指定缓存页在队列中的位置，并进行相应的队列操作。
     * <br/>
     * 为此，我们将 std::list 与 std::map 组合使用，其中 std::map 的值类型为一个 std::list 的迭代器。
     */
    std::map<std::pair<int, int>, BufferPageIterator> buffer_map;

    BufferManager() = default;

    /**
     * 内部方法，用于分配缓存页。
     * <br/>
     * 若缓存队列已满，则取出队尾的缓存页，添加到队首作为分配出的缓存页，File 负责处理可能的写回操作。
     * <br/>
     * 若缓存队列未满，则分配一块新的缓存页。
     * <br/>
     * 无论哪种情况，文件的读入均需由 File 负责。BufferManager 只负责缓存页的查找与分配。
     * @param file_id  文件编号
     * @param page_id  页编号
     * @return         分配的缓存页
     */
    BufferPage *alloc_buffer(std::size_t file_id, std::size_t page_id);

    /**
     * 尝试寻找缓存页。若未命中，返回一个新分配的缓存页，调用者需自行向该缓存页中写入数据，并处理可能的写回。
     * @param file_id        文件编号
     * @param page_id        页编号
     * @param bypass_search  若为真，则表明调用者保证该页不在缓存中，将跳过缓存页查找。
     * @return               若找到缓存页，则返回；否则返回一个新分配的缓存页。
     */
    BufferPage *get_buffer(std::size_t file_id, std::size_t page_id, bool bypass_search = false);

    /**
     * 从缓存队列中释放缓存页，并返回所释放的页。File 应负责所释放缓存页的写回。
     * <br/>
     * 若指定的缓存页不存在，则返回空指针。
     * @param file_id  文件编号
     * @param page_id  页编号
     * @return         若缓存页存在，则返回；否则返回空指针。
     */
    BufferPage *release_buffer(std::size_t file_id, std::size_t page_id);
};
