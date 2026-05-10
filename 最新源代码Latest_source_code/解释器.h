#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

class 语法树节点;

class 解释器 {
public:
    struct 函数 {
        std::vector<std::string> 参数列表;
        std::vector<std::shared_ptr<语法树节点>> 函数体;
    };

    std::map<std::string, std::string> 变量表;
    std::map<std::string, 函数> 函数表;
    std::map<std::string, std::vector<std::string>> 数组表;

    void 添加函数(const std::string& 名称, const 函数& 函数);
    void 执行(const std::vector<std::shared_ptr<语法树节点>>& 节点列表);
    std::string 调用函数(const std::string& 名称, const std::vector<std::string>& 实参列表);
    std::string 求值(const std::string& 表达式);
    bool 求值条件(const std::string& 条件);
    std::string getArrayElement(const std::string& 名, const std::string& 索引表达式);
    void setArrayElement(const std::string& 名, const std::string& 索引表达式, const std::string& 值表达式);

    struct 返回异常 {
        std::string 值;
        返回异常(const std::string& v) : 值(v) {}
    };

private:
    bool 是整数(const std::string& 字符串);
    bool 是数字(const std::string& s);
    std::string 求值表达式(const std::string& 表达式);
    std::string 解析加法(const std::string& 表达式, size_t& 位置);
    std::string 解析乘法(const std::string& 表达式, size_t& 位置);
    std::string 解析基本项(const std::string& 表达式, size_t& 位置);
};