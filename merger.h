#ifndef MERGER_H
#define MERGER_H

#include <string>
#include <vector>

// ��ȡ���ϲ������������
void mergeInvertedIndexes(const std::vector<std::string>& indexFiles);

// ����ȥ��
void finalizeInvertedIndex();
void saveFinalInvertedIndex(const std::string& outputFile);
#endif

