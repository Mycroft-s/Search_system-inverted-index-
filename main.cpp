#include "parser.h"
#include "merger.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    /*
    // 第一步：生成倒排索引
    std::string inputFilePath = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\collection\\collection.tsv";
    parseDocuments(inputFilePath);
*/
    // 第二步：合并倒排索引
    // 动态生成文件名并添加到 indexFiles 列表中
    std::vector<std::string> indexFiles;
    for (int i = 100000; i <= 8800000; i += 100000) {
        std::string fileName = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_" + std::to_string(i) + ".txt";
        indexFiles.push_back(fileName);
    }

    // 添加最后的文件 inverted_index_final.txt
    indexFiles.push_back("C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_final.txt");


    mergeInvertedIndexes(indexFiles);

    finalizeInvertedIndex();
    saveFinalInvertedIndex("C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\final_inverted_index.bin");

    std::cout << "Inverted index generation and merging completed successfully!" << std::endl;

    return 0;
}
// Compare this snippet from Web%20Hm2/main.cpp: