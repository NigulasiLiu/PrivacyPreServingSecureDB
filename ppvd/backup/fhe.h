#ifndef _FHE
#define _FHE

#include <gmp.h>
#include <string>

namespace FHE
{
class SHE{
private:
    mp_bitcnt_t k0;
    mp_bitcnt_t k1;
    mp_bitcnt_t k2;
    mpz_t _N;
    mpz_t _p;
    mpz_t _L;
    gmp_randstate_t state;
private:
  void random_nbit(mpz_t num, mp_bitcnt_t n);
  void random_prime(mpz_t num, mp_bitcnt_t n);
  void init();
public:
  SHE(mp_bitcnt_t k0, mp_bitcnt_t k1, mp_bitcnt_t k2);
  SHE(mp_bitcnt_t k0, mp_bitcnt_t k1, mp_bitcnt_t k2, const std::string& N, const std::string& L, const std::string& p);
  ~SHE();
  void get_params(mp_bitcnt_t& k0, mp_bitcnt_t& k1, mp_bitcnt_t& k2, std::string& N, std::string& L, std::string& p);
  void add(mpz_t result, const mpz_t a, const mpz_t b);
  void add_ul(mpz_t result, const mpz_t a, unsigned long b);
  void mul(mpz_t result, const mpz_t a, const mpz_t b);
  void mul_sl(mpz_t result, const mpz_t a, long b);
  void encrypt(mpz_t cipher, long plain);
  void encrypt(mpz_t cipher, const mpz_t plain);
  long decrypt(const mpz_t cipher);
  void decrypt(mpz_t plain, const mpz_t cipher);
  void pub_encrypt(mpz_t cipher, const mpz_t plain, const mpz_t zero1, const mpz_t zero2);
  void pub_encrypt(mpz_t cipher, long plain, const mpz_t zero1, const mpz_t zero2);
  void sub(mpz_t result, const mpz_t a, const mpz_t b);
  void sub_ul(mpz_t result, const mpz_t a, unsigned long b);
};
}

#endif