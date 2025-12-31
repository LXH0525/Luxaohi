#define _CRT_SECURE_NO_WARNINGS
#include "工具.h"
#include <locale>
#include <codecvt>
#ifdef _WIN32
#include <iostream>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#endif

std::string 全局用户名 = "Administrator";

void 初始化控制台编码() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
    try {
        std::locale utf8_locale("zh_CN.UTF-8");
        std::cout.imbue(utf8_locale);
        std::cin.imbue(utf8_locale);
    }
    catch (...) {
        std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);
        std::cout.imbue(utf8_locale);
        std::cin.imbue(utf8_locale);
    }
}

std::string 获取用户名() {
#ifdef _WIN32
    std::vector<std::string> 尝试方法 = { "USERNAME", "USER", "USERPROFILE" };
    for (const auto& 环境变量 : 尝试方法) {
        const char* 值 = std::getenv(环境变量.c_str());
        if (值 && 值[0] != '\0') {
            std::string 用户名 = 值;
            size_t 最后斜杠 = 用户名.find_last_of("\\/");
            if (最后斜杠 != std::string::npos && 最后斜杠 + 1 < 用户名.length()) {
                用户名 = 用户名.substr(最后斜杠 + 1);
            }
            return 用户名;
        }
    }
    DWORD 大小 = 0;
    GetUserNameA(nullptr, &大小);
    if (大小 > 0) {
        std::vector<char> 缓冲区(大小);
        if (GetUserNameA(缓冲区.data(), &大小)) {
            return std::string(缓冲区.data());
        }
    }
#endif
    return "Administrator";
}