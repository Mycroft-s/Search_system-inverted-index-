#ifndef MERGER_H
#define MERGER_H

#include <string>
#include <vector>

// 读取并合并多个倒排索引
void mergeInvertedIndexes(const std::vector<std::string>& indexFiles);

// 排序并去重
void finalizeInvertedIndex();
void saveMergedInvertedIndexAndGenerateLexicon(const std::string& outputFile, const std::string& lexiconFile);
#endif
