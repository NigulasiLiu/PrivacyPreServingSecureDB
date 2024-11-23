#include "fhe.h"
#include "nlohmann/json_fwd.hpp"
#include <ctime>
#include <fstream>
#include <gmp.h>
#include <gmpxx.h>
#include <nlohmann/json.hpp>
#include <string>

using namespace nlohmann;
using namespace std;
namespace FHE {

mpz_class SHE::random_nbit(mp_bitcnt_t n)
{
    return rnd_gen.get_z_bits(n);
}

mpz_class SHE::random_prime(mp_bitcnt_t n)
{
    mpz_t tmp;
    mpz_init(tmp);
    mpz_nextprime(tmp, random_nbit(n).get_mpz_t());
    mpz_class ret(tmp);
    mpz_clear(tmp);
    return ret;
}

void SHE::init()
{
    mpz_class q;
    _p = random_prime(k0);
    q = random_prime(k0);
    _N = _p * q;
    _L = random_prime(k2);
}

SHE::SHE(mp_bitcnt_t k0, mp_bitcnt_t k1, mp_bitcnt_t k2)
    :k0(k0), k1(k1), k2(k2), rnd_gen(gmp_randinit_default)
{
    rnd_gen.seed(time(nullptr));
    init();
}

SHE::SHE()
    :rnd_gen(gmp_randinit_default)
{
    rnd_gen.seed(time(nullptr));
}

void SHE::save_params_to_file(const std::string& filename)
{
    json content;
    content["k0"] = k0;
    content["k1"] = k1;
    content["k2"] = k2;
    content["L"] = _L.get_str();
    content["N"] = _N.get_str();
    content["p"] = _p.get_str();
    ofstream out_file(filename);
    out_file << content << endl;
}

void SHE::set_params_from_file(const std::string& filename)
{
    ifstream in_file(filename);
    json content;
    in_file >> content;
    k0 = content["k0"];
    k1 = content["k1"];
    k2 = content["k2"];
    
    _p.set_str(content["p"], 10);
    _L.set_str(content["L"], 10);
    _N.set_str(content["N"], 10);
}

mpz_class SHE::add(const mpz_class& a, const mpz_class& b)
{
    return (a + b) % _N;
}

mpz_class SHE::sub(const mpz_class& a, const mpz_class& b)
{
    return (a - b) % _N;
}

mpz_class SHE::mul(const mpz_class& a, const mpz_class& b)
{
    return (a * b) % _N;
}

mpz_class SHE::encrypt(const mpz_class& plain)
{
    auto r = random_nbit(k2);
    auto rr = random_nbit(k0);
    return ((r * _L + plain) * (1 + rr * _p)) % _N;
}

mpz_class SHE::decrypt(const mpz_class& cipher)
{
    return (cipher % _p) % _L;
}

mpz_class SHE::pub_encrypt(const mpz_class& plain, const mpz_class& zero1, const mpz_class& zero2)
{
    auto r1 = random_nbit(8);
    auto r2 = random_nbit(8);
    return (plain + r1 * zero1 + r2 * zero2) % _N;
}



} // namespace FHE ends