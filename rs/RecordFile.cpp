#include <cstring>

#include "RecordFile.h"

u_int16_t &RecordFile::get_slot_offset(BufferPage *page, std::size_t slot_id) {
    return ((u_int16_t *) page->data)[(PAGE_SIZE >> 1) - 1 - slot_id];
}

RID RecordFile::alloc_vacancy(std::size_t record_size) {
    assert(record_size < PAGE_SIZE);
    assert(record_size > 0);
    u_char record_size_level =
            (record_size + (1 << (PAGE_SIZE_IDX - VAC_LEVEL_IDX)) - 1) >> (PAGE_SIZE_IDX - VAC_LEVEL_IDX);
    for (u_char fsp_id = 0; fsp_id < meta.fsp_cnt; ++fsp_id) {
        if (meta.fsp_data[fsp_id] >= record_size_level) {
            // 在 fsp 中找到有足够空闲空间的页
            auto fsp = file->get_page(1 + fsp_id * (PAGE_SIZE / 2 + 1));
            std::size_t cursor = 1;
            assert(fsp->data[cursor] == meta.fsp_data[fsp_id]);
            while (cursor < PAGE_SIZE / 2) {
                cursor = fsp->data[cursor * 2] >= record_size_level ? cursor * 2 : cursor * 2 + 1;
            }
            assert(fsp->data[cursor] >= record_size_level);

            // 分配空闲槽
            std::size_t page_id = 2 + fsp_id * (PAGE_SIZE / 2 + 1) + (cursor - PAGE_SIZE / 2);
            auto page = file->get_page(page_id);
            auto header = (RecordPageHeader *) page->data;
            if (fsp->data[cursor] == 0xFF) {
                // 表示这是一个新页
                header->slot_cnt = 1;
                header->free_size = PAGE_SIZE - RECORD_PAGE_HEADER_SIZE - 2;
                header->free_offset = RECORD_PAGE_HEADER_SIZE;
                header->free_slot_id = 0;
            } else {
                assert(header->free_size >> (PAGE_SIZE_IDX - VAC_LEVEL_IDX) == fsp->data[cursor]);
                if (header->free_slot_id >= header->slot_cnt) {
                    header->slot_cnt = header->free_slot_id + 1;
                }
            }
            get_slot_offset(page, header->free_slot_id) = header->free_offset;
            page->dirty = true;
            return {page_id, header->free_slot_id};
        }
    }

    // 需要创建新的 fsp 以及新的数据页
    auto meta_page = file->get_page(0);
    auto meta_data = (RecordFileMeta *) meta_page->data;
    meta_data->page_cnt += 2;
    meta_data->fsp_data[meta_data->fsp_cnt++] = 0xFF;
    meta_page->dirty = true;
    u_char fsp_id = meta.fsp_cnt;
    meta.page_cnt += 2;
    meta.fsp_data[meta.fsp_cnt++] = 0xFF;
    auto fsp = file->get_page(1 + fsp_id * (PAGE_SIZE / 2 + 1));
    memset(fsp->data, 0xFF, PAGE_SIZE);
    fsp->dirty = true;
    std::size_t page_id = 2 + fsp_id * (PAGE_SIZE / 2 + 1);
    auto page = file->get_page(page_id);

    // 维护新创建的数据页的页头
    auto header = (RecordPageHeader *) page->data;
    header->slot_cnt = 1;
    header->free_size = PAGE_SIZE - RECORD_PAGE_HEADER_SIZE - 2;
    header->free_offset = RECORD_PAGE_HEADER_SIZE;
    header->free_slot_id = 0;

    // 分配空闲槽
    get_slot_offset(page, header->free_slot_id) = header->free_offset;
    page->dirty = true;
    return {page_id, header->free_slot_id};
}

std::pair<std::size_t, u_int16_t> RecordFile::vacancy_at(BufferPage *page, std::size_t slot_id) const {
    auto header = (RecordPageHeader *) page->data;
    if (header->slot_cnt == 0) {
        // 该页为空，全都可分配
        return std::make_pair(PAGE_SIZE - RECORD_PAGE_HEADER_SIZE - 2, RECORD_PAGE_HEADER_SIZE);
    } else if (slot_id == header->slot_cnt) {
        // 找到最后一条记录的末尾，计算剩余部分长度
        u_int16_t last_slot_offset = get_slot_offset(page, slot_id - 1);
        assert(last_slot_offset >= RECORD_PAGE_HEADER_SIZE);
        u_int16_t last_record_size = *((u_int16_t *) (page->data + last_slot_offset + meta.fixed_size - 2));
        u_int16_t slot_offset = last_slot_offset + last_record_size;
        assert(slot_offset >= RECORD_PAGE_HEADER_SIZE);
        return std::make_pair(PAGE_SIZE - slot_offset - header->slot_cnt * 2 - 2, slot_offset);
    } else {
        // 尝试获取当前记录的字符偏移
        u_int16_t slot_offset = get_slot_offset(page, slot_id);
        if (slot_offset == 0) {
            // 说明当前记录本身无效，从前一条记录的偏移量入手
            if (slot_id > 0) {
                slot_offset = get_slot_offset(page, slot_id - 1);
                if (slot_offset == 0) {
                    // 说明前一条记录也无效，根据语义，应当返回 0
                    return std::make_pair(0, 0);
                }
                // 前一条记录有效，根据语义，需要将 slot_offset 调整为前一条记录的末尾地址
                u_int16_t last_record_size = *((u_int16_t *) (page->data + slot_offset + meta.fixed_size - 2));
                slot_offset += last_record_size;
            } else {
                // 当前记录已经在最开始
                slot_offset = RECORD_PAGE_HEADER_SIZE;
            }
        }
        assert(slot_offset >= RECORD_PAGE_HEADER_SIZE);
        u_int16_t next_slot_offset;
        std::size_t next = slot_id + 1;
        while (next < header->slot_cnt && (next_slot_offset = get_slot_offset(page, next)) == 0) {
            ++next;
        }
        if (next == header->slot_cnt) {
            return std::make_pair(PAGE_SIZE - slot_offset - header->slot_cnt * 2, slot_offset);
        } else {
            return std::make_pair(next_slot_offset - slot_offset, slot_offset);
        }
    }
}

void RecordFile::fix_page_header(BufferPage *page) {
    auto header = (RecordPageHeader *) page->data;
    header->free_size = 0;
    // 找到最大空闲空间
    // 遍历式查找，效率较低
    for (std::size_t i = 0; i <= header->slot_cnt; ++i) {
        if (i == header->slot_cnt || get_slot_offset(page, i) == 0) {
            std::pair<std::size_t, u_int16_t> vacancy_info;
            if (std::get<0>((vacancy_info = vacancy_at(page, i))) > header->free_size) {
                header->free_size = std::get<0>(vacancy_info);
                header->free_offset = std::get<1>(vacancy_info);
                header->free_slot_id = i;
            }
        }
    }

    // 更新 fsp
    // 当前分级策略可能导致一定的空间浪费
    u_char free_space_level = header->free_size >> (PAGE_SIZE_IDX - VAC_LEVEL_IDX);
    std::size_t fsp_id = (page->page_id - 2) / (PAGE_SIZE / 2 + 1);
    std::size_t page_id_in_group = (page->page_id - 2) % (PAGE_SIZE / 2 + 1);
    assert(page_id_in_group < PAGE_SIZE / 2);
    auto fsp = file->get_page(fsp_id * (PAGE_SIZE / 2 + 1) + 1);
    auto cursor = page_id_in_group + PAGE_SIZE / 2;
    if (fsp->data[cursor] != free_space_level) {
        fsp->dirty = true;
        fsp->data[cursor] = free_space_level;

        // 向根迭代
        while (cursor > 1) {
            auto parent_val = std::max(fsp->data[cursor], fsp->data[cursor ^ 1]);
            if (parent_val == fsp->data[cursor >> 1]) break;
            cursor >>= 1;
            fsp->data[cursor] = parent_val;
        }

        // 若更新到根，还需更新元信息页
        if (cursor == 1) {
            auto meta_page = file->get_page(0);
            auto meta_data = (RecordFileMeta *) meta_page->data;
            meta_data->fsp_data[fsp_id] = fsp->data[cursor];
            meta_page->dirty = true;
            meta.fsp_data[fsp_id] = fsp->data[cursor];
        }
    }
}

RecordFile::RecordFile(File *file) : file(file) {
    auto page = file->get_page(0);
    memcpy(&meta, page->data, sizeof meta);
    assert(meta.page_cnt > 0);
}

std::size_t RecordFile::get_record(const RID &rid, char *record) {
    auto page = file->get_page(rid.page_id);
    u_int16_t slot_offset = get_slot_offset(page, rid.slot_id);
    assert(slot_offset != 0);

    u_int16_t record_size = *((u_int16_t *) (page->data + slot_offset + meta.fixed_size - 2));
    memcpy(record, page->data + slot_offset, record_size);
    return record_size;
}

RID RecordFile::insert_record(std::size_t record_size, const char *record) {
    // 寻找空闲空间
    RID rid = alloc_vacancy(record_size);

    // 写入记录
    auto page = file->get_page(rid.page_id);
    page->dirty = true;
    u_int16_t slot_offset = get_slot_offset(page, rid.slot_id);
    memcpy(page->data + slot_offset, record, record_size);
    assert(*((u_int16_t *) (page->data + slot_offset + meta.fixed_size - 2)) == record_size);

    // 维护页头部空闲空间信息
    fix_page_header(page);
    return rid;
}

void RecordFile::delete_record(const RID &rid) {
    // 删除记录
    auto page = file->get_page(rid.page_id);
    page->dirty = true;
    get_slot_offset(page, rid.slot_id) = 0;

    // 更新槽数目
    auto header = (RecordPageHeader *) page->data;
    while (header->slot_cnt > 0 && get_slot_offset(page, header->slot_cnt - 1) == 0) {
        --header->slot_cnt;
    }

    // 维护页头部空闲空间信息
    fix_page_header(page);
}

RID RecordFile::update_record(const RID &rid, std::size_t record_size, const char *record) {
    auto page = file->get_page(rid.page_id);
    page->dirty = true;
    std::pair<std::size_t, u_int16_t> vacancy_info;
    if (record_size <= std::get<0>(vacancy_info = vacancy_at(page, rid.slot_id))) {
        u_int16_t slot_offset = std::get<1>(vacancy_info);
        memcpy(page->data + slot_offset, record, record_size);
        assert(*((u_int16_t *) (page->data + slot_offset + meta.fixed_size - 2)) == record_size);
        return rid;
    } else {
        delete_record(rid);
        return insert_record(record_size, record);
    }
}

RID RecordFile::find_from(const RID &rid) const {
    for (std::size_t curr_page = rid.page_id; curr_page < meta.page_cnt; ++curr_page) {
        if (curr_page < 2) continue;
        if (curr_page % (PAGE_SIZE / 2 + 1) == 1) continue;

        auto page = file->get_page(curr_page);
        auto header = (RecordPageHeader *) page->data;

        for (std::size_t curr_slot = (curr_page == rid.page_id ? rid.slot_id + 1 : 0);
             curr_slot < header->slot_cnt; ++curr_slot) {
            if (get_slot_offset(page, curr_slot) != 0) {
                return {curr_page, curr_slot};
            }
        }
    }

    return {0, 0};
}

RID RecordFile::find_first() const {
    return find_from({0, 0});
}

RID RecordFile::find_next(const RID &rid) const {
    return find_from(rid);
}

void RecordFile::close() {
    if (file) file->close();
    file = nullptr;
}
