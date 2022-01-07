#include "Frontend.h"

std::string Frontend::pad(const std::string &s, std::size_t max_width) {
    return " " + s + std::string(max_width - s.length() + 1, ' ');
}

void Frontend::print_table(const Frontend::Table &table) const {
    // Count width
    std::vector<std::size_t> max_value_width;
    std::vector<std::string> line_segments;
    std::string separator = "+";
    std::size_t row_count = table.empty() ? 0 : table[0].values.size();
    for (const auto &column: table) {
        std::size_t w = 0;
        if (column.name.length() > w) {
            w = column.name.length();
        }
        for (const auto &v: column.values) {
            if (v.length() > w) {
                w = v.length();
            }
        }
        max_value_width.emplace_back(w);
        line_segments.emplace_back(w + 2, '-');
        separator += std::string(w + 2, '-') + "+";
        assert(row_count == column.values.size());
    }
    separator += "\n";

    // Print header
    print_string(separator);
    print_string("|");
    for (std::size_t i = 0; i < table.size(); ++i) {
        print_string(pad(table[i].name, max_value_width[i]));
        print_string("|");
    }
    print_string("\n");
    print_string(separator);

    // Print body
    for (std::size_t r = 0; r < row_count; ++r) {
        print_string("|");
        for (std::size_t i = 0; i < table.size(); ++i) {
            print_string(pad(table[i].values[r], max_value_width[i]));
            print_string("|");
        }
        print_string("\n");
    }
    if (row_count > 0) {
        print_string(separator);
    }

    // Print footer
    print_string(std::to_string(row_count) + " row" + (row_count == 1 ? "" : "s") + " in set\n");
}

void StdioFrontend::print_string(const std::string &s) const {
    std::cout << s;
}

void StrStreamFrontend::print_string(const std::string &s) const {
    stream << s;
}

StrStreamFrontend::StrStreamFrontend(std::strstream &stream) : stream(stream) {
}
