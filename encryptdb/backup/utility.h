#pragma once
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <numeric>
#include <ostream>
#include <vector>
#include <random>
#include <string>
#include <fstream>
#include <iostream>
#include <gmp.h>

using namespace std;

namespace utility
{
vector<vector<int>> read_dataset(string filename);
double nbit_random(unsigned n);
double nbit_random_force(unsigned n);
vector<int> parse_dataset_path(const string& path);
vector<double> generate_query_random(const int d, int T = 50);
vector<int> generate_int_query_random(const int d, int T = 50);
vector<double> nbit_random_array(unsigned int bitlen, size_t n);
vector<double> nbit_random_array_force(unsigned int bitlen, size_t n);
vector<double> random_sign_array(size_t n);
double square_diff(double a, double b);
template<typename T>
void print_vector(const vector<T>& arr) {
    for (const auto i : arr) {
        cout << i << " ";
    }
    cout << endl;
}

#define EXECUTE_SECOND(start, end) ((double)(end-start)/CLOCKS_PER_SEC)

class MPZ_VECTOR {
private:
    mpz_t* _vec;
    size_t _size;
public:
    MPZ_VECTOR():_size(0), _vec(nullptr){}
    MPZ_VECTOR(const size_t);
    ~MPZ_VECTOR();
    const size_t size() const {return _size;};
    // void push_back(const mpz_t element);
    // void pop_back();
    mpz_t& operator[](const size_t);
    mpz_t* begin() const {return _vec;}
    mpz_t* end() const {return (_vec+_size);}

    MPZ_VECTOR(const MPZ_VECTOR& other) : MPZ_VECTOR(other._size) {
        for (int i(0); i < _size; ++i) {
            mpz_set(_vec[i], other._vec[i]);
        }
    }
    MPZ_VECTOR& operator=(const MPZ_VECTOR& other) {
        for (int i(0); i < _size; ++i) {
            mpz_clear(_vec[i]);
        }
        _size = other._size;
        _vec = (mpz_t*)realloc(_vec, _size * sizeof(mpz_t));
        for (int i(0); i < _size; ++i) {
            mpz_init_set(_vec[i], other._vec[i]);
        }
        return *this;
    }
};


struct MPZ_T{
    mpz_t ptr;
    MPZ_T(){
        mpz_init(ptr);
    }
    MPZ_T(unsigned long num) {
        mpz_init_set_ui(ptr, num);
    }
    MPZ_T(mpz_t num) {
        mpz_init_set(ptr, num);
    }
    ~MPZ_T(){mpz_clear(ptr);}
    void print()
    {
        gmp_printf("%Zd\n", ptr);
    }

    MPZ_T(const MPZ_T&) = delete;
    MPZ_T& operator=(const MPZ_T&) = delete;
    friend ostream& operator<<(ostream& os, const MPZ_T& a) {
        os << a.ptr;
        return os;
    }
};

struct BiMap {
    std::vector<int> pi;
    std::vector<int> rpi;
    size_t size;
    BiMap(size_t size)
        : size(size)
        , pi(size)
        , rpi(size) 
    {
        init();
    }

private:
    void init()
    {
        iota(pi.begin(), pi.end(), 0);
        random_device dev;
        mt19937 g(dev());
        shuffle(pi.begin(), pi.end(), g);
        for (int i(0); i < size; ++i) {
            rpi[pi[i]] = i;
        }
    }
};
} // namespace end;