// tests/test.cc
#include <iostream>

#include "../src/log.h"

int main(int argc, char **argv) {
    std::cout << "hello ddddddddd__FILE__=" << __RELATIVE_PATH__ << std::endl;
    return 0;
}

//输出：hello __FILE__=/home/sylar/test/cmake_test/tests/test.cc
