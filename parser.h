#ifndef PARSER_H
#define PARSER_H

#include <string>

// 生成倒排索引并分批保存
void parseDocuments(const std::string& filePath);

// 保存倒排索引
void saveInvertedIndex(const std::string& outputFile);

#endif
