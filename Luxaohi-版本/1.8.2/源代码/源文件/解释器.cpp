#define _CRT_SECURE_NO_WARNINGS
#include "解释器.h"
#include "语法树.h"
#include "工具.h"
#include <iostream>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <algorithm>

void 解释器::添加函数(const std::string& 名称, const 函数& 函数) {
    函数表[名称] = 函数;
}

void 解释器::执行(const std::vector<std::shared_ptr<语法树节点>>& 节点列表) {
    for (auto& 节点 : 节点列表) 节点->执行(*this);
}

void 解释器::调用函数(const std::string& 名称, const std::vector<std::string>& 实参列表) {
    if (函数表.find(名称) != 函数表.end()) {
        const auto& 函数 = 函数表[名称];
        if (实参列表.size() != 函数.参数列表.size()) {
            throw std::runtime_error("刘小黑[这是？参数数量不匹配？]: " + 名称);
        }
        for (const auto& 节点 : 函数.函数体) 节点->执行(*this);
    }
    else if (名称 == "喵叫") {
        for (const auto& 参数 : 实参列表) std::cout << 求值(参数);
        std::cout << std::endl;
    }
    else {
        throw std::runtime_error("刘小黑[好像是未定义的函数呢喵~]: " + 名称);
    }
}

std::string 解释器::求值(const std::string& 表达式) {
    if (变量表.find(表达式) != 变量表.end()) return 变量表[表达式];
    if (表达式.find_first_of("+-*/()") != std::string::npos) return 求值表达式(表达式);
    return 表达式;
}

bool 解释器::求值条件(const std::string& 条件) {
    std::string 条件副本 = 条件;
    size_t 等于位置 = 条件副本.find("==");
    size_t 不等位置 = 条件副本.find("!=");
    size_t 大于位置 = 条件副本.find(">");
    size_t 小于位置 = 条件副本.find("<");
    size_t 大等位置 = 条件副本.find(">=");
    size_t 小等位置 = 条件副本.find("<=");
    if (等于位置 != std::string::npos) {
        std::string 左边 = 求值(条件副本.substr(0, 等于位置));
        std::string 右边 = 求值(条件副本.substr(等于位置 + 2));
        return 左边 == 右边;
    }
    else if (不等位置 != std::string::npos) {
        std::string 左边 = 求值(条件副本.substr(0, 不等位置));
        std::string 右边 = 求值(条件副本.substr(不等位置 + 2));
        return 左边 != 右边;
    }
    else if (大等位置 != std::string::npos) {
        std::string 左字符串 = 求值(条件副本.substr(0, 大等位置));
        std::string 右字符串 = 求值(条件副本.substr(大等位置 + 2));
        int 左边 = std::stoi(左字符串); int 右边 = std::stoi(右字符串);
        return 左边 >= 右边;
    }
    else if (小等位置 != std::string::npos) {
        std::string 左字符串 = 求值(条件副本.substr(0, 小等位置));
        std::string 右字符串 = 求值(条件副本.substr(小等位置 + 2));
        int 左边 = std::stoi(左字符串); int 右边 = std::stoi(右字符串);
        return 左边 <= 右边;
    }
    else if (大于位置 != std::string::npos) {
        std::string 左字符串 = 求值(条件副本.substr(0, 大于位置));
        std::string 右字符串 = 求值(条件副本.substr(大于位置 + 1));
        int 左边 = std::stoi(左字符串); int 右边 = std::stoi(右字符串);
        return 左边 > 右边;
    }
    else if (小于位置 != std::string::npos) {
        std::string 左字符串 = 求值(条件副本.substr(0, 小于位置));
        std::string 右字符串 = 求值(条件副本.substr(小于位置 + 1));
        int 左边 = std::stoi(左字符串); int 右边 = std::stoi(右字符串);
        return 左边 < 右边;
    }
    std::string 值 = 求值(条件副本);
    return !值.empty() && 值 != "0" && 值 != "假";
}

bool 解释器::是整数(const std::string& 字符串) {
    if (字符串.empty()) return false;
    size_t 开始 = (字符串[0] == '-') ? 1 : 0;
    if (开始 >= 字符串.size()) return false;
    for (size_t i = 开始; i < 字符串.size(); i++) {
        if (!isdigit(字符串[i])) return false;
    }
    return true;
}

std::string 解释器::求值表达式(const std::string& 表达式) {
    std::string 清理表达式;
    for (char c : 表达式) if (c != ' ') 清理表达式 += c;
    size_t 位置 = 0;
    std::string 结果 = 解析加法(清理表达式, 位置);
    if (位置 != 清理表达式.size()) {
        throw std::runtime_error("刘小黑[" + 全局用户名 + "，表达式解析不完整。]");
    }
    return 结果;
}

std::string 解释器::解析加法(const std::string& 表达式, size_t& 位置) {
    std::string 左边 = 解析乘法(表达式, 位置);
    while (位置 < 表达式.size()) {
        char 操作符 = 表达式[位置];
        if (操作符 == '+' || 操作符 == '-') {
            位置++;
            std::string 右边 = 解析乘法(表达式, 位置);
            if (!是整数(左边) || !是整数(右边)) {
                throw std::runtime_error("刘小黑[" + 全局用户名 + "，数学运算需要数字参数]");
            }
            int 左值 = std::stoi(左边); int 右值 = std::stoi(右边);
            int 结果 = (操作符 == '+') ? 左值 + 右值 : 左值 - 右值;
            左边 = std::to_string(结果);
        }
        else break;
    }
    return 左边;
}

std::string 解释器::解析乘法(const std::string& 表达式, size_t& 位置) {
    std::string 左边 = 解析基本项(表达式, 位置);
    while (位置 < 表达式.size()) {
        char 操作符 = 表达式[位置];
        if (操作符 == '*') {
            位置++; std::string 右边 = 解析基本项(表达式, 位置);
            if (!是整数(左边) || !是整数(右边)) {
                throw std::runtime_error("刘小黑[" + 全局用户名 + "，数学运算需要数字参数]");
            }
            int 左值 = std::stoi(左边); int 右值 = std::stoi(右边);
            左边 = std::to_string(左值 * 右值);
        }
        else if (操作符 == '/') {
            位置++; std::string 右边 = 解析基本项(表达式, 位置);
            if (!是整数(左边) || !是整数(右边)) {
                throw std::runtime_error("刘小黑[" + 全局用户名 + "，数学运算需要数字参数喵]");
            }
            int 左值 = std::stoi(左边); int 右值 = std::stoi(右边);
            if (右值 == 0) throw std::runtime_error("刘小黑[" + 全局用户名 + "，不能除以零的啦...]");
            左边 = std::to_string(左值 / 右值);
        }
        else break;
    }
    return 左边;
}

std::string 解释器::解析基本项(const std::string& 表达式, size_t& 位置) {
    if (位置 >= 表达式.size()) throw std::runtime_error("刘小黑[" + 全局用户名 + "，这个表达式好像不完整呢]");
    if (isdigit(表达式[位置])) {
        std::string 数字;
        while (位置 < 表达式.size() && isdigit(表达式[位置])) 数字 += 表达式[位置++];
        return 数字;
    }
    if (::是字母字符(表达式[位置])) {
        std::string 变量;
        while (位置 < 表达式.size() && ::是字母数字字符(表达式[位置])) 变量 += 表达式[位置++];
        if (变量表.find(变量) != 变量表.end()) return 变量表[变量];
        throw std::runtime_error("刘小黑[" + 全局用户名 + "，这个是一个未定义的变量: " + 变量 + "哎。]");
    }
    if (表达式[位置] == '(') {
        位置++; std::string 结果 = 解析加法(表达式, 位置);
        if (位置 >= 表达式.size() || 表达式[位置] != ')') throw std::runtime_error("刘小黑[" + 全局用户名 + "，括号不匹配喵。]");
        位置++; return 结果;
    }
    if (表达式[位置] == '-' && 位置 + 1 < 表达式.size() && isdigit(表达式[位置 + 1])) {
        位置++; std::string 数字 = "-";
        while (位置 < 表达式.size() && isdigit(表达式[位置])) 数字 += 表达式[位置++];
        return 数字;
    }
    throw std::runtime_error("刘小黑[" + 全局用户名 + "，这个表达式我看不懂: " + std::string(1, 表达式[位置]) + "]");
}