#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <set>

// 倒排索引数据结构
std::unordered_map<std::string, std::vector<int>> mergedIndex;

// 读取单个倒排索引文件并合并到主索引中
void mergeInvertedIndexes(const std::vector<std::string>& indexFiles) {
    for (const std::string& filePath : indexFiles) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            continue;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string term;
            iss >> term;

            int docID;
            // 直接将文档ID插入对应的词条的倒排索引中
            while (iss >> docID) {
                mergedIndex[term].push_back(docID);
            }
        }
        file.close();
    }
}

// 对合并后的倒排索引进行排序并去重
void finalizeInvertedIndex() {
    for (auto& entry : mergedIndex) {
        // 使用 std::set 代替排序和去重
        std::set<int> uniqueDocIDs(entry.second.begin(), entry.second.end());
        entry.second.assign(uniqueDocIDs.begin(), uniqueDocIDs.end());  // 将结果重新赋值回向量
    }
}

// VarByte 编码函数：将文档ID压缩成字节流
void varByteEncode(int number, std::vector<uint8_t>& encodedBytes) {
    while (true) {
        uint8_t byte = number & 0x7F;  // 取最低的7位
        number >>= 7;
        if (number == 0) {
            encodedBytes.push_back(byte | 0x80);  // 最高位设为1表示结束
            break;
        }
        else {
            encodedBytes.push_back(byte);  // 最高位为0，表示还有更多字节
        }
    }
}

// 保存最终的倒排索引到二进制文件（使用 VarByte 编码）
void saveFinalInvertedIndex(const std::string& outputFile) {
    std::ofstream outFile(outputFile, std::ios::binary);  // 以二进制模式打开文件
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing: " << outputFile << std::endl;
        return;
    }

    for (const auto& entry : mergedIndex) {
        // 写入词条
        size_t termSize = entry.first.size();
        outFile.write(reinterpret_cast<const char*>(&termSize), sizeof(size_t));  // 写入词条长度
        outFile.write(entry.first.c_str(), termSize);  // 写入词条本身

        // 对每个文档ID进行 VarByte 编码并写入文件
        size_t numDocIDs = entry.second.size();
        outFile.write(reinterpret_cast<const char*>(&numDocIDs), sizeof(size_t));  // 写入文档ID数量

        // 使用一个预先分配的向量来存储编码后的字节
        std::vector<uint8_t> encodedBytes;
        for (int docID : entry.second) {
            varByteEncode(docID, encodedBytes);
        }
        // 一次性写入编码后的字节
        outFile.write(reinterpret_cast<const char*>(encodedBytes.data()), encodedBytes.size());
    }

    outFile.close();
    std::cout << "[INFO] Final compressed inverted index saved to " << outputFile << std::endl;
}

