#define _CRT_SECURE_NO_WARNINGS
#include "交互.h"
#include "词法分析器.h"
#include "语法分析器.h"
#include "内容彩色支持.h"
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
    输出文本("———————————————————————————————————————————————————————————————————————————");
    输出文本("Luxaohi 中文编程语言解释器");
    输出文本("[此版本主要作者与开发者：ryun、GitHub Copilot、Deep Seek]");
    输出文本("[使用的IDE：Visual Studio 2026、CLion 2025.3.2]");
    输出文本("[贡献者：请转至'https://github.com/LXH0525/Luxaohi/blob/main/%E6%96%87%E4%BB%B6/%E8%B4%A1%E7%8C%AE%E8%80%85.txt'查看]");
    输出文本("刘小黑[你好 " + 全局用户名 + " ，我叫刘小黑，很高兴认识你喵~]");
    输出文本("———————————————————————————————————————————————————————————————————————————");
    输出文本("提示：");
    输出文本("输入\"叫我 昵称\"可以更改称呼");
    输出文本("输入\"清理\"可以清屏");
    输出文本("输入\"睡觉\"可以退出");
    输出文本("———————————————————————————————————————————————————————————————————————————");
}

void 执行文件(const std::string& 文件路径, 解释器& 解释器) {
    std::ifstream 文件(文件路径, std::ios::in | std::ios::binary);
    if (!文件.is_open()) {
#ifdef _WIN32
        HANDLE 文件句柄 = CreateFileA(文件路径.c_str(), GENERIC_READ, FILE_SHARE_READ,
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
        throw std::runtime_error("刘小黑[喵呜，无法打开文件 " + 文件路径 + " ]");
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
    工具(2);
    全局用户名 = 获取用户名();
    显示欢迎信息();
    解释器 解释器;
    while (true) {
        输出文本("(>ω<) ","BB",0); std::string 行; std::getline(std::cin, 行);
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
            std::string 剩余部分 = 行.substr(strlen("运行"));

            size_t 开始位置 = 剩余部分.find_first_not_of(" \t");
            if (开始位置 == std::string::npos) {
                std::cout << "刘小黑[可以告诉我一下文件路径嘛喵~]" << std::endl;
                continue;
            }

            // 提取路径字符串，可能包含引号
            std::string 原始路径 = 剩余部分.substr(开始位置);

            // 去除首尾的空格
            size_t 结束位置 = 原始路径.find_last_not_of(" \t");
            std::string 清理后的路径 = 原始路径.substr(0, 结束位置 + 1);

            // 检查并去除引号
            std::string 文件路径 = 清理后的路径;

            if (!文件路径.empty() && 文件路径.length() >= 2) {
                // 1. 先检查英文引号（最简单的情况）
                if ((文件路径[0] == '"' && 文件路径.back() == '"') ||
                    (文件路径[0] == '\'' && 文件路径.back() == '\'')) {
                    // 英文引号：直接去掉首尾各1个字节
                    文件路径 = 文件路径.substr(1, 文件路径.length() - 2);
                }
                // 2. 再检查中文引号（需要特殊处理）
                else if (文件路径.length() >= 6) {
                    // 获取开头的UTF-8字符
                    std::string 开头字符;
                    if (static_cast<unsigned char>(文件路径[0]) >= 0xF0) {
                        开头字符 = 文件路径.substr(0, 4);
                    }
                    else if (static_cast<unsigned char>(文件路径[0]) >= 0xE0) {
                        开头字符 = 文件路径.substr(0, 3);  // 中文标点
                    }
                    else if (static_cast<unsigned char>(文件路径[0]) >= 0xC0) {
                        开头字符 = 文件路径.substr(0, 2);
                    }
                    else {
                        开头字符 = 文件路径.substr(0, 1);
                    }

                    // 获取结尾的UTF-8字符
                    size_t 结尾字符长度 = 1;
                    size_t 结尾位置 = 文件路径.length() - 1;
                    while (结尾位置 > 0 &&
                        (static_cast<unsigned char>(文件路径[结尾位置]) & 0xC0) == 0x80) {
                        结尾位置--;
                        结尾字符长度++;
                    }
                    std::string 结尾字符 = 文件路径.substr(结尾位置, 结尾字符长度);

                    // 检查引号是否匹配
                    if ((开头字符 == "“" && 结尾字符 == "”") ||
                        (开头字符 == "‘" && 结尾字符 == "’")) {
                        // 正确的中文引号对
                        文件路径 = 文件路径.substr(开头字符.length(),
                            文件路径.length() - 开头字符.length() - 结尾字符.length());
                    }
                    else if ((开头字符 == "“" || 开头字符 == "‘" ||
                        开头字符 == "”" || 开头字符 == "’") &&
                        (结尾字符 == "\"" || 结尾字符 == "'")) {
                        // 开头是中文引号，结尾是英文引号
                        std::cout << "刘小黑[请不要混合使用中文和英文引号喵~]" << std::endl;
                        continue;
                    }
                    else if ((开头字符 == "\"" || 开头字符 == "'") &&
                        (结尾字符 == "”" || 结尾字符 == "’")) {
                        // 开头是英文引号，结尾是中文引号
                        std::cout << "刘小黑[请不要混合使用中文和英文引号喵~]" << std::endl;
                        continue;
                    }
                    // 其他情况：不是引号，保持原样
                }
            }

            // 检查去除引号后是否为空
            if (文件路径.empty()) {
                std::cout << "刘小黑[可以告诉我一下文件路径嘛喵~]" << std::endl;
                continue;
            }

            try {
                执行文件(文件路径, 解释器);
            }
            catch (const std::exception& 异常) {
                std::cout << 异常.what() << std::endl;
            }
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