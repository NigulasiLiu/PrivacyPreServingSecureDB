#pragma once
#include "tools.h"
#include "nlohmann/json.hpp"
#include <NTL/ZZ_p.h>
#include <cstddef>
#include <gmpxx.h>
#include <gmp.h>
#include <pbcwrapper/PBC.h>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <NTL/ZZ.h>


struct PublicKey {
    Pairing e;
    G1 g;
    GT gt;
    vector<G1> omega;
    NTL::ZZ p;
};

struct PrivateKey {
    NTL::ZZ_p alpha;
    NTL::ZZ k;
};

struct Dataset
{
    using EdbElement = long;

    std::vector<long> plain;
    std::vector<EdbElement> cipher;
    int n;

    Dataset() noexcept {}
    void setN(int n) {
        this->n = n;
        plain.resize(n);
        cipher.resize(n);
    }

    void RetrieveDataset(const string& filename) {
            int n = get_n_from_filename(filename);
            setN(n); // 这一步已经提前 resize 了 plain 数组

            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "无法打开数据集文件: " << filename << std::endl;
                exit(1);
            }

            std::string line;
            int idx = 0;
            // 逐行读取流式解析
            while (std::getline(file, line) && idx < n) {
                if (line.empty()) continue;

                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string hexValueStr = line.substr(colonPos + 1);
                    // 存入 ppvd 原生的 vector<long> plain 数组中
                    plain[idx] = std::stoull(hexValueStr, nullptr, 16);
                    idx++;
                }
            }
            file.close();
        }

    int db_get(int idx) {
        return plain.at(idx);
    }

    void db_set(int idx, int val) {
        plain[idx] = val;
    }

    const EdbElement& edb_get(int idx) {
        return cipher.at(idx);
    }

    void edb_set(int idx, const EdbElement& val) {
        cipher[idx] = val;
    }

    int getN() const { return n; }
};

struct EmulatorParams
{
    std::string filename;
};

void emulator(const EmulatorParams*);
void testcode();