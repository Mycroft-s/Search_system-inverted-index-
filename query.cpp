#include "query.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <set>
#include <cmath>

// 用于存储词典（term -> 文件偏移量）的数据结构
std::unordered_map<std::string, int64_t> lexicon;

// 用于存储倒排索引缓存，避免频繁磁盘读取
std::unordered_map<std::string, std::vector<int>> invertedIndexCache;

// 用于加载词典
void loadLexicon(const std::string& lexiconFile) {
    std::ifstream inFile(lexiconFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error opening lexicon file!" << std::endl;
        return;
    }

    std::string term;
    int64_t offset;

    while (inFile >> term >> offset) {
        lexicon[term] = offset;
    }

    inFile.close();
}

// 加载倒排列表，根据词条从文件中读取相应的文档ID
std::vector<int> loadInvertedList(const std::string& term, const std::string& indexFilePath) {
    // 如果词条在缓存中，直接返回
    if (invertedIndexCache.find(term) != invertedIndexCache.end()) {
        return invertedIndexCache[term];
    }

    std::ifstream indexFile(indexFilePath, std::ios::binary);
    if (!indexFile.is_open()) {
        std::cerr << "Error opening index file: " << indexFilePath << std::endl;
        return {};
    }

    // 在词典中查找词条的偏移量
    auto it = lexicon.find(term);
    if (it == lexicon.end()) {
        std::cerr << "Term not found in lexicon: " << term << std::endl;
        return {};
    }

    // 定位到倒排列表在文件中的偏移量
    int64_t offset = it->second;
    indexFile.seekg(offset, std::ios::beg);

    // 假设每个倒排列表存储的是文档ID的数量，后面跟着文档ID列表
    size_t numDocIDs;
    indexFile.read(reinterpret_cast<char*>(&numDocIDs), sizeof(size_t));

    std::vector<int> docIDs(numDocIDs);
    indexFile.read(reinterpret_cast<char*>(docIDs.data()), numDocIDs * sizeof(int));

    indexFile.close();

    // 将读取的倒排列表存入缓存
    invertedIndexCache[term] = docIDs;
    return docIDs;
}

// 计算BM25评分
double computeBM25(int termFrequency, int docFrequency, int totalDocuments, int documentLength, int avgDocumentLength) {
    double k1 = 1.5;  // BM25 参数
    double b = 0.75;  // BM25 参数
    double idf = std::log((totalDocuments - docFrequency + 0.5) / (docFrequency + 0.5) + 1);

    double tfComponent = (termFrequency * (k1 + 1)) / (termFrequency + k1 * (1 - b + b * (documentLength / avgDocumentLength)));
    return idf * tfComponent;
}

// 处理查询，加载相应的倒排列表并计算BM25评分
void processQuery(const std::string& query, const std::string& indexFilePath, bool conjunctive) {
    // 分词处理
    std::vector<std::string> terms = tokenize(query);
    std::unordered_map<int, double> docScores;  // 存储每个文档的BM25评分

    if (terms.empty()) {
        std::cout << "No terms found in query." << std::endl;
        return;
    }

    std::vector<int> results;

    if (conjunctive) {  // 合取查询：找到同时包含所有词条的文档
        results = loadInvertedList(terms[0], indexFilePath);  // 先加载第一个词条的倒排列表
        for (size_t i = 1; i < terms.size(); ++i) {
            std::vector<int> otherList = loadInvertedList(terms[i], indexFilePath);
            std::vector<int> intersection;
            std::set_intersection(results.begin(), results.end(), otherList.begin(), otherList.end(),
                std::back_inserter(intersection));
            results = intersection;  // 保留同时出现的文档
        }
    }
    else {  // 析取查询：合并包含任意词条的文档
        std::unordered_map<int, int> docCount;  // 统计每个文档的出现次数
        for (const std::string& term : terms) {
            std::vector<int> docIDs = loadInvertedList(term, indexFilePath);
            for (int docID : docIDs) {
                docCount[docID]++;
            }
        }

        for (const auto& pair : docCount) {
            results.push_back(pair.first);  // 合并文档ID
        }
    }

    // 假设我们有一些文档频率、文档长度等信息用于计算BM25
    int totalDocuments = 10000;  // 假设总共有10000个文档
    int avgDocumentLength = 500;  // 假设平均文档长度

    // 遍历结果并计算BM25评分
    for (int docID : results) {
        int termFrequency = 1;  // 这里假设每个词条在文档中的词频是1（实际应该根据倒排列表中的信息）
        int docFrequency = 100;  // 这里假设词条在100个文档中出现
        int documentLength = 500;  // 假设文档长度为500

        double score = computeBM25(termFrequency, docFrequency, totalDocuments, documentLength, avgDocumentLength);
        docScores[docID] = score;
    }

    // 输出BM25评分前10的文档
    std::vector<std::pair<int, double>> sortedResults(docScores.begin(), docScores.end());
    std::sort(sortedResults.begin(), sortedResults.end(),
        [](const std::pair<int, double>& a, const std::pair<int, double>& b) { return a.second > b.second; });

    std::cout << "Top 10 documents:" << std::endl;
    for (size_t i = 0; i < std::min(sortedResults.size(), size_t(10)); ++i) {
        std::cout << "DocID: " << sortedResults[i].first << ", Score: " << sortedResults[i].second << std::endl;
    }
}

// 查询处理器，等待用户输入查询
void startQueryProcessor() {
    // 加载词典
    std::string lexiconFile = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\lexicon.bin";
    loadLexicon(lexiconFile);

    std::string indexFilePath = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\final_inverted_index.bin";

    std::string query;
    while (true) {
        std::cout << "Enter query (or 'exit' to quit): ";
        std::getline(std::cin, query);
        if (query == "exit") break;

        std::cout << "Select mode (1: conjunctive, 2: disjunctive): ";
        std::string mode;
        std::getline(std::cin, mode);
        bool conjunctive = (mode == "1");

        processQuery(query, indexFilePath, conjunctive);  // 处理查询
    }
}
