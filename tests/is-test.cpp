#include <cstring>
#include <algorithm>
#include "../is/IndexSystem.h"

#define FILENAME_0 ("bin/is-test-0.txt")
#define FILENAME_1 ("bin/is-test-1.txt")

void simple_test(){
    {
        // 创建文件
        IndexSystem is = IndexSystem();
        is.create_file(FILENAME_0);
        auto file = is.open_file(FILENAME_0);

        RID rid;

        // 写入第一条索引
        rid = RID{1, 1};
        file->insert_record(1, rid);
        
        //  查找该条索引
        IndexScan index_scan;
        file->search(0, 2, index_scan);
        assert(index_scan.get_next_entry(rid) == true);
        assert(rid.page_id == 1);
        assert(rid.slot_id == 1);
        assert(index_scan.get_next_entry(rid) == false);

        //  关闭并删除文件
        file->close();
        Filesystem::remove_file(FILENAME_0);
    }
}

void multiple_data_test() {
    {
        // 创建文件
        IndexSystem is = IndexSystem(5);  //  用一个比较小的m
        is.create_file(FILENAME_1);
        auto file = is.open_file(FILENAME_1);

        RID rid;

        //  插入 1000 条索引，乱序插入
        int N = 1000;
        int id[N + 10];
        for(int i = 1; i <= N; i ++) id[i] = i;
        std::random_shuffle(id + 1, id + N + 1);
        for(int i = 1; i <= N; i ++){
            rid = RID{(u_int16_t)id[i], (u_int16_t)(id[i] * 2)};
            //  写入
            file->insert_record(id[i], rid);
        }

        /* printf("Insert finished\n"); */

        //  查找 [300, 500]
        int L = 300;
        int R = 500;
        IndexScan index_scan;
        file->search(L, R, index_scan);
        for(int i = L; i <= R; i ++){
            assert(index_scan.get_next_entry(rid) == true);
            assert(rid.page_id == i);
            assert(rid.slot_id == i * 2);
        }
        assert(index_scan.get_next_entry(rid) == false);

        /* printf("Insert then Search is right\n"); */

        //  删除 [340, 460]
        int d_L = 340;
        int d_R = 460;
        file->delete_record_range(L, R);

        //  再查找 [300, 500]
        file->search(L, R, index_scan);
        for(int i = L; i < d_L; i ++){
            assert(index_scan.get_next_entry(rid) == true);
            assert(rid.page_id == i);
            assert(rid.slot_id == i * 2);
        }
        for(int i = d_R + 1; i <= R; i ++){
            assert(index_scan.get_next_entry(rid) == true);
            assert(rid.page_id == i);
            assert(rid.slot_id == i * 2);
        }
        assert(index_scan.get_next_entry(rid) == false);

        /* printf("Delete then Search is right\n"); */

        //  更新 320 这条记录，变成 350
        int old_key = 320;
        int new_key = 350;
        file->update_record(old_key, RID{(u_int16_t)old_key, (u_int16_t)(old_key * 2)}, 
                            new_key, RID{(u_int16_t)new_key, (u_int16_t)(new_key * 2)});
        file->search(old_key, old_key, index_scan);
        assert(index_scan.get_next_entry(rid) == false);
        file->search(new_key, new_key, index_scan);
        assert(index_scan.get_next_entry(rid) == true);
        assert(rid.page_id == new_key);
        assert(rid.slot_id == new_key * 2);

        /* printf("Update then Search is right\n"); */

        //  关闭并删除文件
        file->close();
        Filesystem::remove_file(FILENAME_1);
    }
}

int main(){
    simple_test();
    multiple_data_test();
    return 0;
}