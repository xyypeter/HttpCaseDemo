#include <iostream>
#include <iomanip>
#include <cstdint>

uint32_t my_htonl(uint32_t x);

int main(){
    uint32_t test = 0x12345678;

    uint32_t result = my_htonl(test);

    std::cout << std::hex << std::showbase;
    std::cout << "Original : " << test << std::endl;
    std::cout << "Converted: " << result << std::endl;

    return 0;
}

//将一个32位整数从主机字节序转换为网络字节序（大端序）
uint32_t my_htonl(uint32_t hostlong) {
    uint16_t test = 0x1;

    // 小端
    if (*(uint8_t*)&test == 1) {
        return ((hostlong & 0x000000FF) << 24) |
            ((hostlong & 0x0000FF00) << 8) |
            ((hostlong & 0x00FF0000) >> 8) |
            ((hostlong & 0xFF000000) >> 24);
    }

    // 大端直接返回
    return hostlong;
}