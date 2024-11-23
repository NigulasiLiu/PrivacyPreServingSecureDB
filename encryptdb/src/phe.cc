#include "fhe.h"
#include "nlohmann/json_fwd.hpp"
#include <ctime>
#include <fstream>
#include <gmp.h>
#include <gmpxx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

using namespace nlohmann;
using namespace std;
namespace FHE {

// mpz_class MpzClassPowerM(const mpz_class& a, const mpz_class& x, const mpz_class& m)
// {
//     mpz_t num;
//     mpz_init(num);
//     mpz_powm(num, a.get_mpz_t(), x.get_mpz_t(), m.get_mpz_t());
//     mpz_class ret(num);
//     mpz_clear(num);
//     return ret;
// }

mpz_class PHE::random_nbit(mp_bitcnt_t n)
{
    return rnd_gen.get_z_bits(n);
}

mpz_class PHE::random_prime(mp_bitcnt_t n)
{
    mpz_t tmp;
    mpz_init(tmp);
    auto r = random_nbit(n);
    r |= mpz_class(1) << (n-1);
    mpz_nextprime(tmp, r.get_mpz_t());
    mpz_class ret(tmp);
    mpz_clear(tmp);
    return ret;
}

void PHE::init()
{
    auto half_bitlen = n_bitlen / 2;
    auto p = random_prime(half_bitlen);
    auto q = random_prime(half_bitlen);
    cout << "|p| = " << mpz_sizeinbase(p.get_mpz_t(), 2) << endl;
    cout << "|q| = " << mpz_sizeinbase(q.get_mpz_t(), 2) << endl;
    if(!mpz_probab_prime_p(p.get_mpz_t(), 30)
       || !mpz_probab_prime_p(q.get_mpz_t(), 30))
    {
        cout << "p or q is not prime" << endl;
    }
    if (mpz_sizeinbase(p.get_mpz_t(), 2) != mpz_sizeinbase(q.get_mpz_t(), 2)) {
        cout << "|p| != |q|" << endl;
    }
    if ((mpz_class)gcd(p*q, (p-1)*(q-1)) != 1) {
        cout << "gcd(pq, (p-1)(q-1)) != 1" << endl;
    }
    _N = p * q;
    _N2 = _N * _N;
    _lambda = lcm(p-1, q-1);
    if (_lambda % (p-1) != 0 || _lambda % (q-1) != 0) {
        cout << "_lambda is wrong" << endl;
    }
    _g = _N + 1;
    mpz_class glambda;
    mpz_powm(glambda.get_mpz_t(), _g.get_mpz_t(), _lambda.get_mpz_t(), _N2.get_mpz_t());
    mpz_class lx = L(glambda);
    // cout << "mu^-1 = " << lx << endl;
    mpz_invert(_mu.get_mpz_t(), lx.get_mpz_t(), _N.get_mpz_t());
}

mpz_class PHE::L(const mpz_class& x)
{
    return (x - 1) / _N;
}

PHE::PHE()
    :rnd_gen(gmp_randinit_default)
{
    rnd_gen.seed(time(nullptr));
}

PHE::PHE(mp_bitcnt_t n_bitlen)
    :n_bitlen(n_bitlen), rnd_gen(gmp_randinit_default)
{
    rnd_gen.seed(time(nullptr));
    init();
}

void PHE::save_params_to_file(const std::string& filename)
{
    json content;
    content["N"] = _N.get_str();
    content["lambda"] = _lambda.get_str();
    content["mu"] = _mu.get_str();
    content["n_bitlen"] = n_bitlen;
    ofstream out_file(filename);
    out_file << content << endl;
}

void PHE::set_params_from_file(const std::string& filename)
{
    ifstream in_file(filename);
    json content;
    in_file >> content;
    
    _N.set_str(content["N"], 10);
    _N2 = _N * _N;
    _lambda.set_str(content["lambda"], 10);
    _g = _N + 1;
    _mu.set_str(content["mu"], 10);
    n_bitlen = content["n_bitlen"];
}

mpz_class PHE::add(const mpz_class& a, const mpz_class& b)
{
    return (a * b) % _N2;
}

mpz_class PHE::mul(const mpz_class& a, const mpz_class& b)
{
    mpz_class ret;
    mpz_powm(ret.get_mpz_t(),
        a.get_mpz_t(),
        b.get_mpz_t(),
        _N2.get_mpz_t());
    return ret;
}

mpz_class PHE::encrypt(const mpz_class& plain)
{
    mpz_class r = random_nbit(n_bitlen) % _N;
    mpz_class gm, rn;
    mpz_powm(gm.get_mpz_t(),
        _g.get_mpz_t(),
        plain.get_mpz_t(),
        _N2.get_mpz_t());
    mpz_powm(rn.get_mpz_t(),
        r.get_mpz_t(),
        _N.get_mpz_t(),
        _N2.get_mpz_t());
    return (gm * rn) % _N2;
}

mpz_class PHE::decrypt(const mpz_class& cipher)
{
    mpz_class clambda;
    mpz_powm(clambda.get_mpz_t(),
        cipher.get_mpz_t(),
        _lambda.get_mpz_t(),
        _N2.get_mpz_t());
    auto Lclambda = L(clambda);
    // cout << endl;
    // cout << "L(clambda) = " << Lclambda << endl;
    auto plain = (Lclambda * _mu) % _N;
    return plain;
}

} // namespace FHE ends