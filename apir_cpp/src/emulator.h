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
        setN(n);
        nlohmann::json json_obj;
        std::ifstream jfile(filename);
        jfile >> json_obj;
        json_obj.get_to(plain);
        jfile.close();
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
    long n;
};

void emulator(const EmulatorParams*);
void testcode();