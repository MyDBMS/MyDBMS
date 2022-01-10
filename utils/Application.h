#pragma once

#include "antlr4-runtime.h"
#include "Frontend.h"
#include "MyVisitor.h"
#include "SQLLexer.h"
#include "SQLParser.h"
#include "SQLBaseVisitor.h"
#include "../utils/Application.h"

class Application {
    StdioFrontend frontend{};

    ManageSystem ms;

    MyVisitor iVisitor;

    void parse(const std::string &sSQL);

public:

    explicit Application(const std::string &system_root);

    void run();
};