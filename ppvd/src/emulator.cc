#include "emulator.h"
#include "merkel.h"
#include "tools.h"
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <NTL/vec_ZZ_p.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>
#include <pbc/pbc_field.h>
#include <pbc/pbc_pairing.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace NTL;
using namespace std;

PublicKey pk;
PrivateKey sk;
string filename = "../dataset/n3-byte1.json";
Dataset dataset;
ZZ_pX poly;
G1 delta;
mpz_class q;

gmp_randclass gmp_r(gmp_randinit_default);

unsigned int oprf(const std::string& x, NTL::ZZ k)
{
    auto r = random_ZZ_p();
    ZZ_p hx;
    stringstream ss;
    ss << sha256_wrapper(x);
    ss >> hx;
    ZZ_p a = power(hx, rep(r));
    ZZ_p b = power(a, k);
    ss.clear();
    ZZ_p second = power(b, rep(inv(r)));
    ss << x << second;
    string out = sha256_wrapper(ss.str()).substr((256/4)-7);
    return stol(out, nullptr, 16);
}

void Setup(long security_param = 1024)
{
    ZZ seed;
    seed = time(nullptr);
    SetSeed(seed);
    gmp_r.seed(time(nullptr));
    FILE *sys_param_file = fopen("../params/pairing.param", "r");
    pk.e.init(Pairing(sys_param_file).get_pbc_param_t());
    pk.g = G1(pk.e, false);
    pk.gt = pk.e(pk.g, pk.g);
    pk.p = GenPrime_ZZ(security_param);
    stringstream ss;
    ss << "730750862221594424981965739670091261094297337857";
    ss >> pk.p;
    LOG_INFO("p = "+ZZtoMpzClass(pk.p).get_str());
    ZZ_p::init(pk.p);

    // sk.alpha = random_ZZ_p();
    // sk.k = random_ZZ_p();
    sk.alpha = random_ZZ_p();
    sk.k = RandomBits_ZZ(security_param);

    dataset.RetrieveDataset(filename);
    long n = dataset.getN();
    pk.omega.reserve(n);
    Zr alpha = ZZptoZr(pk.e, sk.alpha);
    for (long i(0); i < n; ++i) {
        Zr zr_i = Zr(pk.e, i);
        pk.omega.push_back(pk.g ^ (alpha ^ zr_i));
    }
}

void EncodeDB()
{
    long n = dataset.getN();
    q = 0;
    for (int i(0); i < n; ++i) {
        auto val = dataset.db_get(i);
        auto mask = oprf(to_string(i), sk.k);
        // LOG_INFO("[" + to_string(i) + "] mask = " + to_string(mask));
        dataset.edb_set(i, val ^ mask);
        // LOG_INFO("[" + to_string(i) +"] = " + to_string(dataset.edb_get(i)));
        q = max(q.get_si(), dataset.edb_get(i));
    }
    q += gmp_r.get_z_bits(32);
    LOG_INFO("q = " + q.get_str());

    // generate polynomial
    vec_ZZ_p x, y;
    x.SetLength(n);
    y.SetLength(n);
    for (long i(0); i < n; ++i) {
        x[i] = i;
        y[i] = dataset.edb_get(i);
    }
    interpolate(poly, x, y);

    auto falpha = ZZptoMpzClass(eval(poly, sk.alpha));
    delta = pk.g ^ MpzClasstoZr(pk.e, falpha);
}

// return [r, sigma_i]
pair<mpz_class, mpz_class> EncodeIndex(int idx)
{
    mpz_class r;
    r = q + gmp_r.get_z_bits(32);
    LOG_INFO("r = " + r.get_str());
    LOG_INFO("sigma_i = " + mpz_class(r+idx).get_str());
    return make_pair(r, r+idx);
}

// return [sigma_vi, pi]
pair<mpz_class, G1> Query(mpz_class sigma_i)
{
    ZZ_p zzp_sigma_i = MpzClasstoZZp(sigma_i);
    LOG_INFO("sigma_i = " + sigma_i.get_str());
    mpz_class sigma_vi = ZZptoMpzClass(eval(poly, zzp_sigma_i));
    stringstream ss;
    ss << "[" << -sigma_i << " 1]";
    ZZ_pX Qx, fac;
    ss >> fac;
    div(Qx, poly-eval(poly, zzp_sigma_i), fac);

    // ================= test ============================
    // cout << "f(x) = " << poly << endl;
    // cout << "Q(x) = " << Qx << endl;
    // cout << "fac = " << fac << endl;
    // ZZ_pX t_mul_fx;
    // mul(t_mul_fx, Qx, fac);
    // t_mul_fx += eval(poly, zzp_sigma_i);
    // cout << "t_mul_fx = " << t_mul_fx << endl;
    // auto t_falpha = eval(poly, sk.alpha);
    // auto t_qalpha = eval(Qx, sk.alpha);
    // auto t_alpha_sub_sigma = sk.alpha - zzp_sigma_i;
    // auto t_fsigma = eval(poly, zzp_sigma_i);
    // auto t_left = t_falpha;
    // auto t_right = t_alpha_sub_sigma * t_qalpha + t_fsigma;
    // cout << "t_left = " << t_left << endl;
    // cout << "t_right = " << t_right << endl;
    // cout << "cmp = " << (t_left == t_right) << endl;
    // cout << "ZZ_p values:" << endl;
    // cout << "f(alpha) = " << t_falpha << endl;
    // cout << "alpha - sigma" << t_alpha_sub_sigma << endl;
    // cout << "Q(alpha) = " << t_qalpha << endl;
    // cout << "f(sigma) = " << t_fsigma << endl;
    // cout << "Zr values:" << endl;
    // auto t_zr_falpha = ZZptoZr(pk.e, t_falpha);
    // auto t_zr_qalpha = ZZptoZr(pk.e, t_qalpha);
    // auto t_zr_alpha_sub_sigma = ZZptoZr(pk.e, t_alpha_sub_sigma);
    // auto t_zr_fsigma = ZZptoZr(pk.e, t_fsigma);
    
    // auto t_g_falpha = pk.g ^ t_zr_falpha;
    // auto t_g_asubs = pk.g ^ t_zr_alpha_sub_sigma;
    // auto t_g_qalpha = pk.g ^ t_zr_qalpha;
    // auto t_g_fsigma = pk.g ^ t_zr_fsigma;
    // cout << "verify = " << (pk.e(t_g_falpha, pk.g) == pk.e(t_g_asubs, t_g_qalpha)*pk.e(t_g_fsigma, pk.g)) << endl;
    // cout << "delta is " << (t_g_falpha == delta ? "correct" : "wrong") << endl;
    // t_g_asubs.dump(stdout, "asubs");
    // t_g_qalpha.dump(stdout, "Q(alpha)");
    // t_g_fsigma.dump(stdout, "f(sigma)");
    // ================= test end ========================


    auto qx_deg = deg(Qx);
    LOG_INFO("Q(x)'s deg is " + to_string(qx_deg));
    // cout << "Q(x) = " << Qx << endl;
    G1 self_pi;
    for (long i(0); i <= qx_deg; ++i) {
        auto co = coeff(Qx, i);
        // cout << i << "'s coeff = " << co << endl;
        auto item = pk.omega[i] ^ ZZptoZr(pk.e, co);
        if (i == 0) {
            self_pi = item;
        } else {
            self_pi *= item;
        }
    }

    // auto qalpha = ZZptoMpzClass(eval(Qx, sk.alpha));
    // G1 pi = pk.g ^ MpzClasstoZr(pk.e, qalpha);
    // if (self_pi == pi) {
    //     LOG_INFO("self_pi is correct");
    // } else {
    //     LOG_FATAL("self_pi is wrong");
    // }
    return make_pair(sigma_vi, self_pi);
}

bool Verify(mpz_class sigma_i, mpz_class sigma_vi, G1 pi)
{
    bool res;
    Zr alpha = MpzClasstoZr(pk.e, ZZptoMpzClass(sk.alpha));
    // alpha.dump(stdout, "alpha ", 10);
    Zr zr_sigma_i = MpzClasstoZr(pk.e, sigma_i);
    // zr_sigma_i.dump(stdout, "zr_sigma_i ", 10);
    Zr zr_sigma_vi = MpzClasstoZr(pk.e, sigma_vi);
    // zr_sigma_vi.dump(stdout, "zr_sigma_vi ", 10);

    // ================== test =====================
    // cout << "================== Verify ===================" << endl;
    // auto t_alpha_sub_sigma = sk.alpha - MpzClasstoZZp(sigma_i);
    // cout << "correct alpha - sigma = " << t_alpha_sub_sigma << endl;
    // Zr t_asubs_zr(pk.e, ZZptoMpzClass(t_alpha_sub_sigma).get_si());
    // t_asubs_zr.dump(stdout, "alpha - sigma in Zr ", 10);
    // element_t asubs_zr_t;
    // element_init_Zr(asubs_zr_t, const_cast<pairing_s*>(pk.e.getPairing()));
    // element_set_mpz(asubs_zr_t, ((mpz_class)(ZZptoMpzClass(sk.alpha)-sigma_i)).get_mpz_t());
    // t_asubs_zr.setElement(asubs_zr_t);
    // t_asubs_zr.dump(stdout, "alpha - sigma in Zr ", 10);
    // auto ga = pk.g ^ t_asubs_zr;
    // auto gb = (pk.g ^ alpha)/(pk.g ^ zr_sigma_i);
    // ga.dump(stdout, "ga ");
    // gb.dump(stdout, "gb ");
    
    // cout << endl;
    // pi.dump(stdout, "Q(alpha) in Verify");
    // gb.dump(stdout, "asubs in Verify");
    // (pk.g ^ zr_sigma_vi).dump(stdout, "f(sigma) in Verify");

    // cout << "=============================================" << endl;
    // =============================================
    
    res = pk.e(delta, pk.g) == pk.e(pi, (pk.g ^ alpha)/(pk.g ^ zr_sigma_i)) * pk.e(pk.g^zr_sigma_vi, pk.g);
    return res;
}

long DecodeResult(mpz_class sigma_vi, int idx, mpz_class r)
{
    LOG_INFO("sigma_vi = " + sigma_vi.get_str());
    mpz_class cipher = sigma_vi % r;
    LOG_INFO("sigma_vi % r = " + cipher.get_str());
    auto mask = oprf(to_string(idx), sk.k);
    LOG_INFO("mask = " + to_string(mask));
    auto a = cipher.get_si();
    return a ^ mask;
}
 
void emulator(const EmulatorParams* params)
{
    int idx = 2;
    TimeCounter tc;
    if (params != nullptr) {
        filename = params->filename;
    }
    int n = get_n_from_filename(filename);
    LOG_INFO("n = " + to_string(n));

    tc.Start();
    Setup();
    tc.EndAndPrintSec("Setup");

    tc.Start();
    EncodeDB();
    tc.EndAndPrintSec("EncodeDB");

    tc.Start();
    auto [r, sigma_i] = EncodeIndex(idx);
    tc.EndAndPrintSec("EncodeIndex");

    tc.Start();
    auto [sigma_vi, pi] = Query(sigma_i);
    tc.EndAndPrintMs("Query");

    tc.Start();
    bool v = Verify(sigma_i, sigma_vi, pi);
    tc.EndAndPrintMs("Verify");
    if (v) {
        LOG_INFO("verification passed");
    } else {
        LOG_FATAL("verification failed");
    }

    tc.Start();
    auto result = DecodeResult(sigma_vi, idx, r);
    tc.EndAndPrintMs("Decrypt");

    if (result == dataset.db_get(idx)) {
        LOG_INFO("decrypt result is correct");
    } else {
        LOG_FATAL(string("decrypt result is wrong. src is ") + to_string(dataset.db_get(idx)) + ". result is " + to_string(result));
    }
}

void testcode()
{
    Setup();
    ZZ k;
    k = random();
    cout << "oprf" << endl;
    for (int i(0); i < 3; ++i) {
        cout << oprf("pdoubleang", k) << endl;
    }
}