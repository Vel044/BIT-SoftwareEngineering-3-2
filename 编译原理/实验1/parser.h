#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <map>

enum TokenType {
    INT, RETURN, MAIN, PRINTLN_INT,
    IDENTIFIER, INTEGER,
    ASSIGN, PLUS, MINUS, MUL, DIV, MOD,
    LT, LE, GT, GE, EQ, NE,
    BIT_AND, BIT_OR, BIT_XOR, LOG_AND,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON
};

union YYSTYPE {
    std::string* str;
    int num;
    struct Node* node;
};
extern YYSTYPE yylval;

struct Node {
    virtual ~Node() {}
};

struct Program : Node {
    Node* function;
};

struct Function : Node {
    std::string name;
    std::vector<std::string> params;
    std::vector<Node*> statements;
};

struct VarDecl : Node {
    std::string name;
};

struct Assign : Node {
    std::string var;
    Node* expr;
};

struct Return : Node {
    Node* expr;
};

struct FuncCall : Node {
    std::string name;
    Node* arg;
};

struct Expr : Node {};

struct Constant : Expr {
    int value;
};

struct Variable : Expr {
    std::string name;
};

struct BinaryOp : Expr {
    std::string op;
    Node* left;
    Node* right;
};

class Parser {
public:
    Parser(std::vector<std::pair<TokenType, YYSTYPE>>& tokens);
    Node* parse();
private:
    std::vector<std::pair<TokenType, YYSTYPE>> tokens;
    size_t pos;
    std::map<std::string, int> var_offsets;
    int var_count;
    TokenType current_token();
    YYSTYPE current_value();
    void advance();
    Node* parse_program();
    Node* parse_function();
    Node* parse_statement();
    Node* parse_var_decl();
    Node* parse_assign();
    Node* parse_return();
    Node* parse_func_call();
    Node* parse_expr();
    Node* parse_expr_precedence(int min_prec);
};

#endif