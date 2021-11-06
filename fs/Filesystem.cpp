#include "Filesystem.h"

void Filesystem::create_file(const char *filename) {
    File::create(filename);
}

void Filesystem::remove_file(const char *filename) {
    File::rm(filename);
}

File *Filesystem::open_file(const char *filename) {
    auto file = new File(&buffer_manager);
    file->open(filename, files.size());
    files.push_back(file);
    return file;
}

Filesystem::~Filesystem() {
    for (File *file: files) {
        if (file) {
            file->close();
            delete file;
        }
    }
}

