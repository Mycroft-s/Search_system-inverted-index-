#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

// 生成倒排索引并分批保存
void parseDocuments(const std::string& filePath);

// 保存倒排索引
void saveInvertedIndex(const std::string& outputFile);

std::vector<std::string> tokenize(const std::string& text);
#endif
