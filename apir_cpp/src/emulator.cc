#include "emulator.h"
#include "merkel.h"
#include "tools.h"
#include <NTL/ZZ.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace NTL;
using namespace std;

string filename = "../dataset/n3-byte1.json";
// Dataset dataset;
long n = 256;

int bit_length = 512;
gmp_randclass gmp_r(gmp_randinit_default);
mpz_class d = 1;
mpz_class p;
mpz_class g_r;
mpz_class g_t;
vector<mpz_class> h;

mpz_class get_nbit_primer(int n) {
    mpz_class ret = gmp_r.get_z_bits(n);
    mpz_nextprime(ret.get_mpz_t(), ret.get_mpz_t());
    return ret;
}

void Setup()
{
    gmp_r.seed(time(nullptr));
    p = get_nbit_primer(bit_length);
}

void EncodeDB()
{
    h.reserve(n);
    d = 1;
    for (long i(0); i < n; ++i) {
        mpz_class val = i & 1;
        h.emplace_back(gmp_r.get_z_bits(bit_length) % p);
        mpz_class tmp;
        mpz_powm(tmp.get_mpz_t(), h.back().get_mpz_t(), 
            val.get_mpz_t(), p.get_mpz_t());
        d = (d * tmp) % p;
    }
}

vector<mpz_class> g_q;
vector<mpz_class> EncodeIndex(int idx)
{
    g_r = get_nbit_primer(bit_length-1);
    g_t = get_nbit_primer(bit_length-1);
    mpz_class rt = g_r + g_t;
    vector<mpz_class> ret;
    ret.reserve(n);
    for (long i(0); i < n; ++i) {
        mpz_class tmp;
        if (i != idx) {
            mpz_powm(tmp.get_mpz_t(), h[i].get_mpz_t(), g_r.get_mpz_t(), p.get_mpz_t());
            // tmp = (tmp * d) % p;
        } else {
            mpz_powm(tmp.get_mpz_t(), h[i].get_mpz_t(), rt.get_mpz_t(), p.get_mpz_t());
        }
        ret.emplace_back(tmp);
    }
    if (g_q.empty()) {
        g_q = std::move(ret);
    }
    return {};
}

// return [sigma_vi, pi]
mpz_class Query(vector<mpz_class>& q)
{
    mpz_class ret = 1;
    for (long i(0); i < n; ++i) {
        mpz_class tmp;
        mpz_powm(tmp.get_mpz_t(), q[i].get_mpz_t(), 
            mpz_class(i&1).get_mpz_t(), 
            p.get_mpz_t());
        ret = (ret * tmp) % p;
    }
    return ret;
}

bool Verify()
{
    bool res = true;
    // Zr alpha = MpzClasstoZr(pk.e, ZZptoMpzClass(sk.alpha));
    // // alpha.dump(stdout, "alpha ", 10);
    // Zr zr_sigma_i = MpzClasstoZr(pk.e, sigma_i);
    // // zr_sigma_i.dump(stdout, "zr_sigma_i ", 10);
    // Zr zr_sigma_vi = MpzClasstoZr(pk.e, sigma_vi);
    // // zr_sigma_vi.dump(stdout, "zr_sigma_vi ", 10);

    // res = pk.e(delta, pk.g) == pk.e(pi, (pk.g ^ alpha)/(pk.g ^ zr_sigma_i)) * pk.e(pk.g^zr_sigma_vi, pk.g);
    return res;
}

long DecodeResult(long idx, mpz_class a)
{
    mpz_class inv_d;
    mpz_invert(inv_d.get_mpz_t(), d.get_mpz_t(), p.get_mpz_t());
    mpz_class m;
    mpz_powm(m.get_mpz_t(), inv_d.get_mpz_t(), g_r.get_mpz_t(), p.get_mpz_t());
    m = (m * a) % p;
    long ret = -1;
    mpz_class should_m;
    mpz_powm(should_m.get_mpz_t(), h[idx].get_mpz_t(), g_t.get_mpz_t(), p.get_mpz_t());
    if (m == 1) {
        ret = 0;
    }
    else if (m == should_m) {
        ret = 1;
    }
    return ret;
}
 
void emulator(const EmulatorParams* params)
{
    int idx = 2;
    int bitlen = 32;
    TimeCounter tc;
    if (params != nullptr) {
        n = params->n * bitlen;
    }
    LOG_INFO("n = " + to_string(n));

    tc.Start();
    Setup();
    tc.EndAndPrintSec("Setup");

    tc.Start();
    EncodeDB();
    tc.EndAndPrintSec("EncodeDB");

    tc.Start();
    vector<mpz_class> q;
    for (int i(0); i < bitlen; ++i)
        q = EncodeIndex(idx);
    tc.EndAndPrintSec("EncodeIndex");

    tc.Start();
    mpz_class alpha;
    for (int i(0); i < bitlen; ++i)
        alpha = Query(g_q);
    tc.EndAndPrintMs("Query");

    // tc.Start();
    // bool v = Verify(sigma_i, sigma_vi, pi);
    // tc.EndAndPrintMs("Verify");
    // if (v) {
    //     LOG_INFO("verification passed");
    // } else {
    //     LOG_FATAL("verification failed");
    // }

    tc.Start();
    long result;
    for (int i(0); i < bitlen; ++i)
        result = DecodeResult(idx, alpha);
    tc.EndAndPrintMs("Decrypt");

    if (result == (idx & 1)) {
        LOG_INFO("decrypt result is correct");
    } else {
        LOG_FATAL(string("decrypt result is wrong. src is ") + to_string(idx&1) + ". result is " + to_string(result));
    }
}

// void testcode()
// {
//     Setup();
//     ZZ k;
//     k = random();
//     cout << "oprf" << endl;
//     for (int i(0); i < 3; ++i) {
//         cout << oprf("pdoubleang", k) << endl;
//     }
// }