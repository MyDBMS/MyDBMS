#include "../fs/Filesystem.h"

#define FILENAME_0 ("bin/fs-test-0.txt")
#define FILENAME_1 ("bin/fs-test-1.txt")
#define FILENAME_2 ("bin/fs-test-2.txt")

#define PAGE_MAX (BUFFER_MAX * 2)

void test_write() {
    Filesystem fs = Filesystem();
    Filesystem::create_file(FILENAME_0);
    Filesystem::create_file(FILENAME_1);
    File *file_0 = fs.open_file(FILENAME_0);
    File *file_1 = fs.open_file(FILENAME_1);
    for (int page_id = 0; page_id < PAGE_MAX; ++page_id) {
        BufferPage *page_0 = file_0->get_page(page_id);
        page_0->data[0] = page_id;
        page_0->dirty = true;
        BufferPage *page_1 = file_1->get_page(page_id);
        page_1->data[PAGE_SIZE / 4 - 1] = PAGE_MAX - page_id;
        page_1->dirty = true;
    }
}

void test_read() {
    Filesystem fs = Filesystem();
    File *file_0 = fs.open_file(FILENAME_0);
    File *file_1 = fs.open_file(FILENAME_1);
    for (int page_id = 0; page_id < PAGE_MAX; ++page_id) {
        BufferPage *page_0 = file_0->get_page(page_id);
        assert(page_0->data[0] == page_id);
        BufferPage *page_1 = file_1->get_page(page_id);
        assert(page_1->data[PAGE_SIZE / 4 - 1] == PAGE_MAX - page_id);
    }
    Filesystem::remove_file(FILENAME_0);
    Filesystem::remove_file(FILENAME_1);
}

void test_dirty() {
    Filesystem fs = Filesystem();
    Filesystem::create_file(FILENAME_2);
    File *file = fs.open_file(FILENAME_2);
    BufferPage *page;

    page = file->get_page(0);
    assert(page->dirty == false);
    page->data[0] = 'A';
    page->dirty = true;

    page = file->get_page(1);
    assert(page->dirty == false);
    page->data[0] = 'B';
    page->dirty = true;

    page = file->get_page(0);
    assert(page->dirty == true);
    assert(page->data[0] == 'A');

    page = file->get_page(1);
    assert(page->dirty == true);
    assert(page->data[0] == 'B');

    file->close();
    file = fs.open_file(FILENAME_2);

    page = file->get_page(0);
    assert(page->dirty == false);
    assert(page->data[0] == 'A');

    page = file->get_page(1);
    assert(page->dirty == false);
    assert(page->data[0] == 'B');

    file->close();
    Filesystem::remove_file(FILENAME_2);
}

int main() {
    test_write();
    test_read();
    test_dirty();
    return 0;
}
