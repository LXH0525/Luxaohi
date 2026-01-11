#pragma once
#include <string>

class 解释器;

void 交互执行();
void 执行文件(const std::string& 文件路径, 解释器& 解释器);