#include "utility.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <gmp.h>
#include <string>
#include <vector>
#include <libgen.h>

namespace utility {
static gmp_randstate_t state;
static bool is_inited = false;

vector<vector<int>> read_dataset(string filename) {
    vector<vector<int>> ret;
    nlohmann::json json_obj;
    ifstream jfile(filename);
    jfile >> json_obj;
    json_obj.get_to(ret);
    jfile.close();
    return ret;
}

void init_random_state() {
    gmp_randinit_default(state);
    gmp_randseed_ui(state, clock());
}

double nbit_random(unsigned int n) {
    if (!is_inited) {
        init_random_state();
        is_inited = true;
    }
    mpz_t r;
    mpz_init(r);
    mpz_urandomb(r, state, n);
    double ret = mpz_get_d(r);
    mpz_clear(r);
    return ret;
}

double nbit_random_force(unsigned int n) {
    if (!is_inited) {
        init_random_state();
        is_inited = true;
    }
    mpz_t r;
    mpz_init(r);
    mpz_urandomb(r, state, n);
    mpz_t mask;
    mpz_init_set_ui(mask, 1);
    mpz_mul_2exp(mask, mask, n-1);
    
    mpz_ior(r, r, mask);
    double ret = mpz_get_d(r);

    mpz_clear(r);
    mpz_clear(mask);

    return ret;
}

vector<int> parse_dataset_path(const string& path) {
    vector<int> ret;
    ret.reserve(3);
    char *buf = new char[path.size()+1];
    strcpy(buf, path.c_str());
    string bn = basename(buf);
    int e_idx;
    while ((e_idx = bn.find("-")) != bn.npos || (e_idx = bn.find(".")) != bn.npos) {
        ret.push_back(stoi(bn.substr(0, e_idx)));
        bn = bn.substr(e_idx+1);
    }
    return ret;
}

vector<double> generate_query_random(const int d, int T) {
    vector<double> ret;
    ret.reserve(d);
    srand(clock());
    for(int i(0); i < d; ++i) {
        ret.emplace_back(rand() % (T));
    }
    return ret;
}

vector<int> generate_int_query_random(const int d, int T) {
    vector<int> ret(d);
    srand(time(nullptr));
    for (int i(0); i < d; ++i) {
        ret[i] = rand() % T;
    }
    return ret;
}

vector<double> nbit_random_array(unsigned int bitlen, size_t n) {
    vector<double> ret(n);
    for(size_t i(0); i < n; ++i) {
        ret[i] = nbit_random(bitlen);
    }
    return ret;
}

vector<double> nbit_random_array_force(unsigned int bitlen, size_t n) {
    vector<double> ret(n);
    for(size_t i(0); i < n; ++i) {
        ret[i] = nbit_random_force(bitlen);
    }
    return ret;
}

inline int flip_coin() {
    return nbit_random(1) ? 1 : -1;
}

vector<double> random_sign_array(size_t n) {
    vector<double> ret(n);
    for(int i(0); i < n; ++i) {
        ret[i] = flip_coin() == 1 ? 1.0 : -1.0;
    }
    return ret;
}

double square_diff(double a, double b) {
    return a*a - b*b;
}

MPZ_VECTOR::MPZ_VECTOR(const size_t n):_size(n) {
    _vec = (mpz_t*)malloc(sizeof(mpz_t) * _size);
    for(int i(0); i < _size; ++i) {
        mpz_init(_vec[i]);
    }
}

MPZ_VECTOR::~MPZ_VECTOR() {
    for(int i(0); i < _size; ++i) {
        mpz_clear(_vec[i]);
    }
    free(_vec);
}

// void MPZ_VECTOR::push_back(const mpz_t element) {
//     mpz_t tmp;
//     mpz_init_set(tmp, element);
//     _vec.push_back(tmp);
// }

// void MPZ_VECTOR::pop_back() {
//     if (!size()) return;
//     mpz_clear(_vec.back());
//     _vec.pop_back();
// }

mpz_t& MPZ_VECTOR::operator[](const size_t idx) {
    return _vec[idx];
}
}