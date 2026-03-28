#define _CRT_SECURE_NO_WARNINGS
#include "词法分析器.h"
#include <stdexcept>
#include <cctype>
#include <algorithm>

static void 替换所有(std::string& s, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}

static std::string 规范化标点(const std::string& 原) {
    std::string s = 原;
    // 常见全角/中文标点 -> ASCII 映射
    替换所有(s, "（", "(");
    替换所有(s, "）", ")");
    替换所有(s, "，", ",");
    替换所有(s, "；", ";");
    替换所有(s, "：", ":");
    替换所有(s, "【", "[");
    替换所有(s, "】", "]");
    替换所有(s, "＝", "=");
    替换所有(s, "＋", "+");
    替换所有(s, "－", "-");
    替换所有(s, "＊", "*");
    替换所有(s, "／", "/");
    替换所有(s, "＃", "#");
    // 中文引号 -> ASCII 引号
    替换所有(s, "“", "\"");
    替换所有(s, "”", "\"");
    替换所有(s, "‘", "\'");
    替换所有(s, "’", "\'");
    return s;
}

词法分析器::令牌::令牌(令牌类型 t, std::string v, int l, int c)
    : 类型(t), 值(std::move(v)), 行号(l), 列号(c) {
}

词法分析器::词法分析器(const std::string& 代码)
    : 代码(), 位置(0), 行号(1), 列号(1) {
    this->代码 = 规范化标点(代码);
}

std::vector<词法分析器::令牌> 词法分析器::分析() {
    std::vector<令牌> 令牌列表;
    while (位置 < 代码.size()) {
        unsigned char uc = static_cast<unsigned char>(代码[位置]);

        // 空白
        if (isspace(uc)) {
            if (代码[位置] == '\n') {
                令牌列表.emplace_back(换行符, "\n", 行号, 列号);
                行号++;
                列号 = 1;
            }
            else {
                列号++;
            }
            位置++;
            continue;
        }

        // 注释支持：单行 '#' 和多行 '#* ... *#'
        if (代码[位置] == '#') {
            if (位置 + 1 < 代码.size() && 代码[位置 + 1] == '*') {
                // 多行注释
                位置 += 2; 列号 += 2;
                bool 已闭合 = false;
                while (位置 < 代码.size()) {
                    if (代码[位置] == '\n') { 行号++; 列号 = 1; 位置++; continue; }
                    if (位置 + 1 < 代码.size() && 代码[位置] == '*' && 代码[位置 + 1] == '#') {
                        位置 += 2; 列号 += 2; 已闭合 = true; break;
                    }
                    位置++; 列号++;
                }
                if (!已闭合) throw std::runtime_error("刘小黑[注释未闭合呢喵~]");
                continue;
            } else {
                // 单行注释
                位置++; 列号++;
                while (位置 < 代码.size() && 代码[位置] != '\n') { 位置++; 列号++; }
                continue;
            }
        }

        // 标识符 / 关键字
        if (是字母字符(代码[位置])) {
            int 起始列 = 列号;
            std::string 值;
            while (位置 < 代码.size() && 是字母数字字符(代码[位置])) {
                值 += 代码[位置++]; 列号++;
            }
            if (值 == "喵") 令牌列表.emplace_back(函数关键字, 值, 行号, 起始列);
            else if (值 == "如果") 令牌列表.emplace_back(如果关键字, 值, 行号, 起始列);
            else if (值 == "那么") 令牌列表.emplace_back(那么关键字, 值, 行号, 起始列);
            else if (值 == "否则") 令牌列表.emplace_back(否则关键字, 值, 行号, 起始列);
            else if (值 == "或者") 令牌列表.emplace_back(或者关键字, 值, 行号, 起始列);
            else if (值 == "变量") 令牌列表.emplace_back(变量关键字, 值, 行号, 起始列);
            else if (值 == "返回") 令牌列表.emplace_back(返回关键字, 值, 行号, 起始列);
            else if (值 == "当") 令牌列表.emplace_back(当关键字, 值, 行号, 起始列);
            else if (值 == "数组") 令牌列表.emplace_back(数组关键字, 值, 行号, 起始列);
            else 令牌列表.emplace_back(标识符, 值, 行号, 起始列);
            continue;
        }

        // 数字（支持整数与小数）
        if (isdigit(static_cast<unsigned char>(代码[位置]))) {
            int 起始列 = 列号;
            std::string 值;
            bool 有小数点 = false;
            while (位置 < 代码.size()) {
                char c = 代码[位置];
                if (c == '.') {
                    if (有小数点) break;
                    有小数点 = true;
                    值 += 代码[位置++]; 列号++;
                } else if (isdigit(static_cast<unsigned char>(c))) {
                    值 += 代码[位置++]; 列号++;
                } else break;
            }
            令牌列表.emplace_back(数字, 值, 行号, 起始列);
            continue;
        }

        // 字符串
        if (代码[位置] == '"' || 代码[位置] == '\'') {
            char 引号 = 代码[位置];
            int 起始列 = 列号;
            std::string 值;
            位置++; 列号++;
            while (位置 < 代码.size() && 代码[位置] != 引号) {
                if (代码[位置] == '\\') {
                    位置++; 列号++;
                    if (位置 >= 代码.size()) break;
                    switch (代码[位置]) {
                    case 'n': 值 += '\n'; break;
                    case 't': 值 += '\t'; break;
                    case '\\': 值 += '\\'; break;
                    case '"': 值 += '"'; break;
                    case '\'': 值 += '\''; break;
                    default: 值 += 代码[位置]; break;
                    }
                } else {
                    值 += 代码[位置];
                }
                位置++; 列号++;
            }
            if (位置 >= 代码.size()) throw std::runtime_error("刘小黑[好像是未闭合的字符串呢]");
            位置++; 列号++;
            令牌列表.emplace_back(字符串, 值, 行号, 起始列);
            continue;
        }

        // 双字符运算符
        if (代码[位置] == '=' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(等于, "==", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        if (代码[位置] == '!' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(不等于, "!=", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        if (代码[位置] == '+' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(加等, "+=", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        if (代码[位置] == '-' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(减等, "-=", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        if (代码[位置] == '*' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(乘等, "*=", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        if (代码[位置] == '/' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(除等, "/=", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        // 乘方 ** 和 **=
        if (代码[位置] == '*' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '*') {
            if (位置 + 2 < 代码.size() && 代码[位置 + 2] == '=') {
                令牌列表.emplace_back(乘方等, "**=", 行号, 列号); 位置 += 3; 列号 += 3; continue;
            } else {
                令牌列表.emplace_back(乘方, "**", 行号, 列号); 位置 += 2; 列号 += 2; continue;
            }
        }
        // 模运算 % 和 %=
        if (代码[位置] == '%' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '=') {
            令牌列表.emplace_back(模等, "%=", 行号, 列号); 位置 += 2; 列号 += 2; continue;
        }
        if (代码[位置] == '%') { 令牌列表.emplace_back(模, "%", 行号, 列号); 位置++; 列号++; continue; }
        // 整除 // 和 //=
        if (代码[位置] == '/' && 位置 + 1 < 代码.size() && 代码[位置 + 1] == '/') {
            if (位置 + 2 < 代码.size() && 代码[位置 + 2] == '=') {
                令牌列表.emplace_back(整除等, "//=", 行号, 列号); 位置 += 3; 列号 += 3; continue;
            } else {
                令牌列表.emplace_back(整除, "//", 行号, 列号); 位置 += 2; 列号 += 2; continue;
            }
        }

        // 单字符符号/运算符
        char 字符ASCII = 代码[位置];
        switch (字符ASCII) {
        case '=': 令牌列表.emplace_back(等号, "=", 行号, 列号); break;
        case '+': 令牌列表.emplace_back(加号, "+", 行号, 列号); break;
        case '-': 令牌列表.emplace_back(减号, "-", 行号, 列号); break;
        case '*': 令牌列表.emplace_back(乘号, "*", 行号, 列号); break;
        case '/': 令牌列表.emplace_back(除号, "/", 行号, 列号); break;
        case '>': 令牌列表.emplace_back(大于, ">", 行号, 列号); break;
        case '<': 令牌列表.emplace_back(小于, "<", 行号, 列号); break;
        case '[': 令牌列表.emplace_back(左中括号, "[", 行号, 列号); break;
        case ']': 令牌列表.emplace_back(右中括号, "]", 行号, 列号); break;
        case '(': 令牌列表.emplace_back(左括号, "(", 行号, 列号); break;
        case ')': 令牌列表.emplace_back(右括号, ")", 行号, 列号); break;
        case '{': 令牌列表.emplace_back(左花括号, "{", 行号, 列号); break;
        case '}': 令牌列表.emplace_back(右花括号, "}", 行号, 列号); break;
        case ',': 令牌列表.emplace_back(逗号, ",", 行号, 列号); break;
        case ';': 令牌列表.emplace_back(分号, ";", 行号, 列号); break;
        default:
            throw std::runtime_error(std::string("刘小黑[喵，未知的字符：'") + std::string(1, 字符ASCII) + "' ...]");
        }
        位置++; 列号++;
    }

    令牌列表.emplace_back(结束, "", 行号, 列号);
    return 令牌列表;
}