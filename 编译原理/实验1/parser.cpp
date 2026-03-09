#include "parser.h"
#include <stdexcept>

static const std::map<std::string, int> op_precedence = {
    {"+", 5}, {"-", 5},
    {"*", 6}, {"/", 6}, {"%", 6},
    {"<", 4}, {"<=", 4}, {">", 4}, {">=", 4},
    {"==", 3}, {"!=", 3},
    {"&", 2}, {"^", 1}, {"|", 0},
    {"&&", -1}
};

Parser::Parser(std::vector<std::pair<TokenType, YYSTYPE>>& t) : tokens(t), pos(0), var_count(0) {}

TokenType Parser::current_token() {
    return pos < tokens.size() ? tokens[pos].first : TokenType(-1);
}

YYSTYPE Parser::current_value() {
    return tokens[pos].second;
}

void Parser::advance() {
    pos++;
}

Node* Parser::parse() {
    return parse_program();
}

Node* Parser::parse_program() {
    Program* prog = new Program;
    prog->function = parse_function();
    return prog;
}

Node* Parser::parse_function() {
    Function* func = new Function;
    if (current_token() != INT) throw std::runtime_error("Expected 'int'");
    advance();
    if (current_token() != MAIN) throw std::runtime_error("Expected 'main'");
    func->name = *current_value().str;
    advance();
    if (current_token() != LPAREN) throw std::runtime_error("Expected '('");
    advance();
    if (current_token() == INT) {
        advance();
        if (current_token() != IDENTIFIER) throw std::runtime_error("Expected identifier");
        func->params.push_back(*current_value().str);
        var_offsets[*current_value().str] = 8; // argc at [ebp+8]
        advance();
        if (current_token() == SEMICOLON) {
            advance();
            if (current_token() == INT) {
                advance();
                if (current_token() != IDENTIFIER) throw std::runtime_error("Expected identifier");
                func->params.push_back(*current_value().str);
                var_offsets[*current_value().str] = 12; // argv at [ebp+12]
                advance();
            }
        }
    }
    if (current_token() != RPAREN) throw std::runtime_error("Expected ')'");
    advance();
    if (current_token() != LBRACE) throw std::runtime_error("Expected '{'");
    advance();
    while (current_token() != RBRACE) {
        func->statements.push_back(parse_statement());
    }
    advance();
    return func;
}

Node* Parser::parse_statement() {
    if (current_token() == INT) return parse_var_decl();
    if (current_token() == IDENTIFIER) return parse_assign();
    if (current_token() == RETURN) return parse_return();
    if (current_token() == PRINTLN_INT) return parse_func_call();
    throw std::runtime_error("Invalid statement");
}

Node* Parser::parse_var_decl() {
    VarDecl* decl = new VarDecl;
    advance(); // Skip INT
    if (current_token() != IDENTIFIER) throw std::runtime_error("Expected identifier");
    decl->name = *current_value().str;
    var_offsets[decl->name] = -(++var_count * 4); // [ebp-4], [ebp-8], ...
    advance();
    if (current_token() != SEMICOLON) throw std::runtime_error("Expected ';'");
    advance();
    return decl;
}

Node* Parser::parse_assign() {
    Assign* assign = new Assign;
    if (current_token() != IDENTIFIER) throw std::runtime_error("Expected identifier");
    assign->var = *current_value().str;
    advance();
    if (current_token() != ASSIGN) throw std::runtime_error("Expected '='");
    advance();
    assign->expr = parse_expr();
    if (current_token() != SEMICOLON) throw std::runtime_error("Expected ';'");
    advance();
    return assign;
}

Node* Parser::parse_return() {
    Return* ret = new Return;
    advance(); // Skip RETURN
    ret->expr = parse_expr();
    if (current_token() != SEMICOLON) throw std::runtime_error("Expected ';'");
    advance();
    return ret;
}

Node* Parser::parse_func_call() {
    FuncCall* call = new FuncCall;
    if (current_token() != PRINTLN_INT) throw std::runtime_error("Expected 'println_int'");
    call->name = "println_int";
    advance();
    if (current_token() != LPAREN) throw std::runtime_error("Expected '('");
    advance();
    if (current_token() == IDENTIFIER) {
        Variable* var = new Variable;
        var->name = *current_value().str;
        call->arg = var;
    } else if (current_token() == INTEGER) {
        Constant* con = new Constant;
        con->value = current_value().num;
        call->arg = con;
    } else {
        throw std::runtime_error("Expected identifier or integer");
    }
    advance();
    if (current_token() != RPAREN) throw std::runtime_error("Expected ')'");
    advance();
    if (current_token() != SEMICOLON) throw std::runtime_error("Expected ';'");
    advance();
    return call;
}

Node* Parser::parse_expr() {
    return parse_expr_precedence(-1);
}

Node* Parser::parse_expr_precedence(int min_prec) {
    Node* left;
    if (current_token() == INTEGER) {
        Constant* con = new Constant;
        con->value = current_value().num;
        left = con;
        advance();
    } else if (current_token() == IDENTIFIER) {
        Variable* var = new Variable;
        var->name = *current_value().str;
        left = var;
        advance();
    } else if (current_token() == LPAREN) {
        advance();
        left = parse_expr();
        if (current_token() != RPAREN) throw std::runtime_error("Expected ')'");
        advance();
    } else {
        throw std::runtime_error("Invalid expression");
    }

    while (true) {
        std::string op;
        int prec = -2;
        if (current_token() == PLUS) op = "+", prec = op_precedence.at("+");
        else if (current_token() == MINUS) op = "-", prec = op_precedence.at("-");
        else if (current_token() == MUL) op = "*", prec = op_precedence.at("*");
        else if (current_token() == DIV) op = "/", prec = op_precedence.at("/");
        else if (current_token() == MOD) op = "%", prec = op_precedence.at("%");
        else if (current_token() == LT) op = "<", prec = op_precedence.at("<");
        else if (current_token() == LE) op = "<=", prec = op_precedence.at("<=");
        else if (current_token() == GT) op = ">", prec = op_precedence.at(">");
        else if (current_token() == GE) op = ">=", prec = op_precedence.at(">=");
        else if (current_token() == EQ) op = "==", prec = op_precedence.at("==");
        else if (current_token() == NE) op = "!=", prec = op_precedence.at("!=");
        else if (current_token() == BIT_AND) op = "&", prec = op_precedence.at("&");
        else if ( personally) op = "|", prec = op_precedence.at("|");
        else if (current_token() == BIT_XOR) op = "^", prec = op_precedence.at("^");
        else if (current_token() == LOG_AND) op = "&&", prec = op_precedence.at("&&");
        else break;

        if (prec < min_prec) break;
        advance();
        Node* right = parse_expr_precedence(prec + 1);
        BinaryOp* binop = new BinaryOp;
        binop->op = op;
        binop->left = left;
        binop->right = right;
        left = binop;
    }
    return left;
}