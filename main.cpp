#include "parser.h"
#include "merger.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    /*
    // ��һ�������ɵ�������
    std::string inputFilePath = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\collection\\collection.tsv";
    parseDocuments(inputFilePath);
*/
    // �ڶ������ϲ���������
    // ��̬�����ļ�������ӵ� indexFiles �б���
    std::vector<std::string> indexFiles;
    for (int i = 100000; i <= 8800000; i += 100000) {
        std::string fileName = "C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_" + std::to_string(i) + ".txt";
        indexFiles.push_back(fileName);
    }

    // ��������ļ� inverted_index_final.txt
    indexFiles.push_back("C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\inverted_index_final.txt");


    mergeInvertedIndexes(indexFiles);

    finalizeInvertedIndex();
    saveFinalInvertedIndex("C:\\Users\\Admin\\Desktop\\web search\\week4-6\\index\\final_inverted_index.bin");

    std::cout << "Inverted index generation and merging completed successfully!" << std::endl;

    return 0;
}
// Compare this snippet from Web%20Hm2/main.cpp: