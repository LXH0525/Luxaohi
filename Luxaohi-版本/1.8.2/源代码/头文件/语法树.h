#pragma once
#include <string>
#include <vector>
#include <memory>

class 解释器;

class 语法树节点 {
public:
    virtual ~语法树节点() = default;
    virtual void 执行(解释器& 解释器) = 0;
};

class 函数声明节点 : public 语法树节点 {
public:
    std::string 名称;
    std::vector<std::string> 参数列表;
    std::vector<std::shared_ptr<语法树节点>> 函数体;
    void 执行(解释器& 解释器) override;
};

class 函数调用节点 : public 语法树节点 {
public:
    std::string 名称;
    std::vector<std::string> 实参列表;
    void 执行(解释器& 解释器) override;
};

class 变量赋值节点 : public 语法树节点 {
public:
    std::string 变量名;
    std::string 值;
    void 执行(解释器& 解释器) override;
};

class 条件语句节点 : public 语法树节点 {
public:
    std::string 条件;
    std::vector<std::shared_ptr<语法树节点>> 那么分支;
    std::vector<std::shared_ptr<语法树节点>> 否则分支;
    void 执行(解释器& 解释器) override;
};