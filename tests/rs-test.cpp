#include <cstring>
#include "../rs/RecordSystem.h"

#define FILENAME_0 ("bin/rs-test-0.txt")
#define TEST_FIXED_SIZE 11
#define TEST_FULL_SIZE 19
#define TEST_FULL_SIZE_LONG 21
#define TEST_VAR_CNT 1

void meta_check() {
    assert(sizeof(RecordFileMeta) <= PAGE_SIZE);
    assert(sizeof(RecordPageHeader) <= RECORD_PAGE_HEADER_SIZE);
}

void simple_test() {
    const char record_in[TEST_FULL_SIZE + 5] = {
            0, 'A', 'B', 'C', 'D', '0', '1', '2', '3', 19, 0, 'D', 'a', 't', 'a', 'b', 'a', 's', 'e',
    };

    const char record_update_short[TEST_FULL_SIZE + 5] = {
            0, 'A', 'B', 'C', 'D', '0', '1', '2', '3', 19, 0, 'T', 's', 'i', 'n', 'g', 'h', 'u', 'a',
    };

    const char record_update_long[TEST_FULL_SIZE_LONG + 5] = {
            0, 'A', 'B', 'C', 'D', '0', '1', '2', '3', 21, 0, 'T', 's', 'i', 'n', 'g', 'h', 'u', 'a', 'D', 'B',
    };

    {
        // 创建文件
        RecordSystem rs = RecordSystem();
        rs.create_file(FILENAME_0, TEST_FIXED_SIZE, TEST_VAR_CNT);
        auto file = rs.open_file(FILENAME_0);

        // 写入第一条记录
        RID rid = file->insert_record(TEST_FULL_SIZE, record_in);
        assert(rid.page_id == 2);
        assert(rid.slot_id == 0);

        // 测试读写正确性
        char record_out[TEST_FULL_SIZE + 5];
        std::size_t size_out = file->get_record(rid, record_out);
        assert(size_out == TEST_FULL_SIZE);
        assert(memcmp(record_in, record_out, TEST_FULL_SIZE) == 0);

        // 写入第二条记录
        RID rid_tmp = file->insert_record(TEST_FULL_SIZE, record_in);
        assert(rid_tmp.page_id == 2);
        assert(rid_tmp.slot_id == 1);

        // 更新第一条记录
        rid = file->update_record(rid, TEST_FULL_SIZE, record_update_short);
        assert(rid.page_id == 2);
        assert(rid.slot_id == 0);

        // 测试更新正确性
        size_out = file->get_record(rid, record_out);
        assert(size_out == TEST_FULL_SIZE);
        assert(memcmp(record_update_short, record_out, TEST_FULL_SIZE) == 0);

        // 写入第三条记录
        rid_tmp = file->insert_record(TEST_FULL_SIZE, record_in);
        assert(rid_tmp.page_id == 2);
        assert(rid_tmp.slot_id == 2);

        // 将第一条记录更新为更长的一条记录
        rid = file->update_record(rid, TEST_FULL_SIZE_LONG, record_update_long);
        assert(rid.page_id == 2);
        assert(rid.slot_id == 3);

        // 测试更新正确性
        char record_out_long[TEST_FULL_SIZE_LONG + 5];
        size_out = file->get_record(rid, record_out_long);
        assert(size_out == TEST_FULL_SIZE_LONG);
        assert(memcmp(record_update_long, record_out_long, TEST_FULL_SIZE_LONG) == 0);

        // 关闭文件
        file->close();
    }

    {
        // 从文件系统层面对结果进行验证
        Filesystem fs = Filesystem();
        auto file = fs.open_file(FILENAME_0);

        // 验证元信息页
        auto meta_page = file->get_page(0);
        auto meta_data = (RecordFileMeta *) meta_page->data;
        assert(meta_data->page_cnt == 3);
        assert(meta_data->fixed_size == TEST_FIXED_SIZE);
        assert(meta_data->var_cnt == TEST_VAR_CNT);
        assert(meta_data->fsp_cnt == 1);
        assert(meta_data->fsp_data[0] == 0xFF);

        // 验证空闲空间信息页
        auto fsp = file->get_page(1);
        for (std::size_t i = 1; i < PAGE_SIZE; ++i) {
            assert(fsp->data[i] == (i == 4096 ? 0xFB : 0xFF));
        }

        // 验证数据页
        auto data_page = file->get_page(2);
        auto header = (RecordPageHeader *) data_page->data;
        assert(header->slot_cnt == 4);
        assert(header->free_offset == RECORD_PAGE_HEADER_SIZE + 3 * TEST_FULL_SIZE + TEST_FULL_SIZE_LONG);
        assert(header->free_size == PAGE_SIZE - header->free_offset - 5 * 2);
        assert(header->free_slot_id == 4);

        auto short_data = (u_int16_t *) data_page->data;
        char data_buffer[TEST_FULL_SIZE_LONG + 5];

        assert(short_data[PAGE_SIZE / 2 - 1] == 0);

        assert(short_data[PAGE_SIZE / 2 - 2] == RECORD_PAGE_HEADER_SIZE + TEST_FULL_SIZE);
        memcpy(data_buffer, data_page->data + RECORD_PAGE_HEADER_SIZE + TEST_FULL_SIZE, TEST_FULL_SIZE);
        assert(memcmp(data_buffer, record_in, TEST_FULL_SIZE) == 0);

        assert(short_data[PAGE_SIZE / 2 - 3] == RECORD_PAGE_HEADER_SIZE + TEST_FULL_SIZE * 2);
        memcpy(data_buffer, data_page->data + RECORD_PAGE_HEADER_SIZE + TEST_FULL_SIZE * 2, TEST_FULL_SIZE);
        assert(memcmp(data_buffer, record_in, TEST_FULL_SIZE) == 0);

        assert(short_data[PAGE_SIZE / 2 - 4] == RECORD_PAGE_HEADER_SIZE + TEST_FULL_SIZE * 3);
        memcpy(data_buffer, data_page->data + RECORD_PAGE_HEADER_SIZE + TEST_FULL_SIZE * 3, TEST_FULL_SIZE_LONG);
        assert(memcmp(data_buffer, record_update_long, TEST_FULL_SIZE_LONG) == 0);

        // 关闭并删除文件
        file->close();
        Filesystem::remove_file(FILENAME_0);
    }
}

void complex_test() {

}

int main() {
    meta_check();
    simple_test();
    complex_test();
    return 0;
}
