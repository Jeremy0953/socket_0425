#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "openssl/md5.h"
#include "openssl/sha.h"

// 定义加密函数指针类型
typedef void (*HashFunction)(const std::string&, std::string&);
void trim(std::string& str) {
    const char* whitespace = " \t\n\r\f\v";
    str.erase(0, str.find_first_not_of(whitespace));
    str.erase(str.find_last_not_of(whitespace) + 1);
}

// 将哈希值转换为十六进制字符串
void hashToHex(const unsigned char *hash, size_t len, std::string &encodedHexStr) {
    char buf[129] = {0};
    for (size_t i = 0; i < len; i++) {
        sprintf(buf + i * 2, "%02x", hash[i]);
    }
    encodedHexStr = std::string(buf);
}

// MD5摘要哈希函数
void md5(const std::string &srcStr, std::string &encodedHexStr) {
    unsigned char mdStr[16]; // MD5生成16字节的哈希值
    MD5((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);
    hashToHex(mdStr, sizeof(mdStr), encodedHexStr);
}

// SHA256摘要哈希函数
void sha256(const std::string &srcStr, std::string &encodedHexStr) {
    unsigned char shaStr[32]; // SHA256生成32字节的哈希值
    SHA256((const unsigned char *)srcStr.c_str(), srcStr.length(), shaStr);
    hashToHex(shaStr, sizeof(shaStr), encodedHexStr);
}

// 处理输入文件，加密并输出到文件
void processFile(const std::string &inputFile, const std::string &outputFile, HashFunction hashFunc) {
    std::ifstream file(inputFile);
    std::ofstream out(outputFile);
    std::string line;
    
    while (getline(file, line)) {
        std::istringstream ss(line);
        std::string name, password, name_hash, pw_hash;
        
        getline(ss, name, ',');
        getline(ss, password);
        trim(password);
        hashFunc(name, name_hash);
        hashFunc(password, pw_hash);
        out<<name_hash<<", "<<pw_hash<<"\n";
    }

    file.close();
    out.close();
}

int main() {
    processFile("member_unencrypted.txt", "member_md5.txt", md5);
    processFile("member_unencrypted.txt", "member_sha256.txt", sha256);
    std::cout << "Encryption complete. Check 'member_md5.txt' and 'member_sha256.txt' for results." << std::endl;
    return 0;
}
