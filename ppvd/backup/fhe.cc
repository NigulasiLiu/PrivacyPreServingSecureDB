#include "fhe.h"
#include <cstddef>
#include <cstdlib>
#include <gmp.h>
#include <ctime>
#include <string>

namespace FHE {



SHE::SHE(mp_bitcnt_t k0, mp_bitcnt_t k1, mp_bitcnt_t k2)
    : k0(k0), k1(k1), k2(k2) 
{
    gmp_randinit_default(state);
    gmp_randseed_ui(state, clock());
    init();
}

SHE::SHE(mp_bitcnt_t k0, mp_bitcnt_t k1, mp_bitcnt_t k2, const std::string& N, const std::string& L, const std::string& p)
    : k0(k0), k1(k1), k2(k2)
{
    gmp_randinit_default(state);
    gmp_randseed_ui(state, clock());
    const int base = 10;
    mpz_init_set_str(_N, N.c_str(), base);
    mpz_init_set_str(_L, L.c_str(), base);
    mpz_init_set_str(_p, p.c_str(), base);
}

SHE::~SHE() {
    mpz_clear(_p);
    mpz_clear(_L);
    mpz_clear(_N);
}

void SHE::get_params(mp_bitcnt_t &k0, mp_bitcnt_t &k1, mp_bitcnt_t &k2, std::string &N, std::string &L, std::string &p) {
    k0 = this->k0;
    k1 = this->k1;
    k2 = this->k2;
    const int base = 10;
    char *buf;
    void (*freefunc)(void*, size_t);
    mp_get_memory_functions(NULL, NULL, &freefunc);
    buf = mpz_get_str(NULL, base, _N);
    N = buf;
    freefunc(buf, N.size()+1);

    buf = mpz_get_str(NULL, base, _L);
    L = buf;
    freefunc(buf, L.size()+1);

    buf = mpz_get_str(NULL, base, _p);
    p = buf;
    freefunc(buf, p.size()+1);
}

void SHE::init(){
    mpz_init(_p);
    mpz_init(_L);
    mpz_init(_N);
    mpz_t q;
    mpz_init(q);

    random_prime(_p, k0);
    random_prime(q, k0);
    mpz_mul(_N, _p, q);
    random_nbit(_L, k2);

    mpz_clear(q);
}

void SHE::random_nbit(mpz_t num, mp_bitcnt_t n) {
    mpz_urandomb(num, state, n);
}

void SHE::random_prime(mpz_t num, mp_bitcnt_t n) {
    random_nbit(num, n);
    mpz_nextprime(num, num);
}

void SHE::add(mpz_t result, const mpz_t a, const mpz_t b) {
    mpz_add(result, a, b);
    mpz_mod(result, result, _N);
}

void SHE::add_ul(mpz_t result, const mpz_t a, unsigned long b) {
    mpz_add_ui(result, a, b);
    mpz_mod(result, a, _N);
}

void SHE::mul(mpz_t result, const mpz_t a, const mpz_t b) {
    mpz_mul(result, a, b);
    mpz_mod(result, result, _N);
}

void SHE::mul_sl(mpz_t result, const mpz_t a, long b) {
    mpz_mul_si(result, a, b);
    mpz_mod(result, result, _N);
}

void SHE::encrypt(mpz_t cipher, long plain) {
    mpz_t plain_z;
    mpz_init_set_si(plain_z, plain);

    encrypt(cipher, plain_z);

    mpz_clear(plain_z);
}

void SHE::encrypt(mpz_t cipher, const mpz_t plain) {
    mpz_t r, rr;
    mpz_init(r);
    mpz_init(rr);
    random_nbit(r, k2);
    random_nbit(rr, k0);
    
    mpz_mul(r, r, _L);
    mpz_add(r, r, plain);
    mpz_mul(rr, rr, _p);
    mpz_add_ui(rr, rr, 1);
    mpz_mul(cipher, r, rr);
    mpz_mod(cipher, cipher, _N);

    mpz_clear(r);
    mpz_clear(rr);
}

long SHE::decrypt(const mpz_t cipher) {
    mpz_t plain_z;
    mpz_init(plain_z);
    decrypt(plain_z, cipher);
    long plain = mpz_get_si(plain_z);
    mpz_clear(plain_z);
    return plain;
}

void SHE::decrypt(mpz_t plain, const mpz_t cipher) {
    mpz_mod(plain, cipher, _p);
    mpz_mod(plain, plain, _L);
    mpz_t half_L;
    mpz_init(half_L);
    mpz_div_ui(half_L, _L, 2);
    if (mpz_cmp(plain, half_L) > 0) {
        mpz_sub(plain, plain, _L);
    }
    mpz_clear(half_L);
}

void SHE::pub_encrypt(mpz_t cipher, const mpz_t plain, const mpz_t zero1, const mpz_t zero2) {
    const mp_bitcnt_t r_bit = 64;
    mpz_t r1, r2;
    mpz_init(r1);
    mpz_init(r2);
    random_nbit(r1, r_bit);
    random_nbit(r2, r_bit);

    mpz_mul(r1, r1, zero1);
    mpz_mul(r2, r2, zero2);
    mpz_add(cipher, plain, r1);
    mpz_add(cipher, cipher, r2);

    mpz_clear(r1);
    mpz_clear(r2);
}

void SHE::pub_encrypt(mpz_t cipher, long plain, const mpz_t zero1, const mpz_t zero2) {
    mpz_t plain_z;
    mpz_init(plain_z);
    pub_encrypt(cipher, plain_z, zero1, zero2);
    mpz_clear(plain_z);
}

void SHE::sub(mpz_t result, const mpz_t a, const mpz_t b) {
    // mpz_sub(result, a, b);
    static mpz_t neg_one;
    static bool is_init(false);
    if (!is_init) {
        is_init = true;
        mpz_init(neg_one);
        encrypt(neg_one, -1l);
    }
    mpz_mul(result, neg_one, b);
    mpz_add(result, result, a);
    mpz_mod(result, result, _N);
}

void SHE::sub_ul(mpz_t result, const mpz_t a, unsigned long b) {
    mpz_t b_z;
    mpz_init_set_ui(b_z, b);
    sub(result, a, b_z);
    mpz_clear(b_z);
}
} // namespace FHE