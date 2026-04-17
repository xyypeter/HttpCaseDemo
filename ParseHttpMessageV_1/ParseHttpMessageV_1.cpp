#include <iostream>
#include <cassert>
#include "HttpParser.h"


// 辅助函数：打印请求详情（含头部与Body）
void printRequest(const HttpRequest& req) {
    std::cout << "解析结果: " << req.method << " " << req.url << " " << req.version << std::endl;
    for (const auto& h : req.headers) {
        std::cout << "   " << h.first << ": " << h.second << std::endl;
    }
    if (!req.body.empty()) {
        std::cout << "Body: [" << req.body << "]" << std::endl;
    }
    std::cout << std::endl;
}

int main() {

    // =========================
    // 测试1：完整单包
    // =========================
    {
        HttpParser parser;

        std::string req =
            "POST /test HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 11\r\n"
            "\r\n"
            "hello=world";

        parser.feed(req);

        assert(parser.hasRequest());
        auto r = parser.getRequest();

        std::cout << "[Test1] Single Packet OK → "
            << r.method << " " << r.url
            << " BODY=" << r.body << std::endl;

        assert(r.method == "POST");
        assert(r.body == "hello=world");
    }

    // =========================
    // 测试2：半包（body分两段）
    // =========================
    {
        HttpParser parser;

        std::string part1 =
            "POST /test HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 11\r\n"
            "\r\n"
            "hello=";

        std::string part2 = "world";

        parser.feed(part1);
        assert(!parser.hasRequest());  // ❗还不完整

        parser.feed(part2);
        assert(parser.hasRequest());

        auto r = parser.getRequest();

        std::cout << "[Test2] Half Packet OK → "
            << r.body << std::endl;

        assert(r.body == "hello=world");
    }

    // =========================
    // 测试3：粘包（两个请求一起）
    // =========================
    {
        HttpParser parser;

        std::string req =
            "POST /a HTTP/1.1\r\n"
            "Host: x\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "12345"
            "POST /b HTTP/1.1\r\n"
            "Host: y\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "abcde";

        parser.feed(req);

        assert(parser.hasRequest());
        auto r1 = parser.getRequest();

        assert(parser.hasRequest());
        auto r2 = parser.getRequest();

        std::cout << "[Test3] Sticky Packet OK\n";
        std::cout << "  Req1: " << r1.url << " BODY=" << r1.body << std::endl;
        std::cout << "  Req2: " << r2.url << " BODY=" << r2.body << std::endl;

        assert(r1.body == "12345");
        assert(r2.body == "abcde");
    }

    // =========================
    // 测试4：半包 + 粘包（真实网络）
    // =========================
    {
        HttpParser parser;

        std::string part1 =
            "POST /a HTTP/1.1\r\n"
            "Host: x\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "12";

        std::string part2 =
            "345POST /b HTTP/1.1\r\n"
            "Host: y\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "abcde";

        parser.feed(part1);
        assert(!parser.hasRequest());

        parser.feed(part2);

        assert(parser.hasRequest());
        auto r1 = parser.getRequest();

        assert(parser.hasRequest());
        auto r2 = parser.getRequest();

        std::cout << "[Test4] Mixed Packet OK\n";

        assert(r1.body == "12345");
        assert(r2.body == "abcde");
    }

    // =========================
    // 测试5：多次小块 feed（极端拆包）
    // =========================
    {
        HttpParser parser;

        std::string req =
            "POST /test HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Length: 11\r\n"
            "\r\n"
            "hello=world";

        // 模拟逐字符输入（最极端情况）
        for (char c : req) {
            parser.feed(std::string(1, c));
        }

        assert(parser.hasRequest());
        auto r = parser.getRequest();

        std::cout << "[Test5] Byte-by-Byte OK → "
            << r.body << std::endl;

        assert(r.body == "hello=world");
    }

    std::cout << "\nAll tests passed!\n";

    return 0;
}


//std::cout << "测试 1: 标准完整请求" << std::endl;
//parser.feed("GET /index/html HTTP/1.1\r\n");
//assert(parser.hasRequest()); // 断言解析成功
//HttpRequest req1 = parser.getRequest();
//std::cout << "Method: " << req1.method << ",URL:" << req1.url << std::endl;
//assert(req1.method == "GET");//验证测试的1方法

//std::cout << "测试 2: 半包处理 (数据分两次到达)" << std::endl;
//parser.feed("POST /api/login HTTP");  // 模拟第一次只收到前半段
//assert(!parser.hasRequest());         // 此时不应有请求
//parser.feed("/1.1\r\n");              // 模拟第二次收到后半段
//assert(parser.hasRequest());          // 此时应拼装完成
//HttpRequest req2 = parser.getRequest();
//std::cout << "Method: " << req2.method << ",URL:" << req2.url << std::endl;


//std::cout << "测试 3: 粘包处理 (多条请求同时到达)" << std::endl;
//// 模拟一次性收到两条紧密相连的请求
//parser.feed("PUT /file.txt HTTP/1.1\r\nDELETE /old.txt HTTP/1.1\r\n");

//// 验证第一条
//assert(parser.hasRequest());
//HttpRequest req3 = parser.getRequest();
//std::cout << "First: " << req3.method << ",URL:" << req3.url << std::endl;
//assert(req3.method == "PUT");

//// 验证第二条
//assert(parser.hasRequest());
//HttpRequest req4 = parser.getRequest();
//std::cout << "Second: " << req4.method << ",URL:" << req4.url << std::endl;
//assert(req4.method == "DELETE");