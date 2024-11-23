#ifndef _FHE
#define _FHE

#include <gmp.h>
#include <gmpxx.h>
#include <string>

namespace FHE
{
class SHE{
private:
    mp_bitcnt_t k0;
    mp_bitcnt_t k1;
    mp_bitcnt_t k2;
    mpz_class _N;
    mpz_class _p;
    mpz_class _L;
    gmp_randclass rnd_gen;
private:
  void init();
public:
  mpz_class random_nbit(mp_bitcnt_t n);
  mpz_class random_prime(mp_bitcnt_t n);
  SHE(mp_bitcnt_t k0, mp_bitcnt_t k1, mp_bitcnt_t k2);
  SHE();
  void save_params_to_file(const std::string& filename);
  void set_params_from_file(const std::string& filename);
  mpz_class add(const mpz_class& a, const mpz_class& b);
  mpz_class sub(const mpz_class& a, const mpz_class& b);
  mpz_class mul(const mpz_class& a, const mpz_class& b);
  mpz_class encrypt(const mpz_class& plain);
  mpz_class decrypt(const mpz_class& cipher);
  mpz_class pub_encrypt(const mpz_class& plain, const mpz_class& zero1, const mpz_class& zero2);
};

class PHE{
private:
    mpz_class _N;
    mpz_class _N2;
    mpz_class _lambda;
    mpz_class _g;
    mpz_class _mu;
    mp_bitcnt_t n_bitlen;
    gmp_randclass rnd_gen;
private:
  void init();
  mpz_class L(const mpz_class& x);
public:
  mpz_class random_nbit(mp_bitcnt_t n);
  mpz_class random_prime(mp_bitcnt_t n);
  PHE();
  PHE(mp_bitcnt_t n_bitlen);
  void save_params_to_file(const std::string& filename);
  void set_params_from_file(const std::string& filename);
  mpz_class add(const mpz_class& a, const mpz_class& b);
  mpz_class mul(const mpz_class& a, const mpz_class& b);
  mpz_class encrypt(const mpz_class& plain);
  mpz_class decrypt(const mpz_class& cipher);
};

}

#endif