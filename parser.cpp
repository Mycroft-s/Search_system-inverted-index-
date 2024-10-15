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

void updateInvertedIndex(const std::vector<std::string>& tokens, int docID) {
    for (const std::string& token : tokens) {
        invertedIndex[token].push_back(docID);
    }
}

void parseDocuments(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string line;
    int docID = 0;
    int batchSize = 100000;  // 每批处理100000行

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    while (std::getline(file, line)) {
        docID++;

        std::istringstream ss(line);
        std::string passageID, passageText;
        std::getline(ss, passageID, '\t');
        std::getline(ss, passageText, '\t');

        std::vector<std::string> tokens = tokenize(passageText);
        updateInvertedIndex(tokens, docID);

        // 每处理一批数据，保存一次倒排索引，并输出日志信息
        if (docID % batchSize == 0) {
            std::string outputFileName = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_" + std::to_string(docID) + ".txt";
            saveInvertedIndex(outputFileName);
            invertedIndex.clear();  // 清空内存中的倒排索引

            // 输出处理进度日志
            std::cout << "[INFO] Processed " << docID << " documents, saved intermediate index to " << outputFileName << std::endl;
        }
    }

    // 处理剩余的索引
    if (!invertedIndex.empty()) {
        std::string finalOutputFile = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_final.txt";
        saveInvertedIndex(finalOutputFile);

        // 输出日志信息
        std::cout << "[INFO] Processed " << docID << " documents in total, saved final index to " << finalOutputFile << std::endl;
    }

    file.close();
}

void saveInvertedIndex(const std::string& outputFile) {
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing: " << outputFile << std::endl;
        return;
    }

    for (const auto& entry : invertedIndex) {
        outFile << entry.first << ": ";
        for (int docID : entry.second) {
            outFile << docID << " ";
        }
        outFile << "\n";
    }

    outFile.close();
}
