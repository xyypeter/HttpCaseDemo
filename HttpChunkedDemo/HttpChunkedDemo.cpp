
#include <iostream>
#include <string>

std::string parseChunked(const std::string& rawData) {
    std::string result;
    size_t pos = 0;
    while (true) {
        //1.寻找第一个\r\n
        size_t lineEnd = rawData.find("\r\n", pos);
        if (lineEnd == std::string::npos) break;
        //2.取出ChunkSize
        std::string sizeHex = rawData.substr(pos, lineEnd - pos);
        //3.转为十六进制
        int size = 0;
        try {
            size = std::stoi(sizeHex, nullptr, 16);
        }catch (...) {
            std::cerr << "Invalid chunk size\n";
            break;
        }
        pos = lineEnd + 2;//跳过\r\n  -----移动至 chunkData

        // 4.结束判断--chunkSize = 0
        if (size == 0) {
            // 必须确保后面还有 "\r\n"
            if (pos + 2 <= rawData.size() && rawData.substr(pos, 2) == "\r\n"){
                pos += 2; // consume final CRLF
            }else {
                std::cerr << "Invalid chunk terminator (missing \\r\\n\\r\\n)\n";
            }
            break;
        }

        //5.检查数据长度--防止数据不完整
        if (pos + size > rawData.size()) {
            std::cerr << "Incomplete chunk data\n";
            break;
        }
        
        //6.取数据
        result.append(rawData.substr(pos, size));
        pos += size;//--此处已经跳到 chunkData

        //7.检查chunk后的\r\n
        if (rawData.substr(pos, 2) != "\r\n") {
            std::cerr << "Missing CRLF after chunk data\n";
            break;
        }
        pos += 2;
    }
    return result;
}

int main(){
    std::string raw =
        "4\r\nWiki\r\n"
        "5\r\npedia\r\n"
        "0\r\n\r\n";
    std::string result = parseChunked(raw);
    std::cout << "Result: " << result << std::endl;
    return 0;
}


