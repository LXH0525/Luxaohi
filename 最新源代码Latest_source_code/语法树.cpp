#include "语法树.h"
#include "解释器.h"
#include "工具.h"
#include "高精度数学运算.h"
#include <stdexcept>

void 函数声明节点::执行(解释器& 解释器) {
    解释器.添加函数(名称, { 参数列表, 函数体 });
}

void 函数调用节点::执行(解释器& 解释器) {
    解释器.调用函数(名称, 实参列表);
}

void 变量赋值节点::执行(解释器& 解释器) {
    // 普通变量声明/赋值：计算右侧表达式并存入变量表
    解释器.变量表[变量名] = 解释器.求值(值);
}

void 增量赋值节点::执行(解释器& 解释器) {
    // 支持 +=, -=, *=, /= 操作
    std::string cur;
    auto it = 解释器.变量表.find(变量名);
    // 如果变量未定义，抛出错误，要求先声明后修改
    if (it == 解释器.变量表.end()) throw std::runtime_error("刘小黑[变量需要先定义才能修改呢]");
    cur = it->second;
    std::string rhs = 解释器.求值(值);
    // 本地数字检查，避免调用解释器私有方法
    auto isNum = [](const std::string& s) -> bool {
        if (s.empty()) return false;
        size_t i = 0; if (s[0] == '-') { if (s.size() == 1) return false; i = 1; }
        bool dot = false; for (; i < s.size(); ++i) { char ch = s[i]; if (ch == '.') { if (dot) return false; dot = true; continue; } if (!isdigit(static_cast<unsigned char>(ch))) return false; } return true;
    };
    if (!isNum(cur) || !isNum(rhs)) throw std::runtime_error("刘小黑[数学运算需要数字参数啦]");
    数学高精度 left = toHigh(cur);
    数学高精度 right = toHigh(rhs);
    数学高精度 res;
    if (操作符 == "+=") res = left + right;
    else if (操作符 == "-=") res = left - right;
    else if (操作符 == "*=") res = left * right;
    else if (操作符 == "/=") { if (right == 数学高精度(0)) throw std::runtime_error("刘小黑[喵喵，不可以除以零]"); res = left / right; }
    else if (操作符 == "**=") {
        // 幂运算
        数学高精度 结果 = boost::multiprecision::pow(left, right);
        res = 结果;
    }
    else if (操作符 == "%=") {
        // 取模，转换为整数再取模
        if (right == 数学高精度(0)) throw std::runtime_error("刘小黑[不可以对零取模呢]");
        long long li = static_cast<long long>(left);
        long long ri = static_cast<long long>(right);
        long long mr = li % ri;
        res = 数学高精度(mr);
    }
    else if (操作符 == "//=") {
        if (right == 数学高精度(0)) throw std::runtime_error("刘小黑[喵喵，不可以除以零]");
        数学高精度 tmp = left / right;
        long long iv = static_cast<long long>(tmp);
        res = 数学高精度(iv);
    }
    else throw std::runtime_error("刘小黑[有一个不支持的赋值操作符呐: " + 操作符 + "]");
    解释器.变量表[变量名] = 格式化数学高精度(res);
}

void 数组声明节点::执行(解释器& 解释器) {
    // 大小若为0则使用初始值数量，初始值为字符串字面量或数字等
    std::vector<std::string> arr;
    if (初始值.empty()) {
        arr.assign(大小, std::string());
        解释器.数组表[名称] = arr;
    } else {
        arr.assign(大小, std::string());
        for (size_t i = 0; i < 初始值.size() && i < 大小; ++i) {
            解释器.数组表[名称].push_back(解释器.求值(初始值[i]));
        }
        // 如果初始值比大小少，按 C++ 语义进行零初始化（数值为"0"，字符为"\0"，否则空字符串）
        if (解释器.数组表[名称].size() < 大小) {
            size_t need = 大小 - 解释器.数组表[名称].size();
            std::string 默认填充值 = std::string();
            if (!初始值.empty()) {
                std::string firstValEvaluated = 解释器.求值(初始值[0]);
                auto isNum = [](const std::string& s) {
                    if (s.empty()) return false;
                    size_t i = 0; if (s[0] == '-') { if (s.size() == 1) return false; i = 1; }
                    bool dot = false; for (; i < s.size(); ++i) { if (s[i] == '.') { if (dot) return false; dot = true; continue; } if (!isdigit(static_cast<unsigned char>(s[i]))) return false; } return true;
                };
                if (isNum(firstValEvaluated)) 默认填充值 = "0";
                else if (!firstValEvaluated.empty() && firstValEvaluated.size() == 1) 默认填充值 = std::string(1, firstValEvaluated[0]);
                else 默认填充值 = std::string();
            }
            for (size_t k = 0; k < need; ++k) 解释器.数组表[名称].push_back(默认填充值);
        }
    }
}

void 数组元素赋值节点::执行(解释器& 解释器) {
    // 先计算索引
    auto trim = [](const std::string& s) {
        size_t a = 0; while (a < s.size() && isspace(static_cast<unsigned char>(s[a]))) ++a;
        if (a == s.size()) return std::string(); size_t b = s.size() - 1; while (b > a && isspace(static_cast<unsigned char>(s[b]))) --b;
        return s.substr(a, b - a + 1);
    };

    std::string 索引修剪 = trim(索引表达式);
    std::string idxs = 解释器.求值(索引修剪);
    // 如果求值返回和原始相同，且原始看起来像标识符，则尝试直接从变量表取值
    auto looks_like_ident = [](const std::string& s) {
        if (s.empty()) return false;
        if (!::是字母字符(s[0])) return false;
        for (size_t i = 1; i < s.size(); ++i) if (!::是字母数字字符(s[i])) return false;
        return true;
    };
    if (idxs == 索引修剪 && looks_like_ident(索引修剪)) {
        if (解释器.变量表.find(索引修剪) != 解释器.变量表.end()) {
            idxs = 解释器.变量表[索引修剪];
        }
        else {
            throw std::runtime_error("刘小黑[索引使用了未定义的变量]: " + 索引修剪);
        }
    }

    if (idxs.empty() || !( (idxs[0]>='0' && idxs[0]<='9') || (idxs.size()>0 && idxs[0]=='-' && idxs.size()>1) )) {
        throw std::runtime_error("刘小黑[数组索引需要的是数字呢]: " + idxs);
    }
    int idx = std::stoi(idxs);
    if (解释器.数组表.find(名称) == 解释器.数组表.end()) throw std::runtime_error("刘小黑[一个未定义的数组，叫]: " + 名称 + " 呢喵~");
    if (idx < 0 || static_cast<size_t>(idx) >= 解释器.数组表[名称].size()) throw std::runtime_error("刘小黑[数组索引越界啦喵]");

    std::string val = 解释器.求值(trim(值表达式));
    // 如果求值返回与原始表达式相同，则尝试在值表达式中解析数组访问并替换为元素值后重试求值
    std::string 原始修剪 = trim(值表达式);
    if (val == 原始修剪) {
        std::string s = 原始修剪;
        std::string out;
        for (size_t i = 0; i < s.size();) {
            unsigned char c = static_cast<unsigned char>(s[i]);
            if (::是字母字符(s[i])) {
                // 读取名称
                size_t j = i;
                std::string name;
                while (j < s.size() && ::是字母数字字符(s[j])) { name += s[j++]; }
                if (j < s.size() && s[j] == '[') {
                    // 找到匹配 ]
                    size_t k = j + 1; int depth = 1;
                    std::string inside;
                    while (k < s.size() && depth > 0) {
                        if (s[k] == '[') { inside += s[k]; ++depth; ++k; }
                        else if (s[k] == ']') { --depth; if (depth > 0) { inside += ']'; } ++k; }
                        else { inside += s[k++]; }
                    }
                    if (depth == 0) {
                        // 获取该数组元素值并追加
                        std::string elem = 解释器.getArrayElement(name, inside);
                        out += elem;
                        i = k;
                        continue;
                    }
                    else {
                        out += name; i = j;
                        continue;
                    }
                }
                else {
                    out += name; i = j; continue;
                }
            }
            else { out += s[i++]; }
        }
        try {
            std::string newVal = 解释器.求值(out);
            val = newVal;
        }
        catch (...) {
        }
    }

    // 直接写入数组元素
    解释器.数组表[名称][idx] = val;
}

void 条件语句节点::执行(解释器& 解释器) {
    // 如果条件为真，执行 then 分支并结束
    if (解释器.求值条件(条件)) {
        for (auto& 节点 : 那么分支) 节点->执行(解释器);
        return;
    }

    // 否则，对否则分支按顺序处理：支持链式的 "或者" (作为 elif)
    // 如果否则分支中的某个节点是 条件语句节点，则把它当作一个 elif：先求值其条件，若为真则执行其 then 分支并结束整个 if 链；否则继续检查下一个分支。
    // 如果遇到第一个非 条件语句节点，则把从该节点开始到结束的所有节点视为最终的 else 分支，全部执行后结束。
    for (size_t i = 0; i < 否则分支.size(); ++i) {
        auto& node = 否则分支[i];
        auto condNode = std::dynamic_pointer_cast<条件语句节点>(node);
        if (condNode) {
            // 这是一个 elif 风格的分支，检查其条件
            if (解释器.求值条件(condNode->条件)) {
                for (auto& n : condNode->那么分支) n->执行(解释器);
                return;
            }
            // 条件为假，继续检查下一个否则分支项
        }
        else {
            // 第一个非条件节点，执行从这里到末尾的所有节点作为 else 分支
            for (size_t j = i; j < 否则分支.size(); ++j) {
                否则分支[j]->执行(解释器);
            }
            return;
        }
    }
}

void 当循环节点::执行(解释器& 解释器) {
    // 持续执行循环体，直到条件不满足
    while (解释器.求值条件(条件)) {
        for (auto& 语句 : 循环体) {
            语句->执行(解释器);
        }
    }
}

void 返回语句节点::执行(解释器& 解释器) {
    // 计算返回表达式并通过异常机制抛出以便调用处捕获并结束函数执行
    std::string 返回值 = 解释器.求值(值);
    throw 解释器::返回异常(返回值);
}