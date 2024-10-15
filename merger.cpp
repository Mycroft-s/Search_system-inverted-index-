#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <set>

// �����������ݽṹ
std::unordered_map<std::string, std::vector<int>> mergedIndex;

// ��ȡ�������������ļ����ϲ�����������
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
            // ֱ�ӽ��ĵ�ID�����Ӧ�Ĵ����ĵ���������
            while (iss >> docID) {
                mergedIndex[term].push_back(docID);
            }
        }
        file.close();
    }
}

// �Ժϲ���ĵ���������������ȥ��
void finalizeInvertedIndex() {
    for (auto& entry : mergedIndex) {
        // ʹ�� std::set ���������ȥ��
        std::set<int> uniqueDocIDs(entry.second.begin(), entry.second.end());
        entry.second.assign(uniqueDocIDs.begin(), uniqueDocIDs.end());  // ��������¸�ֵ������
    }
}

// VarByte ���뺯�������ĵ�IDѹ�����ֽ���
void varByteEncode(int number, std::vector<uint8_t>& encodedBytes) {
    while (true) {
        uint8_t byte = number & 0x7F;  // ȡ��͵�7λ
        number >>= 7;
        if (number == 0) {
            encodedBytes.push_back(byte | 0x80);  // ���λ��Ϊ1��ʾ����
            break;
        }
        else {
            encodedBytes.push_back(byte);  // ���λΪ0����ʾ���и����ֽ�
        }
    }
}

// �������յĵ����������������ļ���ʹ�� VarByte ���룩
void saveFinalInvertedIndex(const std::string& outputFile) {
    std::ofstream outFile(outputFile, std::ios::binary);  // �Զ�����ģʽ���ļ�
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing: " << outputFile << std::endl;
        return;
    }

    for (const auto& entry : mergedIndex) {
        // д�����
        size_t termSize = entry.first.size();
        outFile.write(reinterpret_cast<const char*>(&termSize), sizeof(size_t));  // д���������
        outFile.write(entry.first.c_str(), termSize);  // д���������

        // ��ÿ���ĵ�ID���� VarByte ���벢д���ļ�
        size_t numDocIDs = entry.second.size();
        outFile.write(reinterpret_cast<const char*>(&numDocIDs), sizeof(size_t));  // д���ĵ�ID����

        // ʹ��һ��Ԥ�ȷ�����������洢�������ֽ�
        std::vector<uint8_t> encodedBytes;
        for (int docID : entry.second) {
            varByteEncode(docID, encodedBytes);
        }
        // һ����д��������ֽ�
        outFile.write(reinterpret_cast<const char*>(encodedBytes.data()), encodedBytes.size());
    }

    outFile.close();
    std::cout << "[INFO] Final compressed inverted index saved to " << outputFile << std::endl;
}

