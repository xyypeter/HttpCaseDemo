#include <iostream>
#include <cassert>
#include "HttpParser.h"

int main(){
    HttpParser parser;
    std::cout << "测试 1: 标准完整请求" << std::endl;
    parser.feed("GET /index/html HTTP/1.1\r\n");
    assert(parser.hasRequest()); // 断言解析成功
    HttpRequest req1 = parser.getRequest();
    std::cout << "Method: " << req1.method << ",URL:" << req1.url << std::endl;
    assert(req1.method == "GET");//验证测试的1方法

    std::cout << "测试 2: 半包处理 (数据分两次到达)" << std::endl;
    parser.feed("POST /api/login HTTP");  // 模拟第一次只收到前半段
    assert(!parser.hasRequest());         // 此时不应有请求
    parser.feed("/1.1\r\n");              // 模拟第二次收到后半段
    assert(parser.hasRequest());          // 此时应拼装完成
    HttpRequest req2 = parser.getRequest();
    std::cout << "Method: " << req2.method << ",URL:" << req2.url << std::endl;


    std::cout << "测试 3: 粘包处理 (多条请求同时到达)" << std::endl;
    // 模拟一次性收到两条紧密相连的请求
    parser.feed("PUT /file.txt HTTP/1.1\r\nDELETE /old.txt HTTP/1.1\r\n");

    // 验证第一条
    assert(parser.hasRequest());
    HttpRequest req3 = parser.getRequest();
    std::cout << "First: " << req3.method << ",URL:" << req3.url << std::endl;
    assert(req3.method == "PUT");

    // 验证第二条
    assert(parser.hasRequest());
    HttpRequest req4 = parser.getRequest();
    std::cout << "Second: " << req4.method << ",URL:" << req4.url << std::endl;
    assert(req4.method == "DELETE");
    
    return 0;
}
