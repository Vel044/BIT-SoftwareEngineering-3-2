#include <fstream>
#include <iostream>
#include "parser.h"
#include "codegen.h"
#include "FlexLexer.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }

    yyFlexLexer lexer(&input);
    std::vector<std::pair<TokenType, YYSTYPE>> tokens;
    int token;
    while ((token = lexer.yylex()) != 0) {
        if (token == -1) return 1;
        YYSTYPE val = yylval;
        tokens.emplace_back(static_cast<TokenType>(token), val);
    }

    try {
        Parser parser(tokens);
        Node* ast = parser.parse();
        CodeGenerator codegen(ast, parser.var_offsets);
        std::cout << codegen.generate();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}