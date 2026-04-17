#pragma once
#include <string>
#include <queue>
#include <sstream>

struct HttpRequest {
    std::string method; //HTTP请求方法:如"Get" "Post" "PUT"
    std::string url;    //请求的目标URL(包含路径和查询参数，不含协议域名)
    std::string version;//HTTP协议版本(如HTTP/1.1)
};

class HttpParser {
public:
    // 向解析器提供最新的数据块，可以触发解析 并产生0或多个完整请求
    void feed(const std::string& data);
    // 检查是否已有解析完成的请求
    bool hasRequest() const;
    // 从队列中取出最早的一个完整的HTTP请求
    HttpRequest getRequest();
private:
    // 尝试从缓冲区中解析一行（以\r\n结尾）作为请求行,成功则填充current_并返回true，否则返回false
    bool parseRequestLine();
    // 重置当前解析状态，清空缓冲区和当前的请求结构，准备下一次解析
    void reset() {current_ = HttpRequest();}
private:
    std::string buffer_;              // 内部缓冲区,用于积累未解析完成的数据
    HttpRequest current_;             // 当前正在解析的请求
    std::queue<HttpRequest> requests_;// 已解析完成的请求队列
};