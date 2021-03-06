#include "Application.h"

void Application::parse(const std::string &sSQL) {
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
    if (iParser.getNumberOfSyntaxErrors() > 0) {
        frontend.error("Syntax error. Please check.");
        return;
    }
    // 构造你的visitor
    // visitor模式下执行SQL解析过程
    iVisitor.visit(iTree);
}

Application::Application(const std::string &system_root)
        : ms{ManageSystem::load_system(system_root, &frontend)}, iVisitor{ms} {}

void Application::run() {
    frontend.write_line("Welcome to MyDBMS!");
    while (true) {
        std::string stmt = frontend.read_stmt();
        if (stmt == "EXIT;") break;
        clock_t start = clock();
        parse(stmt);
        clock_t end = clock();
        frontend.info("Elapsed " + std::to_string((double) (end - start) / CLOCKS_PER_SEC) + " seconds.");
    }
    frontend.write_line("Bye");
}
