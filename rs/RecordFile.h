#pragma once

#include "../fs/File.h"
#include "RecordFileMeta.h"
#include "RecordPageHeader.h"

#define VAC_LEVEL_IDX 8

/**
 * 用于使用页编号和槽编号在记录文件中唯一确定一条记录。
 */
struct RID {
    std::size_t page_id{};
    std::size_t slot_id{};
};

/**
 * 在 File 类的基础上进一步封装，提供了一些读写记录相关的方法。
 * <br/>
 * 在更上层的模块中，一个 RecordFile 对象将会对应一张表。
 */
class RecordFile {
    File *file;

    RecordFileMeta meta{};

    /**
     * 从数据页中获取指定记录的字节偏移量。
     * @param page     数据页
     * @param slot_id  槽编号
     * @return         由槽编号指定的记录的字节偏移量。若为无效记录，则返回 0。
     */
    static u_int16_t &get_slot_offset(BufferPage *page, std::size_t slot_id);

    /**
     * 在该文件中分配一个新的记录槽。
     * @param record_size  待分配槽的记录的实际长度
     * @return             记录的唯一标识。
     */
    RID alloc_vacancy(std::size_t record_size);

    /**
     * 在数据页中查询指定槽的空闲空间大小。
     * <br/>
     * 若当前槽为有效槽，返回的是假设所对应的记录被删除后，该槽的最大空闲空间大小。
     * <br/>
     * 若当前槽为无效槽，则该方法从语义上约定，前一个槽必为有效槽，此时返回的就是当前槽的最大空闲空间大小。
     * <br/>
     * 若当前槽为无效槽，且前一个槽也为无效槽，则返回 0。
     * @param page     数据页
     * @param slot_id  槽编号
     * @return         一个 (空闲空间大小, 空闲空间偏移量) 的数据对，空闲空间大小的含义如上所述。
     */
    std::pair<std::size_t, u_int16_t> vacancy_at(BufferPage *page, std::size_t slot_id) const;

    /**
     * 用于在插入或删除记录后，维护页头以及文件元信息页的空闲空间相关信息。
     * @param page     数据页
     */
    void fix_page_header(BufferPage *page);

public:

    explicit RecordFile(File *file);

    /**
     * 获取记录。
     * @param rid     记录唯一标识
     * @param record  调用者用于存放记录数据的缓冲区
     * @return        记录实际长度。
     */
    std::size_t get_record(const RID &rid, char *record);

    /**
     * 插入记录。
     * @param record_size  记录实际长度
     * @param record       记录数据
     * @return             记录唯一标识。
     */
    RID insert_record(std::size_t record_size, const char *record);

    /**
     * 删除记录。
     * @param rid  记录唯一标识
     */
    void delete_record(const RID &rid);

    /**
     * 更新记录。
     * @param rid          记录唯一标识
     * @param record_size  待插入记录的实际长度
     * @param record       待插入记录的数据
     * @return             新插入记录的唯一标识
     */
    RID update_record(const RID &rid, std::size_t record_size, const char *record);

    /**
     * 关闭文件。
     */
    void close();
};