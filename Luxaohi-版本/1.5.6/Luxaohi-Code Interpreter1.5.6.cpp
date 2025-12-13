#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <cctype>
#include <zmq.h>
#include <cstring>
#ifdef _WIN32
#include <shlobj.h>  
#pragma comment(lib, "shell32.lib") 
#endif
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;
std::string global_username = "Administrator";

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

// 全局ZeroMQ变量
void* g_zmq_context = nullptr;
void* g_zmq_socket = nullptr;

// 初始化ZeroMQ
bool initZeroMQ() {
    g_zmq_context = zmq_ctx_new();
    if (!g_zmq_context) {
        return false;
    }

    g_zmq_socket = zmq_socket(g_zmq_context, ZMQ_PUSH);
    if (!g_zmq_socket) {
        zmq_ctx_destroy(g_zmq_context);
        return false;
    }

    int result = zmq_connect(g_zmq_socket, "tcp://localhost:5555");
    if (result != 0) {
        zmq_close(g_zmq_socket);
        zmq_ctx_destroy(g_zmq_context);
        g_zmq_socket = nullptr;
        g_zmq_context = nullptr;
        return false;
    }

    return true;
}

// 发送文本到Python语音服务
void sendTextToSpeech(const std::string& text) {
    if (!g_zmq_socket || text.empty()) {
        return;
    }

    zmq_send(g_zmq_socket, text.c_str(), text.length(), 0);
}

// 简化的 speakError 函数
void speakError(const std::string& error_message) {
    std::string clean_error;

    if (error_message.find("未闭合的字符串") != std::string::npos) {
        clean_error = "好像是未闭合的字符串呢";
    }
    else if (error_message.find("未定义的函数") != std::string::npos) {
        clean_error = "好像是未定义的函数呢喵";
    }
    else if (error_message.find("参数数量不匹配") != std::string::npos) {
        clean_error = "这是？参数数量不匹配？";
    }
    else if (error_message.find("未知字符") != std::string::npos) {
        clean_error = "喵！未知字符";
    }
    else {
        size_t start_pos = error_message.find("罗小黑[");
        if (start_pos != std::string::npos) {
            size_t end_pos = error_message.find("]", start_pos + 7);
            if (end_pos != std::string::npos) {
                clean_error = error_message.substr(start_pos + 7, end_pos - (start_pos + 7));
            }
        }
    }

    if (clean_error.empty()) {
        clean_error = "代码有语法错误呢";
    }

    if (!clean_error.empty()) {
        sendTextToSpeech(clean_error);
    }
}

// 清理资源
void cleanupZeroMQ() {
    if (g_zmq_socket) {
        zmq_close(g_zmq_socket);
        g_zmq_socket = nullptr;
    }
    if (g_zmq_context) {
        zmq_ctx_destroy(g_zmq_context);
        g_zmq_context = nullptr;
    }
}

// 初始化朗读功能
void initTextToSpeech() {
    initZeroMQ();
}

//安全的字符检查函数
inline bool is_alpha_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c & 0x80);
}

inline bool is_alnum_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || (c & 0x80);
}

// 设置控制台编码
void initConsoleEncoding() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif

    try {
        locale utf8_locale("zh_CN.UTF-8");
        cout.imbue(utf8_locale);
        cin.imbue(utf8_locale);
    }
    catch (...) {
        locale utf8_locale(locale(), new codecvt_utf8<wchar_t>);
        cout.imbue(utf8_locale);
        cin.imbue(utf8_locale);
    }
}

//用户名获取
std::string getUsernameFlexible() {
    std::vector<std::string> tryMethods;

#ifdef _WIN32
    tryMethods = { "USERNAME", "USER", "USERPROFILE" };
#else  
    tryMethods = { "USER", "LOGNAME", "USERNAME" };
#endif

    for (const auto& envVar : tryMethods) {
        const char* value = std::getenv(envVar.c_str());
        if (value && value[0] != '\0') {
            return value;
        }
    }
    return "Administrator";
}

//语法树节点
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void execute(class Interpreter&) = 0;
};

//解释器
class Interpreter {
public:
    struct Function {
        vector<string> params;
        vector<shared_ptr<ASTNode>> body;
    };

    map<string, string> variables;

    void addFunction(const string& name, const Function& func) {
        functions[name] = func;
    }

    void execute(const vector<shared_ptr<ASTNode>>& nodes) {
        for (auto& node : nodes) {
            node->execute(*this);
        }
    }

    void callFunction(const string& name, const vector<string>& args) {
        if (functions.find(name) != functions.end()) {
            const auto& func = functions[name];
            if (args.size() != func.params.size()) {
                string error_msg = "罗小黑[这是？参数数量不匹配？]: " + name;
                throw runtime_error(error_msg);
            }
            for (const auto& node : func.body) {
                node->execute(*this);
            }
        }
        else if (name == "喵叫") {
            for (const auto& arg : args) {
                cout << evaluate(arg);
            }
            cout << endl;
        }
        else {
            string error_msg = "罗小黑[好像是未定义的函数呢喵~]: " + name;
            throw runtime_error(error_msg);
        }
    }

    string evaluate(const string& expr) {
        if (variables.find(expr) != variables.end()) {
            return variables[expr];
        }
        if (expr.find_first_of("+-*/()") != string::npos) {
            return evaluateExpression(expr);
        }
        return expr;
    }

    bool evaluateCondition(const string& condition) {
        string cond = condition;

        size_t eq_pos = cond.find("==");
        size_t ne_pos = cond.find("!=");
        size_t gt_pos = cond.find(">");
        size_t lt_pos = cond.find("<");
        size_t ge_pos = cond.find(">=");
        size_t le_pos = cond.find("<=");

        if (eq_pos != string::npos) {
            string left = evaluate(cond.substr(0, eq_pos));
            string right = evaluate(cond.substr(eq_pos + 2));
            return left == right;
        }
        else if (ne_pos != string::npos) {
            string left = evaluate(cond.substr(0, ne_pos));
            string right = evaluate(cond.substr(ne_pos + 2));
            return left != right;
        }
        else if (ge_pos != string::npos) {
            string left_str = evaluate(cond.substr(0, ge_pos));
            string right_str = evaluate(cond.substr(ge_pos + 2));
            int left = stoi(left_str);
            int right = stoi(right_str);
            return left >= right;
        }
        else if (le_pos != string::npos) {
            string left_str = evaluate(cond.substr(0, le_pos));
            string right_str = evaluate(cond.substr(le_pos + 2));
            int left = stoi(left_str);
            int right = stoi(right_str);
            return left <= right;
        }
        else if (gt_pos != string::npos) {
            string left_str = evaluate(cond.substr(0, gt_pos));
            string right_str = evaluate(cond.substr(gt_pos + 1));
            int left = stoi(left_str);
            int right = stoi(right_str);
            return left > right;
        }
        else if (lt_pos != string::npos) {
            string left_str = evaluate(cond.substr(0, lt_pos));
            string right_str = evaluate(cond.substr(lt_pos + 1));
            int left = stoi(left_str);
            int right = stoi(right_str);
            return left < right;
        }

        string val = evaluate(cond);
        return !val.empty() && val != "0" && val != "假";
    }

private:
    map<string, Function> functions;

    bool isInteger(const string& str) {
        if (str.empty()) return false;
        size_t start = (str[0] == '-') ? 1 : 0;
        if (start >= str.size()) return false;
        for (size_t i = start; i < str.size(); i++) {
            if (!isdigit(str[i])) return false;
        }
        return true;
    }

    string evaluateExpression(const string& expr) {
        string cleanExpr;
        for (char c : expr) {
            if (c != ' ') cleanExpr += c;
        }
        size_t pos = 0;
        string result = parseAddition(cleanExpr, pos);
        if (pos != cleanExpr.size()) {
            string error_msg = "Niko[" + global_username + "，表达式解析不完整。]";
            throw runtime_error(error_msg);
        }
        return result;
    }

    string parseAddition(const string& expr, size_t& pos) {
        string left = parseMultiplication(expr, pos);

        while (pos < expr.size()) {
            char op = expr[pos];
            if (op == '+' || op == '-') {
                pos++;
                string right = parseMultiplication(expr, pos);

                if (!isInteger(left) || !isInteger(right)) {
                    string error_msg = "Niko[" + global_username + "，数学运算需要数字参数]";
                    throw runtime_error(error_msg);
                }

                int left_val = stoi(left);
                int right_val = stoi(right);
                int result = (op == '+') ? left_val + right_val : left_val - right_val;
                left = to_string(result);
            }
            else {
                break;
            }
        }
        return left;
    }

    string parseMultiplication(const string& expr, size_t& pos) {
        string left = parsePrimary(expr, pos);

        while (pos < expr.size()) {
            char op = expr[pos];
            if (op == '*' || op == '/') {
                pos++;
                string right = parsePrimary(expr, pos);

                if (!isInteger(left) || !isInteger(right)) {
                    string error_msg = "Niko[" + global_username + "，数学运算需要数字参数]";
                    throw runtime_error(error_msg);
                }

                int left_val = stoi(left);
                int right_val = stoi(right);
                if (op == '*') {
                    int result = left_val * right_val;
                    left = to_string(result);
                }
                else {
                    if (right_val == 0) {
                        string error_msg = "Niko[" + global_username + "，不能除以零的。]";
                        throw runtime_error(error_msg);
                    }
                    int result = left_val / right_val;
                    left = to_string(result);
                }
            }
            else {
                break;
            }
        }
        return left;
    }

    string parsePrimary(const string& expr, size_t& pos) {
        if (pos >= expr.size()) {
            string error_msg = "Niko[" + global_username + "，表达式不完整......]";
            throw runtime_error(error_msg);
        }

        if (isdigit(expr[pos])) {
            string num;
            while (pos < expr.size() && isdigit(expr[pos])) {
                num += expr[pos++];
            }
            return num;
        }

        if (is_alpha_char(expr[pos])) {
            string var;
            while (pos < expr.size() && is_alnum_char(expr[pos])) {
                var += expr[pos++];
            }
            if (variables.find(var) != variables.end()) {
                return variables[var];
            }
            string error_msg = "Niko[" + global_username + "，是未定义变量: " + var + "。]";
            throw runtime_error(error_msg);
        }

        if (expr[pos] == '(') {
            pos++;
            string result = parseAddition(expr, pos);
            if (pos >= expr.size() || expr[pos] != ')') {
                string error_msg = "Niko[" + global_username + "，括号不匹配。]";
                throw runtime_error(error_msg);
            }
            pos++;
            return result;
        }
        if (expr[pos] == '-' && pos + 1 < expr.size() && isdigit(expr[pos + 1])) {
            pos++;
            string num = "-";
            while (pos < expr.size() && isdigit(expr[pos])) {
                num += expr[pos++];
            }
            return num;
        }

        string error_msg = "Niko[" + global_username + "，这个表达式我看不懂: " + string(1, expr[pos]) + "]";
        throw runtime_error(error_msg);
    }
};

// 函数定义节点
class FunctionDecl : public ASTNode {
public:
    string name;
    vector<string> params;
    vector<shared_ptr<ASTNode>> body;

    void execute(Interpreter& interpreter) override {
        interpreter.addFunction(name, { params, body });
    }
};

// 函数调用节点
class FunctionCall : public ASTNode {
public:
    string name;
    vector<string> args;

    void execute(Interpreter& interpreter) override {
        interpreter.callFunction(name, args);
    }
};

// 变量赋值节点
class VariableAssign : public ASTNode {
public:
    string name;
    string value;

    void execute(Interpreter& interpreter) override {
        interpreter.variables[name] = interpreter.evaluate(value);
    }
};

// 条件语句节点
class IfStatement : public ASTNode {
public:
    string condition;
    vector<shared_ptr<ASTNode>> thenBranch;
    vector<shared_ptr<ASTNode>> elseBranch;

    void execute(Interpreter& interpreter) override {
        if (interpreter.evaluateCondition(condition)) {
            for (auto& node : thenBranch) {
                node->execute(interpreter);
            }
        }
        else {
            for (auto& node : elseBranch) {
                node->execute(interpreter);
            }
        }
    }
};

// 词法分析器
class Lexer {
public:
    enum TokenType {
        FUNC_KW, IF_KW, THEN_KW, ELSE_KW, VAR_KW, IDENT, NUMBER, STRING,
        LPAREN, RPAREN, LBRACE, RBRACE, COMMA, SEMI, ASSIGN, PLUS, MINUS,
        MULTIPLY, DIVIDE, EQ, NE, GT, LT, END
    };

    struct Token {
        TokenType type;
        string value;
        int line;
        int column;
        Token(TokenType t, string v, int l, int c)
            : type(t), value(move(v)), line(l), column(c) {
        }
    };

    Lexer(const string& code) : code(code), pos(0), line(1), column(1) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        while (pos < code.size()) {
            char c = code[pos];
            if (isspace(c)) {
                if (c == '\n') {
                    line++;
                    column = 1;
                }
                else {
                    column++;
                }
                pos++;
                continue;
            }
            if (is_alpha_char(c)) {
                string value;
                while (pos < code.size() && is_alnum_char(code[pos])) {
                    value += code[pos++];
                    column++;
                }
                if (value == "喵") {
                    tokens.emplace_back(FUNC_KW, value, line, column - value.size());
                }
                else if (value == "如果") {
                    tokens.emplace_back(IF_KW, value, line, column - value.size());
                }
                else if (value == "那么") {
                    tokens.emplace_back(THEN_KW, value, line, column - value.size());
                }
                else if (value == "否则") {
                    tokens.emplace_back(ELSE_KW, value, line, column - value.size());
                }
                else if (value == "变量") {
                    tokens.emplace_back(VAR_KW, value, line, column - value.size());
                }
                else {
                    tokens.emplace_back(IDENT, value, line, column - value.size());
                }
                continue;
            }
            if (isdigit(c)) {
                string value;
                while (pos < code.size() && (isdigit(code[pos]) || code[pos] == '.')) {
                    value += code[pos++];
                    column++;
                }
                tokens.emplace_back(NUMBER, value, line, column - value.size());
                continue;
            }
            if (c == '"' || c == '\'') {
                char quote = c;
                string value;
                pos++; column++;
                while (pos < code.size() && code[pos] != quote) {
                    if (code[pos] == '\\') {
                        pos++; column++;
                        if (pos >= code.size()) break;
                        switch (code[pos]) {
                        case 'n': value += '\n'; break;
                        case 't': value += '\t'; break;
                        case '\\': value += '\\'; break;
                        case '"': value += '"'; break;
                        case '\'': value += '\''; break;
                        default: value += code[pos]; break;
                        }
                    }
                    else {
                        value += code[pos];
                    }
                    pos++; column++;
                }
                if (pos >= code.size()) {
                    throw runtime_error("罗小黑[好像是未闭合的字符串呢]");
                }
                pos++; column++;
                tokens.emplace_back(STRING, value, line, column - value.size() - 2);
                continue;
            }
            if (c == '=' && pos + 1 < code.size() && code[pos + 1] == '=') {
                tokens.emplace_back(EQ, "==", line, column);
                pos += 2; column += 2;
                continue;
            }
            else if (c == '!' && pos + 1 < code.size() && code[pos + 1] == '=') {
                tokens.emplace_back(NE, "!=", line, column);
                pos += 2; column += 2;
                continue;
            }
            else if (c == '=') {
                tokens.emplace_back(ASSIGN, "=", line, column);
                pos++; column++;
                continue;
            }
            else if (c == '+') {
                tokens.emplace_back(PLUS, "+", line, column);
                pos++; column++;
                continue;
            }
            else if (c == '-') {
                tokens.emplace_back(MINUS, "-", line, column);
                pos++; column++;
                continue;
            }
            else if (c == '*') {
                tokens.emplace_back(MULTIPLY, "*", line, column);
                pos++; column++;
                continue;
            }
            else if (c == '/') {
                tokens.emplace_back(DIVIDE, "/", line, column);
                pos++; column++;
                continue;
            }
            else if (c == '>') {
                tokens.emplace_back(GT, ">", line, column);
                pos++; column++;
                continue;
            }
            else if (c == '<') {
                tokens.emplace_back(LT, "<", line, column);
                pos++; column++;
                continue;
            }

            switch (c) {
            case '(': tokens.emplace_back(LPAREN, "(", line, column); break;
            case ')': tokens.emplace_back(RPAREN, ")", line, column); break;
            case '{': tokens.emplace_back(LBRACE, "{", line, column); break;
            case '}': tokens.emplace_back(RBRACE, "}", line, column); break;
            case ',': tokens.emplace_back(COMMA, ",", line, column); break;
            case ';': tokens.emplace_back(SEMI, ";", line, column); break;
            default:
                throw runtime_error(string("罗小黑[喵！]未知字符: ") + c);
            }
            pos++; column++;
        }
        tokens.emplace_back(END, "", line, column);
        return tokens;
    }

private:
    string code;
    size_t pos;
    int line;
    int column;
};

// 语法分析器
class Parser {
public:
    Parser(const vector<Lexer::Token>& tokens) : tokens(tokens), pos(0) {}

    vector<shared_ptr<ASTNode>> parse() {
        vector<shared_ptr<ASTNode>> nodes;
        while (!match(Lexer::END)) {
            if (match(Lexer::FUNC_KW)) {
                nodes.push_back(parseFunction());
            }
            else if (match(Lexer::IF_KW)) {
                nodes.push_back(parseIfStatement());
            }
            else if (match(Lexer::VAR_KW)) {
                nodes.push_back(parseVariableAssign());
            }
            else if (checkIdentWithLParen()) {
                nodes.push_back(parseCall());
            }
            else {
                consume();
            }
        }
        return nodes;
    }

private:
    bool match(Lexer::TokenType type) {
        return pos < tokens.size() && tokens[pos].type == type;
    }

    bool checkIdentWithLParen() {
        return pos + 1 < tokens.size() &&
            tokens[pos].type == Lexer::IDENT &&
            tokens[pos + 1].type == Lexer::LPAREN;
    }

    Lexer::Token consume(Lexer::TokenType type, const string& err) {
        if (match(type)) {
            return tokens[pos++];
        }
        throw runtime_error(err);
    }

    Lexer::Token consume() {
        return tokens[pos++];
    }

    shared_ptr<FunctionDecl> parseFunction() {
        auto func = make_shared<FunctionDecl>();
        consume(Lexer::FUNC_KW, "预期函数关键字'喵'");
        func->name = consume(Lexer::IDENT, "预期函数名").value;
        consume(Lexer::LPAREN, "预期'('");
        while (!match(Lexer::RPAREN)) {
            func->params.push_back(consume(Lexer::IDENT, "预期参数名").value);
            if (!match(Lexer::RPAREN)) {
                consume(Lexer::COMMA, "预期','分隔参数");
            }
        }
        consume(Lexer::RPAREN, "预期')'");
        consume(Lexer::LBRACE, "预期'{'开始函数体");
        while (!match(Lexer::RBRACE)) {
            if (match(Lexer::IF_KW)) {
                func->body.push_back(parseIfStatement());
            }
            else if (match(Lexer::VAR_KW)) {
                func->body.push_back(parseVariableAssign());
            }
            else if (checkIdentWithLParen()) {
                func->body.push_back(parseCall());
            }
            else {
                consume();
            }
        }
        consume(Lexer::RBRACE, "预期'}'结束函数体");
        return func;
    }

    shared_ptr<FunctionCall> parseCall() {
        auto call = make_shared<FunctionCall>();
        call->name = consume(Lexer::IDENT, "预期函数名").value;
        consume(Lexer::LPAREN, "预期'('");

        while (!match(Lexer::RPAREN)) {
            stringstream argExpr;
            while (!match(Lexer::RPAREN) && !match(Lexer::COMMA) && !match(Lexer::END)) {
                auto token = consume();
                argExpr << token.value;
            }

            string arg = argExpr.str();
            if (!arg.empty()) {
                call->args.push_back(arg);
            }

            if (match(Lexer::COMMA)) {
                consume(Lexer::COMMA, "预期','分隔参数");
            }
        }
        consume(Lexer::RPAREN, "预期')'");
        return call;
    }

    shared_ptr<VariableAssign> parseVariableAssign() {
        auto assign = make_shared<VariableAssign>();
        consume(Lexer::VAR_KW, "预期'变量'关键字");
        assign->name = consume(Lexer::IDENT, "预期变量名").value;
        consume(Lexer::ASSIGN, "预期'='");

        stringstream expr;
        while (!match(Lexer::SEMI) && !match(Lexer::END)) {
            auto token = consume();
            expr << token.value;
            if (match(Lexer::SEMI)) break;
        }
        assign->value = expr.str();

        if (match(Lexer::SEMI)) {
            consume(Lexer::SEMI, "预期';'");
        }

        return assign;
    }

    shared_ptr<IfStatement> parseIfStatement() {
        auto ifStmt = make_shared<IfStatement>();
        consume(Lexer::IF_KW, "预期'如果'");

        stringstream condition;
        while (!match(Lexer::THEN_KW) && !match(Lexer::END)) {
            auto token = consume();
            condition << token.value;
        }
        ifStmt->condition = condition.str();

        consume(Lexer::THEN_KW, "预期'那么'");
        consume(Lexer::LBRACE, "预期'{'开始那么分支");

        while (!match(Lexer::RBRACE)) {
            if (match(Lexer::IF_KW)) {
                ifStmt->thenBranch.push_back(parseIfStatement());
            }
            else if (match(Lexer::VAR_KW)) {
                ifStmt->thenBranch.push_back(parseVariableAssign());
            }
            else if (checkIdentWithLParen()) {
                ifStmt->thenBranch.push_back(parseCall());
            }
            else {
                consume();
            }
        }
        consume(Lexer::RBRACE, "预期'}'结束那么分支");

        if (match(Lexer::ELSE_KW)) {
            consume(Lexer::ELSE_KW, "预期'否则'");

            if (match(Lexer::IF_KW)) {
                ifStmt->elseBranch.push_back(parseIfStatement());
            }
            else {
                consume(Lexer::LBRACE, "预期'{'开始否则分支");
                while (!match(Lexer::RBRACE)) {
                    if (match(Lexer::IF_KW)) {
                        ifStmt->elseBranch.push_back(parseIfStatement());
                    }
                    else if (match(Lexer::VAR_KW)) {
                        ifStmt->elseBranch.push_back(parseVariableAssign());
                    }
                    else if (checkIdentWithLParen()) {
                        ifStmt->elseBranch.push_back(parseCall());
                    }
                    else {
                        consume();
                    }
                }
                consume(Lexer::RBRACE, "预期'}'结束否则分支");
            }
        }

        return ifStmt;
    }

    vector<Lexer::Token> tokens;
    size_t pos;
};

// REPL环境
void repl() {
    initConsoleEncoding();
    global_username = getUsernameFlexible();

    auto showWelcome = [&]() {
        cout << "———————————————————————————————————————————————————————————————————————————" << endl;
        cout << "Luxaohi 中文编程语言解释器[作者：ryun、Alex、TRAE、deep seek、豆包] " << endl;
        cout << "Luxaohi 中文编程语言解释器[使用的编程器：TRAE、Visual Studio 2022]" << endl;
        cout << "Luxaohi 中文编程语言解释器[贡献者：暂无]" << endl;
        cout << "Luxaohi 中文编程语言解释器[使用的语言：C++·100%]" << endl;
        cout << "Luxaohi 中文编程语言解释器[使用的角色：罗小黑、Niko]" << endl;
        cout << "罗小黑[" << global_username + "，我叫小黑，很高兴认识你喵~]" << endl;
        cout << "Niko[" << global_username + "？]" << endl;
        cout << "———————————————————————————————————————————————————————————————————————————" << endl;
        cout << "提示：" << endl;
        cout << "输入\"叫我 昵称\"可以更改称呼" << endl;
        cout << "输入\"清理\"可以清屏" << endl;
        cout << "输入\"睡觉\"可以退出" << endl;
        cout << "———————————————————————————————————————————————————————————————————————————" << endl;
        };

    showWelcome();

    initTextToSpeech();

    Interpreter interpreter;
    while (true) {
        cout << "(>ω<) ";
        string line;
        getline(cin, line);

        if (line == "睡觉") {
            cout << "罗小黑[" << global_username + "，再见喵~]" << endl;
            cleanupZeroMQ();
            break;
        }
        else if (line.find("叫我") == 0) {
            string nickname = line.substr(6);

            bool all_space = true;
            for (char c : nickname) {
                if (c != ' ') {
                    all_space = false;
                    break;
                }
            }

            if (nickname.empty() || all_space) {
                cout << "罗小黑[不能用空格！要不然我和Niko怎么叫你喵~]" << endl;
            }
            else {
                size_t start = nickname.find_first_not_of(" ");
                size_t end = nickname.find_last_not_of(" ");
                if (start != string::npos && end != string::npos) {
                    nickname = nickname.substr(start, end - start + 1);
                }

                global_username = nickname;
                cout << "罗小黑[好的，那以后就叫你" << global_username + "了喵~]" << endl;
                cout << "Niko[看来先知机器人要更新数据库了。]" << endl;
            }
            continue;
        }
        else if (line == "清理") {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            showWelcome();
            continue;
        }

        try {
            Lexer lexer(line);
            Parser parser(lexer.tokenize());
            interpreter.execute(parser.parse());
        }
        catch (const exception& e) {
            string error_msg = e.what();
            cout << error_msg << endl;

            if (error_msg.find("罗小黑[") != string::npos) {
                speakError(error_msg);
            }
        }
    }
}

int main() {
    repl();
    return 0;
}