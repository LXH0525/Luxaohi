#define _CRT_SECURE_NO_WARNINGS
#include "词法分析器.h"
#include <stdexcept>
#include <cctype>

词法分析器::令牌::令牌(令牌类型 t, std::string v, int l, int c)
    : 类型(t), 值(std::move(v)), 行号(l), 列号(c) {
}

词法分析器::词法分析器(const std::string& 代码)
    : 代码(代码), 位置(0), 行号(1), 列号(1) {
}

std::vector<词法分析器::令牌> 词法分析器::分析() {
    std::vector<令牌> 令牌列表;
    while (位置 < 代码.size()) {
        char 字符 = 代码[位置];
        if (isspace(字符)) {
            if (字符 == '\n') { 行号++; 列号 = 1; }
            else { 列号++; }
            位置++;
            continue;
        }
        if (是字母字符(字符)) {
            std::string 值;
            while (位置 < 代码.size() && 是字母数字字符(代码[位置])) {
                值 += 代码[位置++]; 列号++;
            }
            if (值 == "喵") 令牌列表.emplace_back(函数关键字, 值, 行号, 列号 - 值.size());
            else if (值 == "如果") 令牌列表.emplace_back(如果关键字, 值, 行号, 列号 - 值.size());
            else if (值 == "那么") 令牌列表.emplace_back(那么关键字, 值, 行号, 列号 - 值.size());
            else if (值 == "否则") 令牌列表.emplace_back(否则关键字, 值, 行号, 列号 - 值.size());
            else if (值 == "变量") 令牌列表.emplace_back(变量关键字, 值, 行号, 列号 - 值.size());
            else 令牌列表.emplace_back(标识符, 值, 行号, 列号 - 值.size());
            continue;
        }
        if (isdigit(字符)) {
            std::string 值;
            while (位置 < 代码.size() && (isdigit(代码[位置]) || 代码[位置] == '.')) {
                值 += 代码[位置++]; 列号++;
            }
            令牌列表.emplace_back(数字, 值, 行号, 列号 - 值.size());
            continue;
        }
        if (字符 == '"' || 字符 == '\'') {
            char 引号 = 字符;
            std::string 值; 位置++; 列号++;
            while (位置 < 代码.size() && 代码[位置] != 引号) {
                if (代码[位置] == '\\') {
                    位置++; 列号++;
                    if (位置 >= 代码.size()) break;
                    switch (代码[位置]) {
                    case 'n': 值 += '\n'; break; case 't': 值 += '\t'; break;
                    case '\\': 值 += '\\'; break; case '"': 值 += '"'; break;
                    case '\'': 值 += '\''; break; default: 值 += 代码[位置]; break;
                    }
                }
                else 值 += 代码[位置];
                位置++; 列号++;
            }
            if (位置 >= 代码.size()) throw std::runtime_error("刘小黑[好像是未闭合的字符串呢]");
            位置++; 列号++;
            令牌列表.emplace_back(字符串, 值, 行号, 列号 - 值.size() - 2);
            continue;
        }
        if (字符 == '=' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(等于, "==", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        else if (字符 == '!' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(不等于, "!=", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        else if (字符 == '=') {
            令牌列表.emplace_back(等号, "=", 行号, 列号); 位置++; 列号++; continue;
        }
        else if (字符 == '+') {
            令牌列表.emplace_back(加号, "+", 行号, 列号); 位置++; 列号++; continue;
        }
        else if (字符 == '-') {
            令牌列表.emplace_back(减号, "-", 行号, 列号); 位置++; 列号++; continue;
        }
        else if (字符 == '*') {
            令牌列表.emplace_back(乘号, "*", 行号, 列号); 位置++; 列号++; continue;
        }
        else if (字符 == '/') {
            令牌列表.emplace_back(除号, "/", 行号, 列号); 位置++; 列号++; continue;
        }
        else if (字符 == '>') {
            令牌列表.emplace_back(大于, ">", 行号, 列号); 位置++; 列号++; continue;
        }
        else if (字符 == '<') {
            令牌列表.emplace_back(小于, "<", 行号, 列号); 位置++; 列号++; continue;
        }
        switch (字符) {
        case '(': 令牌列表.emplace_back(左括号, "(", 行号, 列号); break;
        case ')': 令牌列表.emplace_back(右括号, ")", 行号, 列号); break;
        case '{': 令牌列表.emplace_back(左花括号, "{", 行号, 列号); break;
        case '}': 令牌列表.emplace_back(右花括号, "}", 行号, 列号); break;
        case ',': 令牌列表.emplace_back(逗号, ",", 行号, 列号); break;
        case ';': 令牌列表.emplace_back(分号, ";", 行号, 列号); break;
        default: throw std::runtime_error(std::string("刘小黑[喵，未知的字符: '") + std::string(1, 字符) + "' ...]");
        }
        位置++; 列号++;
    }
    令牌列表.emplace_back(结束, "", 行号, 列号);
    return 令牌列表;
}