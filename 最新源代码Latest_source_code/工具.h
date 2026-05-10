#pragma once
#include <string>
#include <vector>

extern std::string 全局用户名;
int 工具(int 部分);
std::string 获取用户名();

inline bool 是字母字符(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c & 0x80);
}

inline bool 是字母数字字符(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || (c & 0x80);
}