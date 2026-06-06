#define _CRT_SECURE_NO_WARNINGS
#include "交互.h"
#include "解释器.h"
#include "工具.h"

int main(int argc, char* argv[]) {
    工具(1);
    if (argc > 1) {
        解释器 解释器实例;
        执行文件(argv[1], 解释器实例);
    }
    else {
        交互执行();
    }
    return 0;
}
//你看什么呢？啥也没有了啊。