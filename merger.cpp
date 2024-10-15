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

// 在合并过程中生成 Lexicon
void saveMergedInvertedIndexAndGenerateLexicon(const std::string& outputFile, const std::string& lexiconFile) {
    std::ofstream outFile(outputFile, std::ios::binary);  // 以二进制模式打开倒排索引文件
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open output file for writing: " << outputFile << std::endl;
        return;
    }

    std::ofstream lexiconOut(lexiconFile);  // 打开词典文件
    if (!lexiconOut.is_open()) {
        std::cerr << "Error: Unable to open lexicon file for writing: " << lexiconFile << std::endl;
        return;
    }

    int64_t currentOffset = 0;  // 用于记录当前倒排列表在文件中的偏移量

    // 遍历已合并的倒排索引
    for (const auto& entry : mergedIndex) {
        const std::string& term = entry.first;
        const std::vector<int>& docIDs = entry.second;

        // 写入词条到倒排索引文件
        size_t termSize = term.size();
        outFile.write(reinterpret_cast<const char*>(&termSize), sizeof(size_t));  // 写入词条长度
        outFile.write(term.c_str(), termSize);  // 写入词条

        // 写入文档ID数量
        size_t numDocIDs = docIDs.size();
        outFile.write(reinterpret_cast<const char*>(&numDocIDs), sizeof(size_t));  // 写入文档ID数量

        // 对文档ID进行 VarByte 编码
        std::vector<uint8_t> encodedBytes;
        for (int docID : docIDs) {
            varByteEncode(docID, encodedBytes);
        }
        outFile.write(reinterpret_cast<const char*>(encodedBytes.data()), encodedBytes.size());  // 写入编码后的字节

        // 更新词典，记录词条对应的偏移量和倒排列表的字节长度
        int64_t newOffset = outFile.tellp();  // 获取当前写入位置作为新的偏移量
        int32_t length = static_cast<int32_t>(newOffset - currentOffset);  // 计算倒排列表的长度

        // 将词条及其对应的偏移量和长度写入词典
        lexiconOut << term << " " << currentOffset << " " << length << std::endl;

        // 更新当前偏移量
        currentOffset = newOffset;
    }

    outFile.close();
    lexiconOut.close();
    std::cout << "[INFO] Merged inverted index and lexicon generated." << std::endl;
}

