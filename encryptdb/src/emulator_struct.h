#pragma once
#include "tools.h"
#include "nlohmann/json.hpp"
#include <cstddef>
#include <gmpxx.h>
#include <gmp.h>
#include <pbcwrapper/PBC.h>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>

using std::vector;
using std::string;

template<typename T> 
struct MatrixElements {
    int n;
    int sqrt_n;
    vector<T> data;

    MatrixElements() noexcept {};
    MatrixElements(int n) noexcept { setN(n); }

    virtual void setN(int n) {
        this->n = n;
        sqrt_n = sqrt(n);
        if (n != sqrt_n * sqrt_n) {
            cout << "[warn] n is not a square number" << endl;
        }
        data.resize(n);
    }

    T& get(int i, int j) {
        return data[i * sqrt_n + j];
    }

    T& get(int idx) {
        return data[idx];
    }

    void set(int i, int j, const T& val) {
        data[i * sqrt_n + j] = val;
    }

    void set(int idx, const T& val) {
        data[idx] = val;
    }
};

struct GlobalParams {
    Pairing e;
    G1 g;
    GT gt;
    vector<G1> h;
    MatrixElements<G1> hh;
};

struct PublicKey {
    GT zx;
    G1 gx;
};

struct PrivateKey {
    Zr x1;
    Zr x2;
};

struct DO {
    PublicKey pk;
    PrivateKey sk;
};

struct CS {
    G1 rk;
};

struct QU {
    PublicKey pk;
    PrivateKey sk;
};

struct EdbElement {
    G1 gs;
    // Zr num;
    mpz_class num;
private:
    static long lowbit_len;
public:
    std::string toString() const {
        return gs.toString(true) + num.get_str(10);
    }
    mpz_class toMpzClass(bool is_debug = false) const;
    void fromMpzClass(const Pairing& e, const mpz_class& mpz_obj, bool is_debug = false);

    bool operator==(const EdbElement& other) {
        return gs == other.gs && num == other.num;
    }
    bool operator!=(const EdbElement& other) {
        return !(*this == other);
    }
};

struct Dataset
{
    MatrixElements<int> plain;
    MatrixElements<EdbElement> cipher;
    Dataset() noexcept {}

    void setN(int n) {
        plain.setN(n);
        cipher.setN(n);
    }

    void RetrieveDataset(const string& filename) {
        int n = get_n_from_filename(filename);
        setN(n);
        nlohmann::json json_obj;
        std::ifstream jfile(filename);
        jfile >> json_obj;
        json_obj.get_to(plain.data);
        jfile.close();
    }

    int db_get(int i, int j) {
        return plain.get(i, j);
    }

    int db_get(int idx) {
        return plain.get(idx);
    }

    void db_set(int i, int j, int val) {
        plain.set(i, j, val);
    }

    void db_set(int idx, int val) {
        plain.set(idx, val);
    }

    const EdbElement& edb_get(int i, int j) {
        return cipher.get(i, j);
    }

    const EdbElement& edb_get(int idx) {
        return cipher.get(idx);
    }

    void edb_set(int i, int j, const EdbElement& val) {
        cipher.set(i, j, val);
    }

    void edb_set(int idx, const EdbElement& val) {
        cipher.set(idx, val);
    }

    int getN() const { return plain.n; }
    int getSqrtN() const { return plain.sqrt_n; }
};

struct EmulatorParams
{
    std::string filename;
};

void emulator(const EmulatorParams*);
void testcode();