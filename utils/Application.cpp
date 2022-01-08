#include "Application.h"

auto Application::parse(const std::string &sSQL) {
    // 解析SQL语句sSQL的过程
    // std::cout << "\n\x1b[34m[INFO] User Input: " << sSQL << "\x1b[0m\n" << std::endl;
    // 转化为输入流
    antlr4::ANTLRInputStream sInputStream(sSQL);
    /* frontend.info("user input: " + sSQL); */
    // 设置Lexer
    SQLLexer iLexer(&sInputStream);
    antlr4::CommonTokenStream sTokenStream(&iLexer);
    // 设置Parser
    SQLParser iParser(&sTokenStream);
    auto iTree = iParser.program();
    // 构造你的visitor
    // visitor模式下执行SQL解析过程
    auto iRes = iVisitor.visit(iTree);
    return iRes;
}

Application::Application(const std::string &system_root)
        : ms{ManageSystem::load_system(system_root, &frontend)}, iVisitor{ms} {}

void Application::run() {
    while (true) {
        std::string stmt = frontend.read_stmt();
        if (stmt == "EXIT;") break;
        parse(stmt);
    }
    frontend.write_line("Bye");
}
