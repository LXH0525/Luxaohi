#define _CRT_SECURE_NO_WARNINGS
#include "解释器.h"
#include "语法树.h"
#include "工具.h"
#include "高精度数学运算.h"
#include <iostream>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <set>

void 解释器::添加函数(const std::string& 名称, const 函数& 函数) {
    函数表[名称] = 函数;
}
void 解释器::执行(const std::vector<std::shared_ptr<语法树节点>>& 节点列表) {
    for (auto& 节点 : 节点列表) {
        节点->执行(*this);
    }
}

// 返回字符串为函数的返回值
std::string 解释器::调用函数(const std::string& 名称, const std::vector<std::string>& 实参列表) {
    if (函数表.find(名称) != 函数表.end()) {
        const auto& 函数 = 函数表[名称];
        if (实参列表.size() != 函数.参数列表.size()) {
            throw std::runtime_error("刘小黑[这是？参数数量不匹配？]: " + 名称);
        }

        // 绑定参数到变量表
        std::map<std::string, std::string> 旧值;
        std::set<std::string> 存在旧值;
        for (size_t i = 0; i < 函数.参数列表.size(); ++i) {
            const std::string& pname = 函数.参数列表[i];
            std::string pvalue = 求值(实参列表[i]);
            if (变量表.find(pname) != 变量表.end()) {
                旧值[pname] = 变量表[pname];
                存在旧值.insert(pname);
            }
            变量表[pname] = pvalue;
        }

        try {
            for (const auto& 节点 : 函数.函数体) 节点->执行(*this);
        }
        catch (const 返回异常& re) {
            // 恢复参数旧值
            for (const auto& kv : 函数.参数列表) {
                if (存在旧值.find(kv) != 存在旧值.end()) 变量表[kv] = 旧值[kv];
                else 变量表.erase(kv);
            }
            return re.值;
        }

        // 正常结束
        for (const auto& kv : 函数.参数列表) {
            if (存在旧值.find(kv) != 存在旧值.end()) 变量表[kv] = 旧值[kv];
            else 变量表.erase(kv);
        }
        return std::string();
    }
    else if (名称 == "喵叫") {
        for (const auto& 参数 : 实参列表) {
            std::string v = 求值(参数);
            // 如果是数组名且无索引，则打印整个数组
            if (数组表.find(参数) != 数组表.end()) {
                for (const auto& e : 数组表[参数]) std::cout << e;
            } else std::cout << v;
        }
        std::cout << std::endl;
        return std::string();
    }
    else if (名称 == "输入") {
        // 类似 Python 的 input：可选的提示参数（多个参数按空格连接），不换行，读取一行并返回
        if (!实参列表.empty()) {
        for (size_t i = 0; i < 实参列表.size(); ++i) {
            if (i) std::cout << ' ';
            std::cout << 求值(实参列表[i]);
        }
            // 不输出换行，保持在同一行等待输入
            std::cout.flush();
        }
        std::string 输入行;
        if (!std::getline(std::cin, 输入行)) {
            // 如果读取失败，返回空字符串
        return std::string();
    }
        return 输入行;
    }
    else {
        throw std::runtime_error("刘小黑[好像是未定义的函数呢喵~ ]: " + 名称);
    }
}

// 获取数组元素（若索引或元素不存在则抛出）
std::string 解释器::getArrayElement(const std::string& 名, const std::string& 索引表达式) {
    if (数组表.find(名) == 数组表.end()) throw std::runtime_error("刘小黑[有一个未定义的数组唔]: " + 名);
    std::string idxs = 求值(索引表达式);
    if (!是数字(idxs)) throw std::runtime_error("刘小黑[数组的索引需要是数字呢]: " + idxs);
    int idx = std::stoi(idxs);
    if (idx < 0 || static_cast<size_t>(idx) >= 数组表[名].size()) throw std::runtime_error("刘小黑[数组的索引越界啦]");
    return 数组表[名][idx];
}

void 解释器::setArrayElement(const std::string& 名, const std::string& 索引表达式, const std::string& 值表达式) {
    if (数组表.find(名) == 数组表.end()) throw std::runtime_error("刘小黑[有一个未定义的数组唔]: " + 名);
    std::string idxs = 求值(索引表达式);
    if (!是数字(idxs)) throw std::runtime_error("刘小黑[数组的索引需要是数字呢]: " + idxs);
    int idx = std::stoi(idxs);
    if (idx < 0 || static_cast<size_t>(idx) >= 数组表[名].size()) throw std::runtime_error("刘小黑[数组的索引越界啦]");
    数组表[名][idx] = 求值(值表达式);
}

// 判断字符串是否为整数或浮点数
bool 解释器::是数字(const std::string& s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '-') {
        if (s.size() == 1) return false;
        i = 1;
    }
    bool 有点 = false;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (有点) return false;
            有点 = true;
            continue;
        }
        if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

std::string 解释器::求值(const std::string& 表达式) {
    // 先做简单的首尾空格修剪
    auto trim = [](const std::string& str) -> std::string {
        size_t a = 0;
        while (a < str.size() && isspace(static_cast<unsigned char>(str[a]))) ++a;
        if (a == str.size()) return std::string();
        size_t b = str.size() - 1;
        while (b > a && isspace(static_cast<unsigned char>(str[b]))) --b;
        return str.substr(a, b - a + 1);
        };

    std::string expr = trim(表达式);

    if (变量表.find(expr) != 变量表.end()) {
        return 变量表[expr];
    }

    if (是数字(expr)) {
        return expr;
    }

    size_t firstParen = expr.find('(');
    if (firstParen != std::string::npos && firstParen > 0) {
        // 提取并修剪名字部分
        std::string namePart = trim(expr.substr(0, firstParen));
        bool nameValid = !namePart.empty();
        if (nameValid) {
            // 名称首字母需为字母类，其余为字母数字类
            if (!::是字母字符(namePart[0])) nameValid = false;
            for (size_t i = 1; i < namePart.size() && nameValid; ++i) {
                if (!::是字母数字字符(namePart[i])) nameValid = false;
            }
        }
        if (nameValid) {
            // 找到匹配的右括号
            int depth = 0;
            size_t matchPos = std::string::npos;
            for (size_t i = firstParen; i < expr.size(); ++i) {
                if (expr[i] == '(') ++depth;
                else if (expr[i] == ')') {
                    --depth;
                    if (depth == 0) { matchPos = i; break; }
                }
            }
            if (matchPos != std::string::npos && matchPos + 1 == expr.size()) {
                // 解析参数
                std::vector<std::string> args;
                std::string cur;
                depth = 0;
                for (size_t i = firstParen + 1; i < matchPos; ++i) {
                    char c = expr[i];
                    if (c == '(') { ++depth; cur += c; }
                    else if (c == ')') { --depth; cur += c; }
                    else if (c == ',' && depth == 0) {
                        args.push_back(trim(cur));
                        cur.clear();
                    }
                    else cur += c;
                }
                if (!cur.empty() || firstParen + 1 == matchPos) args.push_back(trim(cur));
                // 如果没有语法错误，直接调用函数并返回其返回值
                try {
                    return 调用函数(namePart, args);
                }
                catch (const 返回异常& re) {
                    // 函数正常返回其值
                    return re.值;
                }
            }
        }
    }

    // 支持数组访问语法 name[index] （当表达式为单独访问时）
    size_t firstBracket = expr.find('[');
    if (firstBracket != std::string::npos && firstBracket > 0) {
        std::string namePart = trim(expr.substr(0, firstBracket));
        bool nameValid = !namePart.empty();
        if (nameValid) {
            if (!::是字母字符(namePart[0])) nameValid = false;
            for (size_t i = 1; i < namePart.size() && nameValid; ++i) {
                if (!::是字母数字字符(namePart[i])) nameValid = false;
            }
        }
        if (nameValid) {
            int depth = 0;
            size_t matchPos = std::string::npos;
            for (size_t i = firstBracket; i < expr.size(); ++i) {
                if (expr[i] == '[') ++depth;
                else if (expr[i] == ']') {
                    --depth;
                    if (depth == 0) { matchPos = i; break; }
                }
            }
            if (matchPos != std::string::npos && matchPos + 1 == expr.size()) {
                std::string inside = expr.substr(firstBracket + 1, matchPos - firstBracket - 1);
                return this->getArrayElement(namePart, inside);
            }
        }
    }

    if (expr.find_first_of("+-*/%()[]") != std::string::npos) {
        bool 可解析为表达式 = true;
        for (char c : expr) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (!isdigit(uc) && !::是字母字符(c) &&
                c != '+' && c != '-' && c != '*' && c != '/' &&
                c != '%' &&
                c != '(' && c != ')' && c != '[' && c != ']' && c != ' ' && c != '=' && c != '!' && c != '>' && c != '<' && c != '.') {
                可解析为表达式 = false;
                break;
            }
        }

        if (可解析为表达式) {
            std::string 清理表达式;
            for (char c : expr) if (c != ' ') 清理表达式 += c;

            if (清理表达式.find("==") != std::string::npos ||
                清理表达式.find("!=") != std::string::npos ||
                清理表达式.find(">=") != std::string::npos ||
                清理表达式.find("<=") != std::string::npos ||
                清理表达式.find(">") != std::string::npos ||
                清理表达式.find("<") != std::string::npos) {
                bool 条件结果 = 求值条件(清理表达式);
                return 条件结果 ? "真" : "假";
            }

            return 求值表达式(清理表达式);
        }
    }

    return expr;
}

bool 解释器::求值条件(const std::string& 条件) {
    std::string 条件副本 = 条件;
    size_t 等于位置 = 条件副本.find("==");
    size_t 不等位置 = 条件副本.find("!=");
    size_t 大等位置 = 条件副本.find(">=");
    size_t 小等位置 = 条件副本.find("<=");
    size_t 大于位置 = 条件副本.find(">");
    size_t 小於位置 = 条件副本.find("<");

    auto try_number = [&](const std::string& s, 数学高精度& out) -> bool {
        if (是数字(s)) { out = toHigh(s); return true; }
        std::string v = 求值(s);
        if (是数字(v)) { out = toHigh(v); return true; }
        return false;
        };

    if (等于位置 != std::string::npos) {
        std::string 左 = 求值(条件副本.substr(0, 等于位置));
        std::string 右 = 求值(条件副本.substr(等于位置 + 2));
        数学高精度 ld, rd;
        if (try_number(左, ld) && try_number(右, rd)) return ld == rd;
        return 左 == 右;
    }
    else if (不等位置 != std::string::npos) {
        std::string 左 = 求值(条件副本.substr(0, 不等位置));
        std::string 右 = 求值(条件副本.substr(不等位置 + 2));
        数学高精度 ld, rd;
        if (try_number(左, ld) && try_number(右, rd)) return ld != rd;
        return 左 != 右;
    }
    else if (大等位置 != std::string::npos) {
        std::string 左 = 求值(条件副本.substr(0, 大等位置));
        std::string 右 = 求值(条件副本.substr(大等位置 + 2));
        数学高精度 ld = toHigh(求值(左)), rd = toHigh(求值(右));
        return ld >= rd;
    }
    else if (小等位置 != std::string::npos) {
        std::string 左 = 求值(条件副本.substr(0, 小等位置));
        std::string 右 = 求值(条件副本.substr(小等位置 + 2));
        数学高精度 ld = toHigh(求值(左)), rd = toHigh(求值(右));
        return ld <= rd;
    }
    else if (大于位置 != std::string::npos) {
        std::string 左 = 求值(条件副本.substr(0, 大于位置));
        std::string 右 = 求值(条件副本.substr(大于位置 + 1));
        数学高精度 ld = toHigh(求值(左)), rd = toHigh(求值(右));
        return ld > rd;
    }
    else if (小於位置 != std::string::npos) {
        std::string 左 = 求值(条件副本.substr(0, 小於位置));
        std::string 右 = 求值(条件副本.substr(小於位置 + 1));
        数学高精度 ld = toHigh(求值(左)), rd = toHigh(求值(右));
        return ld < rd;
    }

    std::string 值 = 求值(条件副本);
    return !值.empty() && 值 != "0" && 值 != "假";
}

std::string 解释器::求值表达式(const std::string& 表达式) {
    std::string 清理表达式;
    for (char c : 表达式) if (c != ' ') 清理表达式 += c;
    size_t 位置 = 0;
    std::string 结果 = 解析加法(清理表达式, 位置);
    if (位置 != 清理表达式.size()) {
        throw std::runtime_error("刘小黑[表达式解析不完整啦...]");
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
            if (!是数字(左边) || !是数字(右边)) {
                throw std::runtime_error("刘小黑[数学运算需要数字参数啦...]");
            }
            数学高精度 左值 = toHigh(左边);
            数学高精度 右值 = toHigh(右边);
            数学高精度 结果;
            if (操作符 == '+') 结果 = 左值 + 右值;
            else 结果 = 左值 - 右值;
            左边 = 格式化数学高精度(结果);
        }
        else break;
    }
    return 左边;
}

std::string 解释器::解析乘法(const std::string& 表达式, size_t& 位置) {
    // 扩展：支持 * / % 和整除 // 以及乘方 **（其优先级高于乘除）
    std::string 左边 = 解析基本项(表达式, 位置);
    while (位置 < 表达式.size()) {
        // 处理乘方优先于乘除，所以若遇到 '*' 并后面还有 '*' 则处理为乘方
        if (表达式[位置] == '*' && 位置 + 1 < 表达式.size() && 表达式[位置 + 1] == '*') {
            // 乘方
            位置 += 2;
            std::string 右边 = 解析基本项(表达式, 位置);
            if (!是数字(左边) || !是数字(右边)) throw std::runtime_error("刘小黑[数学运算需要数字参数啦...]");
            数学高精度 a = toHigh(左边), b = toHigh(右边);
            // 使用 pow
            数学高精度 结果 = boost::multiprecision::pow(a, b);
            左边 = 格式化数学高精度(结果);
            continue;
        }
        // 整除 //
        if (表达式[位置] == '/' && 位置 + 1 < 表达式.size() && 表达式[位置 + 1] == '/') {
            位置 += 2;
            std::string 右边 = 解析基本项(表达式, 位置);
            if (!是数字(左边) || !是数字(右边)) throw std::runtime_error("刘小黑[数学运算需要数字参数啦...]");
            数学高精度 a = toHigh(左边), b = toHigh(右边);
            if (b == 数学高精度(0)) throw std::runtime_error("刘小黑[不可以除以零呢]");
            数学高精度 结果 = a / b;
            // 取整
            long long 整 = static_cast<long long>(结果);
            左边 = std::to_string(整);
            continue;
        }
        char 操作符 = 表达式[位置];
        if (操作符 == '*') {
            位置++;
            std::string 右边 = 解析基本项(表达式, 位置);
            if (!是数字(左边) || !是数字(右边)) throw std::runtime_error("刘小黑[数学运算需要数字参数啦...]");
            数学高精度 左值 = toHigh(左边);
            数学高精度 右值 = toHigh(右边);
            数学高精度 结果 = 左值 * 右值;
            左边 = 格式化数学高精度(结果);
        }
        else if (操作符 == '/') {
            位置++;
            std::string 右边 = 解析基本项(表达式, 位置);
            if (!是数字(左边) || !是数字(右边)) throw std::runtime_error("刘小黑[数学运算需要数字参数啦...]");
            数学高精度 左值 = toHigh(左边);
            数学高精度 右值 = toHigh(右边);
            if (右值 == 数学高精度(0)) throw std::runtime_error("刘小黑[不可以除以零呢]");
            数学高精度 结果 = 左值 / 右值;
            左边 = 格式化数学高精度(结果);
        }
        else if (表达式[位置] == '%') {
            位置++;
            std::string 右边 = 解析基本项(表达式, 位置);
            if (!是数字(左边) || !是数字(右边)) throw std::runtime_error("刘小黑[数学运算需要数字参数啦...]");
            数学高精度 a = toHigh(左边), b = toHigh(右边);
            if (b == 数学高精度(0)) throw std::runtime_error("刘小黑[不可以对零取模呢]");
            long long ai = static_cast<long long>(a);
            long long bi = static_cast<long long>(b);
            long long r = ai % bi;
            左边 = std::to_string(r);
        }
        else break;
    }
    return 左边;
}

std::string 解释器::解析基本项(const std::string& 表达式, size_t& 位置) {
    if (位置 >= 表达式.size()) {
        throw std::runtime_error("刘小黑[表达式不完整啦...]");
    }

    if (isdigit(static_cast<unsigned char>(表达式[位置])) ||
        (表达式[位置] == '-' && 位置 + 1 < 表达式.size() && isdigit(static_cast<unsigned char>(表达式[位置 + 1])))) {
        std::string 数字;
        if (表达式[位置] == '-') 数字 += 表达式[位置++];

        bool 有点 = false;
        while (位置 < 表达式.size()) {
            char ch = 表达式[位置];
            if (ch == '.') {
                if (有点) break;
                有点 = true;
                数字 += 表达式[位置++];
            }
            else if (isdigit(static_cast<unsigned char>(ch))) {
                数字 += 表达式[位置++];
            }
            else break;
        }
        return 数字;
    }

    if (::是字母字符(表达式[位置])) {
        std::string 变量;
        while (位置 < 表达式.size() && ::是字母数字字符(表达式[位置])) {
            变量 += 表达式[位置++];
        }
        if (位置 < 表达式.size() && 表达式[位置] == '[') {
            // 读取索引表达式直到对应']'
            ++位置; // 跳过 '['
            int depth = 1;
            std::string 索引;
            while (位置 < 表达式.size() && depth > 0) {
                char c = 表达式[位置++];
                if (c == '[') { depth++; 索引 += c; }
                else if (c == ']') { depth--; if (depth > 0) 索引 += c; }
                else 索引 += c;
            }
            if (depth != 0) throw std::runtime_error("刘小黑[访问数组用的方括号不匹配，可能...是漏写了？]");
            // 获取数组元素
            return this->getArrayElement(变量, 索引);
        }
        if (变量表.find(变量) != 变量表.end()) {
            return 变量表[变量];
        }
        throw std::runtime_error("刘小黑[这个好像是一个未定义变量呢，TA叫]: " + 变量);
    }

    if (表达式[位置] == '(') {
        位置++;
        std::string 结果 = 解析加法(表达式, 位置);
        if (位置 >= 表达式.size() || 表达式[位置] != ')') {
            throw std::runtime_error("刘小黑[括号好像不匹配...]");
        }
        位置++;
        return 结果;
    }

    throw std::runtime_error(std::string("刘小黑[唔...一个无法解析的字符]: ") + std::string(1, 表达式[位置]));
}