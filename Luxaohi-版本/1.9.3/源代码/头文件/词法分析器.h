#pragma once
#include <string>
#include <vector>
#include "工具.h"

class 词法分析器 {
public:
    enum 令牌类型 {
        函数关键字, 如果关键字, 那么关键字, 否则关键字, 变量关键字,
        标识符, 数字, 字符串,
        左括号, 右括号, 左花括号, 右花括号, 逗号, 分号, 等号, 加号, 减号,
        乘号, 除号, 等于, 不等于, 大于, 小于, 结束
    };

    struct 令牌 {
        令牌类型 类型;
        std::string 值;
        int 行号;
        int 列号;
        令牌(令牌类型 t, std::string v, int l, int c);
    };

    词法分析器(const std::string& 代码);
    std::vector<令牌> 分析();

private:
    std::string 代码;
    size_t 位置;
    int 行号;
    int 列号;
};