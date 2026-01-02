#define _CRT_SECURE_NO_WARNINGS
#include "交互.h"
#include "解释器.h"

int main(int argc, char* argv[]) {
    if (argc > 1) {
        解释器 解释器实例;
        执行文件(argv[1], 解释器实例);
    }
    else {
        交互执行();
    }
    return 0;
}