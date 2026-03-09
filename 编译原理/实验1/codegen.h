#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include <string>
#include <map>

class CodeGenerator {
public:
    CodeGenerator(Node* ast, const std::map<std::string, int>& offsets);
    std::string generate();
private:
    Node* ast;
    std::map<std::string, int> var_offsets;
    int label_count;
    std::string generate_program(Node* node);
    std::string generate_function(Node* node);
    std::string generate_statement(Node* node);
    std::string generate_var_decl(Node* node);
    std::string generate_assign(Node* node);
    std::string generate_return(Node* node);
    std::string generate_func_call(Node* node);
    std::string generate_expr(Node* node);
};

#endif