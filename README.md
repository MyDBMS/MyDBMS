# MyDBMS

## 文件管理

`fs` 目录下实现了一个简单的页式文件管理系统。它由三个类（`Filesystem`、`File`、`BufferManager`）和一个结构（`BufferPage`）构成，呈现出如下架构：

- `Filesystem` 是整个文件系统的入口类。它负责管理文件系统中用到的所有 `File` 对象，并维护了一个缓存管理器 `BufferManager`。一个文件系统中的所有 `File` 共享一个 `BufferManager` 的引用。
- `File` 类封装了文件读写相关的系统函数。由于这是一个页式文件管理系统，`File` 类对外暴露的只有 `get_page` 和 `close` 方法，其它文件操作相关的方法或者不提供，或者由 `Filesystem` 类统一管理。
- `BufferManager` 是一个基于 LRU 算法的缓存管理系统，用于对页进行缓存管理。每当一个 `File` 对象希望获取一个页时，它不会立即进行文件读写，而是会先尝试从 `BufferManager` 寻找缓存，并根据是否命中的情况考虑是否进行文件读写。此外，当一个 `File` 对象关闭时，它也会从 `BufferManager` 中获取所有属于它的脏页，并进行写回。
- `BufferPage` 结构用于描述一个缓存页。调用 `File::get_page` 方法将会得到一个 `BufferPage` 的指针。如果需要对 `BufferPage` 进行写操作，用户需自行将 `dirty` 字段改为 `true`；当这片缓存被交换或文件被关闭时，脏缓存页的内容会被写回。一旦通过 `File::get_page` 方法获得了一个 `BufferPage` 引用，调用者应尽快使用，以防该 `BufferPage` 因为被交换出缓存队列而失效。

文件系统的使用样例可参见 `tests/fs-test.cpp`。