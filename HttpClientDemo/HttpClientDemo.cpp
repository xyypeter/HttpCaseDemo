#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <unistd.h>
#endif

int main(){
#ifdef _WIN32
    //在 Windows 平台上，网络功能并非默认开启，而是被封装在一个名为 Winsock（Windows Sockets）​ 的动态链接库里。
    //这段代码的作用就是显式地加载并初始化这个网络库。
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    //1.创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    //2.服务器地址
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = inet_addr("182.61.200.108");//网站对应IP
    //3.连接
    if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        std::cout<<"connect failed\n";
        return -1;
    }
    //4.构造Http请求
    std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n"
        "Connection: close\r\n\r\n";
    //5.发送
    send(sock, request.c_str(), request.size(), 0);
    //6.接收
    char buffer[4096];
    std::string response;//用于存储完整的响应
    while (true) {
        int n = recv(sock, buffer, sizeof(buffer), 0);
        if (n <= 0) break;
        //std::cout.write(buffer, n);//显示在控制台中
        response.append(buffer, n);
    }
    //只输出Header的内容
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
        std::string header = response.substr(0, pos);
        std::cout << "===== HTTP HEADER =====\n";
        std::cout << header << std::endl;
    }
    else {
        std::cout << "Invalid HTTP response\n";
    }


    // 7. 关闭
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}

