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
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#pragma comment(lib, "shell32.lib") 
#pragma comment(lib, "shlwapi.lib")
#endif
#include <fstream>
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
        size_t start_pos = error_message.find("刘小黑[");
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

// 获取Windows系统上的用户名
std::string getUsernameFlexible() {
#ifdef _WIN32
    // 尝试从环境变量获取
    std::vector<std::string> tryMethods = { "USERNAME", "USER", "USERPROFILE" };
    for (const auto& envVar : tryMethods) {
        const char* value = std::getenv(envVar.c_str());
        if (value && value[0] != '\0') {
            std::string username = value;
            // 处理可能的路径（如USERPROFILE可能返回C:\Users\用户名）
            size_t lastSlash = username.find_last_of("\\/");
            if (lastSlash != std::string::npos && lastSlash + 1 < username.length()) {
                username = username.substr(lastSlash + 1);
            }
            return username;
        }
    }

    // 如果环境变量不可用，尝试Windows API
    DWORD size = 0;
    GetUserNameA(nullptr, &size);
    if (size > 0) {
        std::vector<char> buffer(size);
        if (GetUserNameA(buffer.data(), &size)) {
            return std::string(buffer.data());
        }
    }
#endif
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
                string error_msg = "刘小黑[这是？参数数量不匹配？]: " + name;
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
            string error_msg = "刘小黑[好像是未定义的函数呢喵~]: " + name;
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
            string error_msg = "刘小黑[" + global_username + "，表达式解析不完整。]";
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
                    string error_msg = "刘小黑[" + global_username + "，数学运算需要数字参数]";
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
            if (op == '*') {
                pos++;
                string right = parsePrimary(expr, pos);

                if (!isInteger(left) || !isInteger(right)) {
                    string error_msg = "刘小黑[" + global_username + "，数学运算需要数字参数]";
                    throw runtime_error(error_msg);
                }

                int left_val = stoi(left);
                int right_val = stoi(right);
                int result = left_val * right_val;
                left = to_string(result);
            }
            else if (op == '/') {
                pos++;
                string right = parsePrimary(expr, pos);

                if (!isInteger(left) || !isInteger(right)) {
                    string error_msg = "刘小黑[" + global_username + "，数学运算需要数字参数喵]";
                    throw runtime_error(error_msg);
                }

                int left_val = stoi(left);
                int right_val = stoi(right);
                if (right_val == 0) {
                    string error_msg = "刘小黑[" + global_username + "，不能除以零的啦...]";
                    throw runtime_error(error_msg);
                }
                int result = left_val / right_val;
                left = to_string(result);
            }
            else {
                break;
            }
        }
        return left;
    }

    string parsePrimary(const string& expr, size_t& pos) {
        if (pos >= expr.size()) {
            string error_msg = "刘小黑[" + global_username + "，这个表达式好像不完整呢]";
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
            string error_msg = "刘小黑[" + global_username + "，这个是一个未定义的变量: " + var + "哎。]";
            throw runtime_error(error_msg);
        }

        if (expr[pos] == '(') {
            pos++;
            string result = parseAddition(expr, pos);
            if (pos >= expr.size() || expr[pos] != ')') {
                string error_msg = "刘小黑[" + global_username + "，括号不匹配喵。]";
                throw runtime_error(error_msg);
            }
            pos++;
            return result;
        }
        if (expr[pos] == '-') {
            // 检查负号后是否有数字
            if (pos + 1 < expr.size() && isdigit(expr[pos + 1])) {
                pos++;
                string num = "-";
                while (pos < expr.size() && isdigit(expr[pos])) {
                    num += expr[pos++];
                }
                return num;
            }
        }

        string error_msg = "刘小黑[" + global_username + "，这个表达式我看不懂: " + string(1, expr[pos]) + "]";
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
                    throw runtime_error("刘小黑[好像是未闭合的字符串呢]");
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
                throw runtime_error(string("刘小黑[喵，未知的字符: '") + string(1, c) + "' ...]");
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
            // 跳过分号（语句分隔符）
            if (match(Lexer::SEMI)) {
                consume();
                continue;
            }

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
                // 跳过意外的token
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
        // 跳过可能的分号
        size_t checkPos = pos;
        while (checkPos < tokens.size() && tokens[checkPos].type == Lexer::SEMI) {
            checkPos++;
        }

        return checkPos + 1 < tokens.size() &&
            tokens[checkPos].type == Lexer::IDENT &&
            tokens[checkPos + 1].type == Lexer::LPAREN;
    }

    Lexer::Token consume(Lexer::TokenType type, const string& err) {
        // 跳过可能的分号
        while (pos < tokens.size() && tokens[pos].type == Lexer::SEMI) {
            pos++;
        }

        if (match(type)) {
            return tokens[pos++];
        }
        throw runtime_error(err);
    }

    Lexer::Token consume() {
        if (pos < tokens.size()) {
            return tokens[pos++];
        }
        return Lexer::Token(Lexer::END, "", 0, 0);
    }

    shared_ptr<FunctionDecl> parseFunction() {
        auto func = make_shared<FunctionDecl>();
        consume(Lexer::FUNC_KW, "刘小黑[预期函数关键字'喵'呢~]");
        func->name = consume(Lexer::IDENT, "刘小黑[想要一个函数名可以嘛~]").value;
        consume(Lexer::LPAREN, "刘小黑[预期'('呢]");
        while (!match(Lexer::RPAREN)) {
            func->params.push_back(consume(Lexer::IDENT, "刘小黑[预期一个参数名...]").value);
            if (!match(Lexer::RPAREN)) {
                consume(Lexer::COMMA, "刘小黑[预期','用来分隔参数喵~]");
            }
        }
        consume(Lexer::RPAREN, "刘小黑[预期')'呢]");
        consume(Lexer::LBRACE, "刘小黑[预期'{'开始函数体呢]");
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
            else if (match(Lexer::SEMI)) {
                consume(); // 跳过函数体内的分号
            }
            else {
                consume(); // 跳过其他token
            }
        }
        consume(Lexer::RBRACE, "刘小黑[预期'}'结束函数体...]");
        return func;
    }

    shared_ptr<FunctionCall> parseCall() {
        auto call = make_shared<FunctionCall>();
        call->name = consume(Lexer::IDENT, "刘小黑[想要一个函数名可以嘛~]").value;
        consume(Lexer::LPAREN, "刘小黑[预期'('呢]");

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
                consume(Lexer::COMMA, "刘小黑[预期','用来分隔参数喵~]");
            }
        }
        consume(Lexer::RPAREN, "刘小黑[预期')'呢]");
        return call;
    }

    shared_ptr<VariableAssign> parseVariableAssign() {
        auto assign = make_shared<VariableAssign>();
        consume(Lexer::VAR_KW, "刘小黑[预期'变量'关键字喵~]");
        assign->name = consume(Lexer::IDENT, "刘小黑[预期一个变量名~]").value;
        consume(Lexer::ASSIGN, "刘小黑[预期'='呢]");

        stringstream expr;
        // 读取表达式直到分号或文件结束
        while (!match(Lexer::SEMI) && !match(Lexer::END) &&
            !match(Lexer::VAR_KW) && !match(Lexer::IF_KW) &&
            !match(Lexer::FUNC_KW) && !checkIdentWithLParen()) {
            auto token = consume();
            expr << token.value;
        }
        assign->value = expr.str();

        // 如果当前是分号，跳过它
        if (match(Lexer::SEMI)) {
            consume(Lexer::SEMI, "刘小黑[预期';'喵]");
        }

        return assign;
    }

    shared_ptr<IfStatement> parseIfStatement() {
        auto ifStmt = make_shared<IfStatement>();
        consume(Lexer::IF_KW, "刘小黑[预期'如果'...]");

        stringstream condition;
        while (!match(Lexer::THEN_KW) && !match(Lexer::END)) {
            auto token = consume();
            condition << token.value;
        }
        ifStmt->condition = condition.str();

        consume(Lexer::THEN_KW, "刘小黑[预期'那么'分支...]");
        consume(Lexer::LBRACE, "刘小黑[" + global_username + "，那么后面需要加上一个'{'来开始代码块喵~]");

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
            else if (match(Lexer::SEMI)) {
                consume(); // 跳过分号
            }
            else {
                consume(); // 跳过其他token
            }
        }
        consume(Lexer::RBRACE, "刘小黑[" + global_username + "，那么结束的时候需要加上一个'}'来结束代码块喵~]");

        if (match(Lexer::ELSE_KW)) {
            consume(Lexer::ELSE_KW, "[哈哈哈，我是占位]");

            if (match(Lexer::IF_KW)) {
                ifStmt->elseBranch.push_back(parseIfStatement());
            }
            else {
                consume(Lexer::LBRACE, "刘小黑[" + global_username + "，否则后面需要加上一个'{'来开始代码块哦]");
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
                    else if (match(Lexer::SEMI)) {
                        consume(); // 跳过分号
                    }
                    else {
                        consume(); // 跳过其他token
                    }
                }
                consume(Lexer::RBRACE, "刘小黑[" + global_username + "，否则分支缺少结束用的'}'喵~]");
            }
        }

        return ifStmt;
    }

    vector<Lexer::Token> tokens;
    size_t pos;
};

// 文件读取和执行功能
void executeFromFile(const string& filePath, Interpreter& interpreter) {
    // 读取整个文件内容
    ifstream file(filePath, ios::in | ios::binary);

    if (!file.is_open()) {
#ifdef _WIN32
        // Windows备选方案
        HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER fileSize;
            if (GetFileSizeEx(hFile, &fileSize)) {
                if (fileSize.QuadPart > 0 && fileSize.QuadPart <= 10 * 1024 * 1024) {
                    vector<char> buffer(static_cast<size_t>(fileSize.QuadPart) + 1);
                    DWORD bytesRead;

                    if (ReadFile(hFile, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, NULL)) {
                        buffer[bytesRead] = '\0';
                        CloseHandle(hFile);

                        string code(buffer.data(), bytesRead);

                        try {
                            Lexer lexer(code);
                            Parser parser(lexer.tokenize());
                            interpreter.execute(parser.parse());
                            cout << "刘小黑[文件 " + filePath + " 执行成功喵~]" << endl;
                        }
                        catch (const exception& e) {
                            throw e;
                        }
                        return;
                    }
                }
            }
            CloseHandle(hFile);
        }
#endif
        throw runtime_error("刘小黑[喵呜，无法打开文件 " + filePath + " ]");
    }

    // 读取整个文件
    stringstream buffer;
    buffer << file.rdbuf();
    string code = buffer.str();
    file.close();

    if (code.empty()) {
        throw runtime_error("刘小黑[文件 " + filePath + " 是空的呢喵~]");
    }

    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        interpreter.execute(parser.parse());
        cout << "刘小黑[文件 " + filePath + " 运行成功啦]" << endl;
    }
    catch (const exception& e) {
        throw e;
    }
}

// REPL环境
void repl() {
    initConsoleEncoding();
    global_username = getUsernameFlexible();

    auto showWelcome = [&]() {
        cout << "———————————————————————————————————————————————————————————————————————————" << endl;
        cout << "Luxaohi 中文编程语言解释器[作者：ryun、deep seek] " << endl;
        cout << "Luxaohi 中文编程语言解释器[使用的编程器：Visual Studio 2026]" << endl;
        cout << "Luxaohi 中文编程语言解释器[贡献者：暂无]" << endl;
        cout << "刘小黑[你好 " << global_username + "，我叫刘小黑，很高兴认识你喵~]" << endl;
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

        // 去除首尾空格
        size_t start = line.find_first_not_of(" \t");
        if (start == string::npos) {
            continue; // 空行
        }
        size_t end = line.find_last_not_of(" \t");
        line = line.substr(start, end - start + 1);

        if (line == "睡觉") {
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
                cout << "刘小黑[不能用空格！要不然我怎么叫你喵~]" << endl;
            }
            else {
                size_t start = nickname.find_first_not_of(" ");
                size_t end = nickname.find_last_not_of(" ");
                if (start != string::npos && end != string::npos) {
                    nickname = nickname.substr(start, end - start + 1);
                }

                global_username = nickname;
                cout << "刘小黑[好的，那以后就叫你" << global_username + "了喵~]" << endl;
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
        else if (line.find("运行") == 0) {
            // 提取文件路径
            string filePath;

            // 检查是否有引号包围
            size_t quotePos = line.find('"', 2); // 在"运行"之后找引号
            if (quotePos != string::npos) {
                size_t endQuotePos = line.find('"', quotePos + 1);
                if (endQuotePos != string::npos) {
                    filePath = line.substr(quotePos + 1, endQuotePos - quotePos - 1);
                }
            }

            // 如果没有引号，使用传统方式
            if (filePath.empty()) {
                filePath = line.substr(4);

                // 去除路径首尾空格
                size_t pathStart = filePath.find_first_not_of(" \t");
                if (pathStart == string::npos) {
                    cout << "刘小黑[可以告诉我一下文件路径嘛喵~]" << endl;
                    continue;
                }
                size_t pathEnd = filePath.find_last_not_of(" \t");
                filePath = filePath.substr(pathStart, pathEnd - pathStart + 1);

                // 检查是否还有引号
                if (filePath.front() == '"' && filePath.back() == '"') {
                    filePath = filePath.substr(1, filePath.length() - 2);
                }
                else if (filePath.front() == '\'' && filePath.back() == '\'') {
                    filePath = filePath.substr(1, filePath.length() - 2);
                }
            }

            if (filePath.empty()) {
                cout << "刘小黑[可以告诉我一下文件路径嘛喵~]" << endl;
                continue;
            }

            try {
                executeFromFile(filePath, interpreter);
            }
            catch (const exception& e) {
                string error_msg = e.what();
                cout << error_msg << endl;
                if (error_msg.find("刘小黑[") != string::npos) {
                    speakError(error_msg);
                }
            }
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

            if (error_msg.find("刘小黑[") != string::npos) {
                speakError(error_msg);
            }
        }
    }
}

int main() {
    repl();
    return 0;
}