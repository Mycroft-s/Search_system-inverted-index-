#include "parser.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

// 倒排索引数据结构
std::unordered_map<std::string, std::vector<int>> invertedIndex;

// 文档表：DocID -> 文档名称 (URL)
std::unordered_map<int, std::string> pageTable;

// Lexicon 数据结构：词条 -> (倒排列表起始位置，倒排列表长度)
std::unordered_map<std::string, std::pair<int64_t, int32_t>> lexicon;

// 每个文档的长度：docID -> 文档长度
std::unordered_map<int, int> documentLengths;

// 每个词条的文档频率：term -> 文档频率
std::unordered_map<std::string, int> docFrequencyMap;

// 分词函数
std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(text);

    while (tokenStream >> token) {
        for (char& c : token) {
            if (std::ispunct(static_cast<unsigned char>(c))) {
                c = ' ';
            }
        }
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        tokens.push_back(token);
    }
    return tokens;
}

// 更新倒排索引
void updateInvertedIndex(const std::vector<std::string>& tokens, int docID) {
    std::unordered_map<std::string, bool> seenInDoc; // 用于记录某个词条是否已经在该文档中出现过

    for (const std::string& token : tokens) {
        invertedIndex[token].push_back(docID);

        // 如果该词条第一次在该文档中出现，更新文档频率
        if (!seenInDoc[token]) {
            docFrequencyMap[token]++;
            seenInDoc[token] = true;
        }
    }
}

// 保存文档表
void savePageTable(const std::string& pageTableFile) {
    std::ofstream outFile(pageTableFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing: " << pageTableFile << std::endl;
        return;
    }

    for (const auto& entry : pageTable) {
        int docID = entry.first;
        const std::string& docName = entry.second;
        size_t nameLength = docName.size();

        outFile.write(reinterpret_cast<const char*>(&docID), sizeof(docID));
        outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        outFile.write(docName.c_str(), nameLength);
    }

    outFile.close();
}

// 保存倒排索引
void saveInvertedIndex(const std::string& outputFile) {
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing: " << outputFile << std::endl;
        return;
    }

    int64_t currentOffset = outFile.tellp();  // 当前偏移量

    for (const auto& entry : invertedIndex) {
        const std::string& term = entry.first;
        const std::vector<int>& docIDs = entry.second;
        int32_t listSize = docIDs.size() * sizeof(int);  // 倒排列表的字节大小

        outFile << term << ": ";
        for (int docID : docIDs) {
            outFile << docID << " ";
        }
        outFile << "\n";

        int64_t newOffset = outFile.tellp();  // 新的偏移量

        // 更新词典（Lexicon）：记录每个词条的位置和倒排列表长度
        lexicon[term] = { currentOffset, static_cast<int32_t>(newOffset - currentOffset) };
        currentOffset = newOffset;
    }

    outFile.close();
}

// 解析文档并生成倒排索引、文档表和词典
void parseDocuments(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string line;
    int docID = 0;
    int batchSize = 100000;
    int totalDocumentLength = 0;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    while (std::getline(file, line)) {
        docID++;

        std::istringstream ss(line);
        std::string passageID, passageText;
        std::getline(ss, passageID, '\t');  // 文档ID
        std::getline(ss, passageText, '\t');  // 文档内容

        pageTable[docID] = passageID;  // 将 DocID 和文档名称存入 pageTable

        std::vector<std::string> tokens = tokenize(passageText);
        int docLength = tokens.size();
        documentLengths[docID] = docLength;  // 记录文档长度
        totalDocumentLength += docLength;  // 累加文档总长度

        updateInvertedIndex(tokens, docID);

        if (docID % batchSize == 0) {
            std::string outputFileName = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_" + std::to_string(docID) + ".txt";
            saveInvertedIndex(outputFileName);
            invertedIndex.clear();  // 清空内存中的倒排索引

            std::cout << "[INFO] Processed " << docID << " documents, saved intermediate index to " << outputFileName << std::endl;
        }
    }

    if (!invertedIndex.empty()) {
        std::string finalOutputFile = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_final.txt";
        saveInvertedIndex(finalOutputFile);
        std::cout << "[INFO] Processed " << docID << " documents in total, saved final index to " << finalOutputFile << std::endl;
    }

    file.close();

    // 保存文档表
    savePageTable("C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\page_table.bin");
    std::cout << "[INFO] Page table saved successfully." << std::endl;

    // 计算总文档数和平均文档长度
    int totalDocuments = docID;
    int avgDocumentLength = totalDocumentLength / totalDocuments;
    std::cout << "Total Documents: " << totalDocuments << std::endl;
    std::cout << "Average Document Length: " << avgDocumentLength << std::endl;

    // 输出词条的文档频率
    for (const auto& entry : docFrequencyMap) {
        std::cout << "Term: " << entry.first << ", Document Frequency: " << entry.second << std::endl;
    }
}
