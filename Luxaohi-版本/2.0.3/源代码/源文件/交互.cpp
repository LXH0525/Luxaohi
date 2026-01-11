#define _CRT_SECURE_NO_WARNINGS
#include "交互.h"
#include "词法分析器.h"
#include "语法分析器.h"
#include "解释器.h"
#include "语法树.h"
#include "工具.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#endif

static void 显示欢迎信息() {
    std::cout << "———————————————————————————————————————————————————————————————————————————" << std::endl;
    std::cout << "Luxaohi 中文编程语言解释器[作者：ryun、deep seek] " << std::endl;
    std::cout << "Luxaohi 中文编程语言解释器[使用的编程器：Visual Studio 2026]" << std::endl;
    std::cout << "Luxaohi 中文编程语言解释器[贡献者：暂无]" << std::endl;
    std::cout << "刘小黑[你好 " << 全局用户名 + "，我叫刘小黑，很高兴认识你喵~]" << std::endl;
    std::cout << "———————————————————————————————————————————————————————————————————————————" << std::endl;
    std::cout << "提示：" << std::endl;
    std::cout << "输入\"叫我 昵称\"可以更改称呼" << std::endl;
    std::cout << "输入\"清理\"可以清屏" << std::endl;
    std::cout << "输入\"睡觉\"可以退出" << std::endl;
    std::cout << "———————————————————————————————————————————————————————————————————————————" << std::endl;
}

void 执行文件(const std::string& 文件路径, 解释器& 解释器) {
    // 如果调用时未提供路径，直接给出友好提示，避免后续产生乱码错误信息
    if (文件路径.empty()) {
        throw std::runtime_error("刘小黑[运行命令运行文件的路径是什么呐喵？]");
    }

    std::ifstream 文件(文件路径, std::ios::in | std::ios::binary);
    if (!文件.is_open()) {
#ifdef _WIN32
        // 尝试使用宽字符 API 打开（支持 UTF-8 路径）
        std::wstring w路径;
        int len = MultiByteToWideChar(CP_UTF8, 0, 文件路径.c_str(), -1, NULL, 0);
        if (len > 0) {
            w路径.resize(len);
            MultiByteToWideChar(CP_UTF8, 0, 文件路径.c_str(), -1, &w路径[0], len);
            if (!w路径.empty() && w路径.back() == L'\0') w路径.pop_back();
        }

        HANDLE 文件句柄 = CreateFileW(w路径.c_str(), GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (文件句柄 != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER 文件大小;
            if (GetFileSizeEx(文件句柄, &文件大小)) {
                if (文件大小.QuadPart > 0 && 文件大小.QuadPart <= 10 * 1024 * 1024) {
                    std::vector<char> 缓冲区(static_cast<size_t>(文件大小.QuadPart) + 1);
                    DWORD 读取字节数;
                    if (ReadFile(文件句柄, 缓冲区.data(), static_cast<DWORD>(文件大小.QuadPart), &读取字节数, NULL)) {
                        缓冲区[读取字节数] = '\0'; CloseHandle(文件句柄);
                        std::string 代码(缓冲区.data(), 读取字节数);
                        try {
                            词法分析器 词法器(代码);
                            语法分析器 语法分析器(词法器.分析());
                            解释器.执行(语法分析器.分析());
                            std::cout << "刘小黑[文件 " + 文件路径 + " 执行成功喵~]" << std::endl;
                        }
                        catch (const std::exception& 异常) { throw 异常; }
                        return;
                    }
                }
            }
            CloseHandle(文件句柄);
        }
#endif
        // 保留文件路径信息（此处文件路径不为空）；若需隐藏路径可改为固定提示
        throw std::runtime_error(std::string("刘小黑[喵呜，无法打开文件 ") + 文件路径 + " ]");
    }

    std::stringstream 缓冲流; 缓冲流 << 文件.rdbuf();
    std::string 代码 = 缓冲流.str(); 文件.close();
    if (代码.empty()) throw std::runtime_error("刘小黑[文件 " + 文件路径 + " 是空的呢喵~]");
    try {
        词法分析器 词法器(代码);
        语法分析器 语法分析器(词法器.分析());
        解释器.执行(语法分析器.分析());
        std::cout << "刘小黑[文件 " + 文件路径 + " 运行成功啦]" << std::endl;
    }
    catch (const std::exception& 异常) { throw 异常; }
}

void 交互执行() {
    初始化控制台编码(); 全局用户名 = 获取用户名(); 显示欢迎信息();
    解释器 解释器;
    while (true) {
        std::cout << "(>ω<) "; std::string 行; std::getline(std::cin, 行);
        size_t 开始位置 = 行.find_first_not_of(" \t");
        if (开始位置 == std::string::npos) continue;
        size_t 结束位置 = 行.find_last_not_of(" \t");
        行 = 行.substr(开始位置, 结束位置 - 开始位置 + 1);
        if (行 == "睡觉") break;
        else if (行.find("叫我") == 0) {
            std::string 昵称 = 行.substr(6);
            bool 全是空格 = true;
            for (char 字符 : 昵称) if (字符 != ' ') { 全是空格 = false; break; }
            if (昵称.empty() || 全是空格) std::cout << "刘小黑[不能用空格！要不然我怎么叫你喵~]" << std::endl;
            else {
                size_t 开始 = 昵称.find_first_not_of(" ");
                size_t 结束 = 昵称.find_last_not_of(" ");
                if (开始 != std::string::npos && 结束 != std::string::npos) 昵称 = 昵称.substr(开始, 结束 - 开始 + 1);
                全局用户名 = 昵称; std::cout << "刘小黑[好的，那以后就叫你" << 全局用户名 + "了喵~]" << std::endl;
            }
            continue;
        }
        else if (行 == "清理") {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            显示欢迎信息(); continue;
        }
        else if (行.find("运行") == 0) {
            std::string 文件路径; size_t 引号位置 = 行.find('"', 2);
            if (引号位置 != std::string::npos) {
                size_t 结束引号位置 = 行.find('"', 引号位置 + 1);
                if (结束引号位置 != std::string::npos) 文件路径 = 行.substr(引号位置 + 1, 结束引号位置 - 引号位置 - 1);
            }
            if (文件路径.empty()) {
                文件路径 = 行.substr(4);
                size_t 路径开始位置 = 文件路径.find_first_not_of(" \t");
                if (路径开始位置 == std::string::npos) {
                    std::cout << "刘小黑[可以告诉我一下文件路径嘛喵~]" << std::endl; continue;
                }
                size_t 路径结束位置 = 文件路径.find_last_not_of(" \t");
                文件路径 = 文件路径.substr(路径开始位置, 路径结束位置 - 路径开始位置 + 1);
                if (文件路径.front() == '"' && 文件路径.back() == '"') 文件路径 = 文件路径.substr(1, 文件路径.length() - 2);
                else if (文件路径.front() == '\'' && 文件路径.back() == '\'') 文件路径 = 文件路径.substr(1, 文件路径.length() - 2);
            }
            if (文件路径.empty()) { std::cout << "刘小黑[可以告诉我一下文件路径嘛喵~]" << std::endl; continue; }
            try { 执行文件(文件路径, 解释器); }
            catch (const std::exception& 异常) { std::cout << 异常.what() << std::endl; }
            continue;
        }
        try {
            词法分析器 词法器(行);
            语法分析器 语法分析器(词法器.分析());
            解释器.执行(语法分析器.分析());
        }
        catch (const std::exception& 异常) { std::cout << 异常.what() << std::endl; }
    }
}