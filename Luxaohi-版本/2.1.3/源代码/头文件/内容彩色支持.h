#ifndef 内容彩色支持_H
#define 内容彩色支持_H
#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

inline void 输出文本(const string& 文本, const string& 颜色 = "白", bool 换行 = true,const string& 样式 = "") {

    static const unordered_map<string, string> 颜色表 = {
        // 基础色
        {"红", "\033[31m"}, 
        {"R", "\033[31m"},
        {"绿", "\033[32m"},
        {"G", "\033[32m"},
        {"蓝", "\033[34m"}, 
        {"B", "\033[34m"},
        {"黄", "\033[33m"},
        {"Y", "\033[33m"},
        {"紫", "\033[35m"},
        {"青", "\033[36m"}, 
        {"白", "\033[37m"},
        {"黑", "\033[30m"},

        // 亮色
        {"亮红", "\033[91m"},
        {"RR", "\033[91m"},
        {"亮绿", "\033[92m"}, 
        {"GG", "\033[92m"},
        {"亮蓝", "\033[94m"}, 
		{"BB", "\033[94m"},
        {"亮黄", "\033[93m"},
        {"YY", "\033[93m"},
        {"亮紫", "\033[95m"}, 
        {"亮青", "\033[96m"},

        // 背景色
        {"黑-背景", "\033[40m"}, 
        {"红-背景", "\033[41m"},
        {"绿-背景", "\033[42m"}
    };

    // 样式表
    static const unordered_map<string, string> 样式表 = {
        {"粗体", "\033[1m"},
        {"下划线", "\033[4m"},
        {"闪烁", "\033[5m"}
    };

    // 查找颜色
    auto 颜色查找结果 = 颜色表.find(颜色);
    if (颜色查找结果 != 颜色表.end()) {
        cout << 颜色查找结果->second;
    }

    // 处理样式
    if (!样式.empty()) {
        size_t 开始位置 = 0;
        while (true) {
            // 找逗号位置
            size_t 逗号位置 = 样式.find(',', 开始位置);

            // 提取一个样式名
            string 单个样式名 = 样式.substr(开始位置,
                逗号位置 - 开始位置);

            // 快速查找样式
            auto 样式查找结果 = 样式表.find(单个样式名);
            if (样式查找结果 != 样式表.end()) {
                cout << 样式查找结果->second;
            }

            // 如果没逗号了结束
            if (逗号位置 == string::npos) break;

            // 跳过逗号继续下一个
            开始位置 = 逗号位置 + 1;
        }
    }

    // 输出文本
    cout << 文本 << "\033[0m";  // 重置颜色和样式

    // 仅布尔为True的时候换行
    if (换行) {
        cout << endl;
    }
}

#endif