#include <cstdint>
#include <ctime>
#include <gmp.h>

namespace FHE {

class PHE {
public:
    mp_bitcnt_t k_;
    mpz_t N_;
    mpz_t p_;
    mpz_t g_;
    mpz_t lam_;
    mpz_t mu_;
    mpz_t N2_;
    gmp_randstate_t state_;

public:
    int K() const {
        return k_;
    }
    void random_nbit(mpz_t num, mp_bitcnt_t n) {
        mpz_urandomb(num, state_, n);
    }
    void random_prime(mpz_t num, mp_bitcnt_t n) {
        random_nbit(num, n);
        mpz_nextprime(num, num);
    }
    void init() {
        mpz_init(N_);
        mpz_init(p_);
        mpz_init(g_);
        mpz_init(lam_);
        mpz_init(mu_);
        mpz_init(N2_);
        mpz_t q;
        mpz_init(q);
        random_prime(p_, k_);
        random_prime(q, k_);
        mpz_mul(N_, p_, q);
        mpz_mul(N2_, N_, N_);
        // g_ = N_ + 1
        mpz_add_ui(g_, N_, 1);
        // lam_ = lcm(p-1, q-1)
        mpz_t tmp1, tmp2;
        mpz_init(tmp1);
        mpz_init(tmp2);
        mpz_sub_ui(tmp1, q, 1);
        mpz_sub_ui(tmp2, p_, 1);
        mpz_lcm(lam_, tmp1, tmp2);
        // mu_
        mpz_powm(tmp1, g_, lam_, N2_);
        mpz_sub_ui(tmp1, tmp1, 1);
        mpz_div(tmp1, tmp1, N_);
        mpz_invert(mu_, tmp1, N_);
        // gmp_printf("lam_ = %Zd\nmu_ = %Zd\ngcd(mu_, N_) = %Zd\ng_ = %Zd\n"
            // , lam_, mu_, tmp2, g_);
        mpz_clear(tmp1);
        mpz_clear(tmp2);        
        mpz_clear(q);
    }

    PHE(mp_bitcnt_t k) : k_(k) {
        gmp_randinit_default(state_);
        gmp_randseed_ui(state_, clock());
        init();
    }

    ~PHE() {
        mpz_clear(N_);
        mpz_clear(p_);
        mpz_clear(g_);
        mpz_clear(lam_);
        mpz_clear(mu_);
        mpz_clear(N2_);
    }
    
    void encrypt(mpz_t cipher, unsigned long plain) {
        mpz_t r;
        mpz_init(r);
        random_nbit(r, k_);
        mpz_powm_ui(cipher, g_, plain, N2_);
        mpz_powm(r, r, N_, N2_);
        mpz_mul(cipher, cipher, r);
        mpz_mod(cipher, cipher, N2_);
        mpz_clear(r);
    }
    
    void encrypt(mpz_t cipher, mpz_t a) {
        mpz_t r;
        mpz_init(r);
        random_nbit(r, k_);
        mpz_powm(cipher, g_, a, N2_);
        mpz_powm(r, r, N_, N2_);
        mpz_mul(cipher, cipher, r);
        mpz_mod(cipher, cipher, N2_);
        mpz_clear(r);
    }
    
    void encode_encrypt(mpz_t cipher, long plain) {
        mpz_t tmp;
        mpz_init_set_si(tmp, plain);
        encode_encrypt(cipher, tmp);
        mpz_clear(tmp);
    }
    void encode_encrypt(mpz_t cipher, mpz_t a) {
        mpz_t tmp;
        mpz_init(tmp);
        encode(tmp, a);
        encrypt(cipher, tmp);
        mpz_clear(tmp);
    }
    
    void decrypt(mpz_t plain, mpz_t cipher) {
        mpz_t tmp;
        mpz_init(tmp);
        mpz_powm(tmp, cipher, lam_, N2_);
        mpz_sub_ui(tmp, tmp, 1);
        mpz_div(tmp, tmp, N_);
        mpz_mul(tmp, tmp, mu_);
        mpz_mod(plain, tmp, N_);
        mpz_clear(tmp);
    }
    
    void decode_decrypt(mpz_t plain, mpz_t cipher) {
        decrypt(plain, cipher);
        decode(plain, plain);
    }
    
    void add(mpz_t result, mpz_t a, mpz_t b) {
        mpz_mul(result, a, b);
        mpz_mod(result, result, N2_);
    }
    
    void mul(mpz_t result, mpz_t a, unsigned long b) {
        mpz_powm_ui(result, a, b, N2_);
    }
    
    void mul(mpz_t result, mpz_t a, mpz_t b) {
        mpz_powm(result, a, b, N2_);
    }

    void div(mpz_t result, mpz_t a, mpz_t b) {
        mpz_t d;
        mpz_init(d);
        mpz_invert(d, b, N_);
        // mpz_sub(d, N_, b);
        mul(result, a, d);
        mpz_clear(d);
    }
    
    void neg(mpz_t result, mpz_t a) {
        mpz_t tmp;
        mpz_init_set(tmp, N_);
        mpz_sub_ui(tmp, tmp, 1);
        mpz_powm(result, a, tmp, N2_);
        mpz_clear(tmp);
    }
    
    void sub(mpz_t result, mpz_t a, mpz_t b) {
        mpz_t tmp;
        mpz_init(tmp);
        neg(tmp, b);
        add(result, a, tmp);
        mpz_clear(tmp);
    }
    
    void decode(mpz_t result, mpz_t a) {
        mpz_t half;
        mpz_init_set(half, N_);
        mpz_div_ui(half, half, 2);
        if (mpz_cmp(a, half) <= 0) {
            mpz_set(result, a);
        } else {
            mpz_sub(result, a, N_);
        }
        mpz_clear(half);
    }
    
    void encode(mpz_t result, mpz_t a) {
        if (mpz_cmp_si(a, 0) < 0) {
            mpz_add(result, a, N_);
        } else {
            mpz_set(result, a);
        }
    }

};

} // namespace FHE end