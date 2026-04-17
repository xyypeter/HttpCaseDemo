#include "HttpParser.h"

// 将新数据追加到缓冲区，然后尝试解析尽可能多的完整请求行
void HttpParser::feed(const std::string& data){
    // 讲述添加只buffer中 
    buffer_ += data;
    // 只要缓冲区中还能解析出一个完整的请求行，就继续解析
    while (true) {
        if (!parseRequestLine()) break;
        requests_.push(current_); //将解析成功的请求放入队列中
        reset();                  //重置当前请求对象,准备解析下一个
    }
}

// 检查是否有解析完成的请求
bool HttpParser::hasRequest() const{
    return !requests_.empty();
}

// 从队列头部取出一个请求并移除
HttpRequest HttpParser::getRequest(){
    auto req = requests_.front();
    requests_.pop();
    return req;
}

// 请求行解析核心逻辑
bool HttpParser::parseRequestLine(){
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
