#include "Frontend.h"

std::string Frontend::read_stmt() {
    char ch;
    std::string s;
    bool in_str = false;
    while (true) {
        ch = read_char();
        if (ch == EOF) return "EXIT;";
        if (!(s.empty() && (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t'))) s += ch;
        if (ch == '\'') in_str = !in_str;
        if (!in_str && ch == ';') return s;
    }
}

std::string Frontend::pad(const std::string &s, std::size_t max_width) {
    return " " + s + std::string(max_width - s.length() + 1, ' ');
}

Frontend::Frontend(bool test_mode) : test_mode(test_mode) {
}

void Frontend::print_table(const Frontend::Table &table) const {
    // Count width
    std::vector<std::size_t> max_value_width;
    std::vector<std::string> line_segments;
    std::string separator = "+";
    std::size_t row_count = table.empty() ? 0 : table[0].values.size();

    if (row_count == 0) {
        write_string("Empty set\n");
        return;
    }

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
    write_string(separator);
    write_string("|");
    for (std::size_t i = 0; i < table.size(); ++i) {
        write_string(pad(table[i].name, max_value_width[i]));
        write_string("|");
    }
    write_string("\n");
    write_string(separator);

    // Print body
    for (std::size_t r = 0; r < row_count; ++r) {
        write_string("|");
        for (std::size_t i = 0; i < table.size(); ++i) {
            write_string(pad(table[i].values[r], max_value_width[i]));
            write_string("|");
        }
        write_string("\n");
    }
    if (row_count > 0) {
        write_string(separator);
    }

    // Print footer
    write_string(std::to_string(row_count) + " row" + (row_count == 1 ? "" : "s") + " in set\n");
}

void Frontend::ok(int row_cnt) const {
    info("Query OK, " + std::to_string(row_cnt) + " row" + (row_cnt == 1 ? "" : "s") + " affected.");
}

void Frontend::info(const std::string &msg) const {
    write_string(test_mode ? "[INFO] " : "\x1b[34m[INFO] ");
    write_string(msg);
    write_string(test_mode ? "\n" : "\x1b[0m\n");
}

void Frontend::error(const std::string &msg) const {
    write_string(test_mode ? "[ERROR] " : "\x1b[31m[ERROR] ");
    write_string(msg);
    write_string(test_mode ? "\n" : "\x1b[0m\n");
}

void Frontend::warning(const std::string &msg) const {
    write_string(test_mode ? "[WARNING] " : "\x1b[33m[WARNING] ");
    write_string(msg);
    write_string(test_mode ? "\n" : "\x1b[0m\n");
}

void Frontend::write_line(const std::string &msg) const {
    write_string(msg + "\n");
}

void StdioFrontend::write_string(const std::string &s) const {
    std::cout << s;
}

char StdioFrontend::read_char() {
    return (char) std::cin.get();
}

StdioFrontend::StdioFrontend() : Frontend(false) {
};

void StringStreamFrontend::write_string(const std::string &s) const {
    ostream << s;
}

StringStreamFrontend::StringStreamFrontend(std::stringstream &istream, std::stringstream &ostream, bool test_mode)
        : Frontend(test_mode), istream(istream), ostream(ostream) {}

char StringStreamFrontend::read_char() {
    return (char) istream.get();
}
