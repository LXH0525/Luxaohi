#pragma once
#include <vector>
#include <memory>
#include <string>
#include "词法分析器.h"
#include "语法树.h"

class 语法分析器 {
public:
    语法分析器(const std::vector<词法分析器::令牌>& 令牌列表);
    std::vector<std::shared_ptr<语法树节点>> 分析();

private:
    std::vector<词法分析器::令牌> 令牌列表;
    size_t 位置;

    bool 匹配(词法分析器::令牌类型 类型);
    bool 检查标识符后有左括号();
    词法分析器::令牌 消耗(词法分析器::令牌类型 类型, const std::string& 错误信息);
    词法分析器::令牌 消耗();

    std::shared_ptr<函数声明节点> 分析函数();
    std::shared_ptr<函数调用节点> 分析调用();
    std::shared_ptr<变量赋值节点> 分析变量赋值();
    std::shared_ptr<条件语句节点> 分析如果语句();
};