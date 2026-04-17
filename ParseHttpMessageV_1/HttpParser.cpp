#include "HttpParser.h"

//// 将新数据追加到缓冲区，然后尝试解析尽可能多的完整请求行
//void HttpParser::feed(const std::string& data) {
//    // 讲述添加只buffer中 
//    buffer_ += data;
//    // 只要缓冲区中还能解析出一个完整的请求行，就继续解析
//    while (true) {
//        if (!parseRequestLine()) break;
//        requests_.push(current_); //将解析成功的请求放入队列中
//        reset();                  //重置当前请求对象,准备解析下一个
//    }
//}

void HttpParser::feed(const std::string& data) {
    buffer_ += data;
    while (true) {
        bool progress = false;
        switch (state_) {
        case ParseState::REQUEST_LINE: {
            if (!parseRequestLine()) break; // 失败则退出循环等待更多数据
            state_ = ParseState::HEADERS;
            progress = true;
            break; // 跳出switch，进入下一次循环（状态已更新）
        }
        case ParseState::HEADERS: {
            if (!parseHeaders()) break;
            // 检查是否需要进入 BODY 状态
            if (current_.headers.count("Content-Length")) {
                content_length_ = std::stoi(current_.headers["Content-Length"]);
                state_ = ParseState::BODY;
            }
            else {
                // 无Body，直接完成请求
                requests_.push(current_);
                reset();
            }
            progress = true;
            break;
        }
        case ParseState::BODY: {
            if (!parseBody()) break;
            requests_.push(current_);
            reset();
            progress = true;
            break;
        }
        }
        if (!progress) break;
    }
}

// 检查是否有解析完成的请求
bool HttpParser::hasRequest() const {
    return !requests_.empty();
}

// 解析HTTP请求头部
bool HttpParser::parseHeaders(){
    // 1.查找头部结束标志：连续的两个 CRLF (\r\n\r\n)，即空行，此处解决了"半包"问题：若缓冲区中没有完整头部，立即返回 false 等待更多数据
    size_t pos = buffer_.find("\r\n\r\n");
    if (pos == std::string::npos)
        return false;
    // 2.提取与清理：截取头部字符串，并从缓冲区移除已处理部分（含分隔符共 +4 字节）
    std::string headers_str = buffer_.substr(0, pos);
    buffer_.erase(0, pos + 4);
    // 3. 逐行处理：使用字符串流遍历每一行头部字段
    std::istringstream stream(headers_str);
    std::string line;
    while (std::getline(stream, line)) {
        // 3.1 净化行尾：getline 默认去掉 \n，若行尾残留 \r（CRLF格式）则手动去除
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        // 3.2 定位分隔符：HTTP 头部采用 Key: Value 格式
        size_t colon = line.find(":");
        if (colon != std::string::npos) {
            // 提取键（Key）：冒号前的部分
            std::string key = line.substr(0, colon);
            // 提取值（Value）：冒号后的部分（可能以空格开头）
            std::string value = line.substr(colon + 1);
            // 3.3 修剪空格：规范要求 Value 前导空格需忽略（RFC 7230）
            if (!value.empty() && value[0] == ' ')
                value.erase(0, 1);
            // 3.4 存储映射：将键值对存入当前请求的头部字典
            current_.headers[key] = value;
        }
    }
    return true;
}

bool HttpParser::parseBody(){
    if (buffer_.size() < content_length_)
        return false;
    current_.body = buffer_.substr(0, content_length_);
    buffer_.erase(0, content_length_);
    return true;
}

// 从队列头部取出一个请求并移除
HttpRequest HttpParser::getRequest() {
    auto req = requests_.front();
    requests_.pop();
    return req;
}

// 请求行解析核心逻辑
bool HttpParser::parseRequestLine() {
    //查找终止符
    size_t pos = buffer_.find("\r\n");
    if (pos == std::string::npos) return false;
    //提取当前行(包括\r\n,所以下方+2)
    std::string line = buffer_.substr(0, pos);
    buffer_.erase(0, pos + 2);
    //字符提取:利用流操作自动按照空格提取
    std::istringstream iss(line);
    iss >> current_.method >> current_.url >> current_.version;
    return true;
}

void HttpParser::reset() { 
    current_ = HttpRequest(); 
    state_ = ParseState::REQUEST_LINE;
    content_length_ = 0;
}