#include "phe.h"
#include <algorithm>
#include <bits/types/clock_t.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <gmp.h>
#include <iostream>
#include <vector>
#include "utility.h"

using utility::MPZ_T;
using utility::MPZ_VECTOR;


FHE::PHE *phe;
MPZ_T zero, one;

void SLEQ(mpz_t result, mpz_t a, mpz_t b) {
    mpz_t r1, r2, tmp;
    mpz_init(r1);
    mpz_init(r2);
    int s = rand()&1;
    phe->random_nbit(r1, phe->K());
    phe->random_nbit(r2, phe->K());
    mpz_add(r1, r1, r2);
    MPZ_T er2;
    phe->mul(er2.ptr, one.ptr, r2);
    // r1(a-b)-r2
    MPZ_T theta;
    phe->sub(theta.ptr, a, b);
    phe->mul(theta.ptr, theta.ptr, r1);
    phe->sub(theta.ptr, theta.ptr, er2.ptr);
    if (!s) {
        phe->neg(theta.ptr, theta.ptr);
    }
    phe->decode_decrypt(theta.ptr, theta.ptr);

    MPZ_T mu;
    if (mpz_cmp_si(theta.ptr, 0) < 0) {
        // phe->encrypt(mu.ptr, 1ul);
        mpz_set(mu.ptr, one.ptr);
    } else {
        // phe->encrypt(mu.ptr, 0ul);
        mpz_set(mu.ptr, zero.ptr);
    }

    if (s) {
        mpz_set(result, mu.ptr);
    } else {
        phe->sub(result, one.ptr, mu.ptr);
    }
    mpz_clear(r1);
    mpz_clear(r2);
}

void SM(mpz_t result, mpz_t a, mpz_t b) {
    mpz_t r1, r2, aa, bb, er1, er2, ha, hb;
    mpz_init(r1);
    mpz_init(r2);
    mpz_init(aa);
    mpz_init(bb);
    mpz_init(er1);
    mpz_init(ha);
    mpz_init(hb);
    mpz_init(er2);

    phe->random_nbit(r1, phe->K());
    phe->random_nbit(r2, phe->K());
    // phe->encrypt(er2, r2);
    // phe->encrypt(er1, r1);
    phe->mul(er1, one.ptr, r1);
    phe->mul(er2, one.ptr, r2);


    phe->add(aa, a, er1);
    phe->add(bb, b, er2);
    phe->decrypt(ha, aa);
    phe->decrypt(hb, bb);
    mpz_mul(ha, ha, hb);
    
    // phe->encrypt(ha, ha);
    phe->mul(ha, one.ptr, ha);

    phe->mul(aa, a, r2);
    phe->sub(aa, ha, aa);
    phe->mul(bb, b, r1);
    phe->sub(aa, aa, bb);
    
    mpz_t rr;
    mpz_init(rr);
    mpz_mul(rr, r1, r2);
    // phe->encrypt(rr, rr);
    phe->mul(rr, one.ptr, rr);
    phe->sub(result, aa, rr);

    mpz_clear(rr);
    mpz_clear(er1);
    mpz_clear(er2);
    mpz_clear(aa);
    mpz_clear(bb);
    mpz_clear(ha);
    mpz_clear(hb);
    mpz_clear(r1);
    mpz_clear(r2);
}

void SEQ(mpz_t result, mpz_t a, mpz_t b) {
    MPZ_T r1, r2, theta, mu, er2;
    phe->random_nbit(r1.ptr, phe->k_);
    phe->random_nbit(r2.ptr, phe->k_);
    mpz_add(r1.ptr, r1.ptr, r2.ptr);
    // phe->encrypt(er2.ptr, r2.ptr);
    phe->mul(er2.ptr, one.ptr, r2.ptr);
    int s = rand()&1;

    // theta = r1*((a-b)^2) - r2
    phe->sub(theta.ptr, a, b);
    SM(theta.ptr, theta.ptr, theta.ptr);
    phe->mul(theta.ptr, theta.ptr, r1.ptr);
    phe->sub(theta.ptr, theta.ptr, er2.ptr);
    if (!s) {
        phe->neg(theta.ptr, theta.ptr);
    }
    
    phe->decode_decrypt(theta.ptr, theta.ptr);
    if (mpz_cmp_si(theta.ptr, 0) < 0) {
        // phe->encrypt(mu.ptr, 1ul);
        mpz_set(mu.ptr, one.ptr);
    } else {
        // phe->encrypt(mu.ptr, 0ul);
        mpz_set(mu.ptr, zero.ptr);
    }
    
    if (s) {
        mpz_set(result, mu.ptr);
    } else {
        phe->sub(result, one.ptr, mu.ptr);
    }
}

void SLESS(mpz_t result, mpz_t a, mpz_t b) {
    // MPZ_T a1, a2;
    // SLEQ(a1.ptr, a, b);
    // SEQ(a2.ptr, a, b);
    // phe->sub(result, a1.ptr, a2.ptr);
    mpz_t r1, r2, tmp;
    mpz_init(r1);
    mpz_init(r2);
    int s = rand()&1;
    phe->random_nbit(r1, phe->K());
    phe->random_nbit(r2, phe->K());
    mpz_add(r1, r1, r2);
    MPZ_T er2;
    phe->mul(er2.ptr, one.ptr, r2);
    // r1(a-b)+r2
    MPZ_T theta;
    phe->sub(theta.ptr, a, b);
    phe->mul(theta.ptr, theta.ptr, r1);
    phe->add(theta.ptr, theta.ptr, er2.ptr);
    if (!s) {
        phe->neg(theta.ptr, theta.ptr);
    }
    phe->decode_decrypt(theta.ptr, theta.ptr);

    MPZ_T mu;
    if (mpz_cmp_si(theta.ptr, 0) < 0) {
        // phe->encrypt(mu.ptr, 1ul);
        mpz_set(mu.ptr, one.ptr);
    } else {
        // phe->encrypt(mu.ptr, 0ul);
        mpz_set(mu.ptr, zero.ptr);
    }

    if (s) {
        mpz_set(result, mu.ptr);
    } else {
        phe->sub(result, one.ptr, mu.ptr);
    }
    mpz_clear(r1);
    mpz_clear(r2);

}

// paper algorithm

void SAV(mpz_t result, mpz_t a, mpz_t b) {
    int s = rand()&1;
    mpz_t r1, r2;
    mpz_init(r1);
    mpz_init(r2);
    phe->random_nbit(r1, phe->K());
    phe->random_nbit(r2, phe->K());
    mpz_add(r1, r1, r2);
    mpz_t tmp;
    mpz_init(tmp);
    if (s == 1) {
        phe->sub(tmp, a, b);
    } else {
        phe->sub(tmp, b, a);
    }
    mpz_t er2;
    mpz_init(er2);
    // phe->encrypt(er2, r2);
    phe->mul(er2, one.ptr, r2);
    phe->mul(tmp, tmp, r1);
    phe->add(tmp, tmp, er2);
    
    mpz_t alpha, beta;
    mpz_init_set(alpha, tmp);
    mpz_init(beta);
    phe->decode_decrypt(tmp, tmp);
    if (mpz_cmp_si(tmp, 0) >= 0) {
        // phe->encrypt(beta, 0ul);
        mpz_set(beta, zero.ptr);
    } else {
        phe->neg(alpha, alpha);
        // phe->encrypt(beta, 2ul);
        phe->add(beta, one.ptr, one.ptr);
    }
    
    phe->mul(tmp, beta, r2);
    phe->add(tmp, tmp, alpha);
    
    phe->sub(tmp, tmp, er2);
    phe->div(result, tmp, r1);
    
    mpz_clear(er2);
    mpz_clear(alpha);
    mpz_clear(beta);
    mpz_clear(r1);
    mpz_clear(r2);
    mpz_clear(tmp);
}

void SSC(mpz_t result, MPZ_VECTOR& xa, MPZ_VECTOR& xb, MPZ_VECTOR& q) {
    MPZ_T r1, r2, r3, r4, er2, er4;
    int s;
    const int d = xa.size();
    assert(d > 0);
    assert(xb.size() == d);
    assert(q.size() == d);

    MPZ_VECTOR alphav(d), betav(d);
    MPZ_T theta1;
    for (int i(0); i < d; ++i) {
        s = rand()&1;
        phe->random_nbit(r1.ptr, phe->k_);
        phe->random_nbit(r2.ptr, phe->k_);
        phe->random_nbit(r3.ptr, phe->k_);
        phe->random_nbit(r4.ptr, phe->k_);
        mpz_add(r1.ptr, r1.ptr, r2.ptr);
        mpz_add(r3.ptr, r3.ptr, r4.ptr);
        // phe->encrypt(er2.ptr, r2.ptr);
        // phe->encrypt(er4.ptr, r4.ptr);
        phe->mul(er2.ptr, one.ptr, r2.ptr);
        phe->mul(er4.ptr, one.ptr, r4.ptr);
        auto& alpha = alphav[i];
        auto& beta = betav[i];
        phe->sub(alpha, xa[i], q[i]);
        if (s) {
            phe->sub(beta, xb[i], q[i]);
        } else {
            phe->sub(beta, q[i], xb[i]);
            mpz_setbit(theta1.ptr, i);
        }
        phe->mul(alpha, alpha, r1.ptr);
        phe->add(alpha, alpha, er2.ptr);
        phe->mul(beta, beta, r3.ptr);
        phe->add(beta, beta, er4.ptr);
    }
    // phe->encrypt(theta1.ptr, theta1.ptr);
    phe->mul(theta1.ptr, one.ptr, theta1.ptr);

    MPZ_T pta, ptb, theta2;
    for (int i(0); i < d; ++i) {
        auto& alpha = alphav[i];
        auto& beta = betav[i];
        phe->decode_decrypt(pta.ptr, alpha);
        phe->decode_decrypt(ptb.ptr, beta);
        auto ca = mpz_cmp_si(pta.ptr, 0);
        auto cb = mpz_cmp_si(ptb.ptr, 0);
        auto tau = ((ca > 0) && (cb > 0)) || ((ca < 0) && (cb < 0));
        if (!tau) {
            mpz_setbit(theta2.ptr, i);
        }
    }
    // phe->encrypt(theta2.ptr, theta2.ptr);
    phe->mul(theta2.ptr, one.ptr, theta2.ptr);
    SEQ(result, theta1.ptr, theta2.ptr);
}

void SDD(mpz_t result, MPZ_VECTOR& xa, MPZ_VECTOR& xb) {
    MPZ_T r1, r2, er2;
    const int d = xa.size();
    assert(d > 0);
    assert(xb.size() == d);
    MPZ_VECTOR alphav(d);
    int s;
    MPZ_T theta1;
    for (int i(0); i < d; ++i) {
        auto& alpha = alphav[i];
        phe->random_nbit(r1.ptr, phe->k_);
        phe->random_nbit(r2.ptr, phe->k_);
        mpz_add(r1.ptr, r1.ptr, r2.ptr);
        // phe->encrypt(er2.ptr, r2.ptr);
        phe->mul(er2.ptr, one.ptr, r2.ptr);
        s = rand()&1;
        phe->sub(alpha, xa[i], xb[i]);
        phe->mul(alpha, alpha, r1.ptr);
        phe->sub(alpha, alpha, er2.ptr);
        if (s) { 
            mpz_setbit(theta1.ptr, i);
        } else {
            phe->neg(alpha, alpha);
        }
    }
    // phe->encrypt(theta1.ptr, theta1.ptr);
    phe->mul(theta1.ptr, one.ptr, theta1.ptr);
    
    MPZ_T theta2;
    for (int i(0); i < d; ++i) {
        auto& alpha = alphav[i];
        phe->decode_decrypt(alpha, alpha);
        if (mpz_cmp_si(alpha, 0) < 0) {
            mpz_setbit(theta2.ptr, i);
        }
    }
    // phe->encrypt(theta2.ptr, theta2.ptr);
    phe->mul(theta2.ptr, one.ptr, theta2.ptr);
    
    MPZ_T delta1;
    SEQ(delta1.ptr, theta1.ptr, theta2.ptr);
    MPZ_T sa, sb;
    // phe->encrypt(sa.ptr, 0ul);
    // phe->encrypt(sb.ptr, 0ul);
    mpz_set(sa.ptr, zero.ptr);
    mpz_set(sb.ptr, zero.ptr);
    for (int i(0); i < d; ++i) {
        phe->add(sa.ptr, sa.ptr, xa[i]);
        phe->add(sb.ptr, sb.ptr, xb[i]);
    }
    MPZ_T delta2;
    SLESS(delta2.ptr, sa.ptr, sb.ptr);
    SM(result, delta1.ptr, delta2.ptr);
}

void SWQ(mpz_t result, MPZ_VECTOR& xa, MPZ_VECTOR& xb, MPZ_VECTOR& axa) {
    MPZ_T r1, r2, r3, r4, er2, er4;
    const int d = xa.size();
    assert(d > 0);
    assert(xb.size() == d);
    assert(axa.size() == d);
    MPZ_VECTOR alphav(2*d);
    MPZ_T theta1;
    for (int i(0); i < 2*d; ++i) {
        phe->random_nbit(r1.ptr, phe->k_);
        phe->random_nbit(r2.ptr, phe->k_);
        mpz_add(r1.ptr, r1.ptr, r2.ptr);
        // phe->encrypt(er2.ptr, r2.ptr);
        phe->mul(er2.ptr, one.ptr, r2.ptr);
        phe->random_nbit(r3.ptr, phe->k_);
        phe->random_nbit(r4.ptr, phe->k_);
        mpz_add(r3.ptr, r3.ptr, r4.ptr);
        // phe->encrypt(er4.ptr, r4.ptr);
        phe->mul(er4.ptr, one.ptr, r4.ptr);
        auto& alpha = alphav[i];
        int s = rand()&1;
        if (i < d) {
            phe->sub(alpha, xa[i], axa[i]);
            phe->sub(alpha, alpha, xb[i]);
            phe->mul(alpha, alpha, r1.ptr);
            phe->sub(alpha, alpha, er2.ptr);
        } else {
            phe->sub(alpha, xb[i-d], axa[i-d]);
            phe->sub(alpha, alpha, xa[i-d]);
            phe->mul(alpha, alpha, r3.ptr);
            phe->sub(alpha, alpha, er4.ptr);
        }
        if (s) {
            mpz_setbit(theta1.ptr, i);
        } else {
            phe->neg(alpha, alpha);
        }
    }
    // phe->encrypt(theta1.ptr, theta1.ptr);
    phe->mul(theta1.ptr, one.ptr, theta1.ptr);
    
    MPZ_T theta2;
    for (int i(0); i < 2*d; ++i) {
        auto& alpha = alphav[i];
        phe->decode_decrypt(alpha, alpha);
        if (mpz_cmp_si(alpha, 0) < 0) {
            mpz_setbit(theta2.ptr, i);
        }
    }
    // phe->encrypt(theta2.ptr, theta2.ptr);
    phe->mul(theta2.ptr, one.ptr, theta2.ptr);

    SEQ(result, theta1.ptr, theta2.ptr);
}

std::vector<int> PPRSQ(std::vector<MPZ_VECTOR>& chi, MPZ_VECTOR& q) {
    // clock_t st, et;
    // int stage(0);
    // st = clock();
    const int n = chi.size();
    const int d = q.size();
    MPZ_T r1;
    phe->random_nbit(r1.ptr, phe->k_);
    mpz_setbit(r1.ptr, phe->k_);
    std::vector<MPZ_VECTOR> abs_chi(n, MPZ_VECTOR(d));
    MPZ_VECTOR t(n);
    for (int i(0); i < t.size(); ++i) {
        mpz_set(t[i], zero.ptr);
    }
    for (int i(0); i < n; ++i) {
        for (int j(0); j < d; ++j) {
            SAV(abs_chi[i][j], chi[i][j], q[j]);
            phe->add(t[i], t[i], abs_chi[i][j]);
        }
    }
    MPZ_T erj;
    for (int i(0); i < t.size(); ++i) {
        phe->random_nbit(erj.ptr, phe->k_-1);
        phe->mul(erj.ptr, one.ptr, erj.ptr);
        auto& tj = t[i];
        phe->mul(tj, tj, r1.ptr);
        phe->add(tj, tj, erj.ptr);
    }
    
    // et = clock();
    // printf("[stage %d] consume time: %lfs\n", stage++, EXECUTE_SECOND(st, et));
    // S2
    // st = clock();
    for (int i(0); i < t.size(); ++i) {
        phe->decode_decrypt(t[i], t[i]);
    }
    std::vector<int> sidx(n);
    std::iota(sidx.begin(), sidx.end(), 0);
    std::sort(sidx.begin(), sidx.end(), [&](int a, int b){
        return mpz_cmp(t[a], t[b]) < 0;
    });

    std::vector<MPZ_VECTOR> schi(n), sabs_chi(n);
    for (int i(0); i < n; ++i) {
        int idx = sidx[i];
        schi[i] = chi[idx];
        sabs_chi[i] = abs_chi[idx];
    }
    // et = clock();
    // printf("[stage %d] consume time: %lfs\n", stage++, EXECUTE_SECOND(st, et));
    // st = clock();
    // int cnt(0);
    MPZ_VECTOR f(n);
    MPZ_T delta1, delta2, delta, rj;
    for (int j(0); j < n; ++j) {
        mpz_set(f[j], zero.ptr);
        for (int k(0); k < j; ++k) {
            SSC(delta1.ptr, schi[k], schi[j], q);
            SDD(delta2.ptr, sabs_chi[k], sabs_chi[j]);
            SM(delta.ptr, delta1.ptr, delta2.ptr);
            phe->add(f[j], f[j], delta.ptr);
            // cnt++;
        }
        phe->random_nbit(rj.ptr, phe->k_);
        phe->mul(f[j], f[j], rj.ptr);
        // if (j%50 == 0 || j == n-1) {
        //     et = clock();
        //     printf("[stage %d, cnt = %d] consume time: %lfs\n", stage, cnt, EXECUTE_SECOND(st, et));
        //     cnt = 0;
        //     st = clock();
        // }
    }
    // stage++;
    // et = clock();
    // printf("[stage %d] consume time: %lfs\n", stage++, EXECUTE_SECOND(st, et));
    // st = clock();
    // S2
    std::vector<int> res;
    MPZ_T rf;
    for (int idx(0); idx < n; ++idx) {
        phe->decode_decrypt(rf.ptr, f[idx]);
        if (mpz_cmp_ui(rf.ptr, 0) == 0) {
            res.push_back(idx);
        }
    }
    
    // S1
    for (int j(0); j < res.size(); ++j) {
        int idx = res[j];
        mpz_set(f[idx], zero.ptr);
        for (int k(0); k < n; ++k) {
            if (k == idx) continue;
            SWQ(delta.ptr, schi[k], schi[idx], sabs_chi[idx]);
            phe->add(f[idx], f[idx], delta.ptr);
        }
        phe->random_nbit(rj.ptr, phe->k_);
        phe->mul(f[idx], f[idx], rj.ptr);
    }
    
    std::vector<int> res2;
    for (int idx : res) {
        phe->decode_decrypt(rf.ptr, f[idx]);
        if (mpz_cmp_ui(rf.ptr, 0) == 0) {
            res2.push_back(idx);
        }
    }
    for (int i(0); i < res2.size(); ++i) {
        res2[i] = sidx[res2[i]];
    }
    
    // et = clock();
    // printf("[stage %d] consume time: %lfs\n", stage++, EXECUTE_SECOND(st, et));
    
    return res2;
}

void protocol_test()
{
    clock_t st, et;
    const int DUPCNT = 100;
    srand(time(nullptr));
    phe = new FHE::PHE(256);
    phe->encrypt(one.ptr, 1ul);
    st = clock();
    for (int i(0); i < DUPCNT; ++i) {
        phe->encrypt(zero.ptr, 0ul);
    }
    et = clock();
    cout << "paillier encryption consume time: " << (double)EXECUTE_SECOND(st, et)/DUPCNT << "s" << endl;
    mpz_t a, b, r, t, p, q;
    mpz_init(a);
    mpz_init(b);
    mpz_init(r);
    mpz_init(t);
    mpz_init(p);
    mpz_init(q);
    phe->encode_encrypt(a, 3);
    phe->encode_encrypt(b, 5);
    SAV(r, a, b);
    SAV(t, b, a);
    phe->decode_decrypt(r, r);
    phe->decode_decrypt(t, t);
    gmp_printf("|3 - 5| = %Zd\n|5 - 3| = %Zd\n", r, t);
    st = clock();
    for (int i(0); i < DUPCNT; ++i) {
        SM(r, a, b);
    }
    et = clock();
    cout << "SM consume time: " << (double)EXECUTE_SECOND(st, et)/DUPCNT << "s" << endl;
    phe->decode_decrypt(r, r);
    gmp_printf("3 * 5 = %Zd\n", r);

    SLESS(r, a, b);
    SLESS(t, b, a);
    SLESS(q, a, a);
    phe->decode_decrypt(q, q);
    phe->decode_decrypt(r, r);
    phe->decode_decrypt(t, t);
    gmp_printf("bool(3 < 5) = %Zd\nbool(5 < 3) = %Zd\nbool(3 < 3) = %Zd\n", r, t, q);

    SEQ(r, a, b);
    SEQ(t, a, a);
    SEQ(q, b, a);
    phe->decode_decrypt(q, q);
    phe->decode_decrypt(r, r);
    phe->decode_decrypt(t, t);
    gmp_printf("bool(3 == 5) = %Zd\nbool(3 == 3) = %Zd\nbool(5 == 3) = %Zd\n", r, t, q);

    SLEQ(r, a, b);
    SLEQ(t, a, a);
    SLEQ(q, b, a);
    phe->decode_decrypt(q, q);
    phe->decode_decrypt(r, r);
    phe->decode_decrypt(t, t);
    gmp_printf("bool(3 <= 5) = %Zd\nbool(3 <= 3) = %Zd\nbool(5 <= 3) = %Zd\n", r, t, q);
    
    const int d = 3;
    std::vector<int> xv = {2,3,1};
    std::vector<int> yv = {4,3,2};
    std::vector<int> qv = {1,1,1};
    MPZ_VECTOR xa(d), xb(d), eq(d);
    for (int i(0); i < d; ++i) {
        phe->encode_encrypt(xa[i], xv[i]);
        phe->encode_encrypt(xb[i], yv[i]);
        phe->encode_encrypt(eq[i], qv[i]);
    }
    st = clock();
    for (int i(0); i < DUPCNT; ++i) {
        SSC(r, xa, xb, eq);
    }
    et = clock();
    cout << "SSC consume time: " << (double)EXECUTE_SECOND(st, et)/DUPCNT << "s" << endl;
    phe->decode_decrypt(r, r);
    for (int i(0); i < d; ++i) {
        cout << (xv[i]-qv[i])*(yv[i]-qv[i]) << ", ";
    }
    gmp_printf(" SSC = %Zd\n", r);

    MPZ_VECTOR axa(d), axb(d);
    for (int i(0); i < d; ++i) {
        SAV(axa[i], xa[i], eq[i]);
        SAV(axb[i], xb[i], eq[i]);
    }
    st = clock();
    for (int i(0); i < DUPCNT; ++i) {
        SDD(r, axa, axb);
    }
    et = clock();
    cout << "SDD consume time: " << (double)EXECUTE_SECOND(st, et)/DUPCNT << "s" << endl;
    phe->decode_decrypt(r, r);
    for (int i(0); i < d; ++i) {
        printf("(%d, %d), ", abs(xv[i]-qv[i]), abs(yv[i]-qv[i]));
    }
    gmp_printf(" SDD = %Zd\n", r);

    st = clock();
    for (int i(0); i < DUPCNT; ++i) {
        SWQ(r, xa, xb, axa);
    }
    et = clock();
    cout << "SWQ consume time: " << (double)EXECUTE_SECOND(st, et)/DUPCNT << "s" << endl;
    phe->decode_decrypt(r, r);
    for (int i(0); i < d; ++i) {
        int a1 = abs(xv[i] - qv[i]);
        printf("%d in [%d, %d], ", yv[i], xv[i]-a1, xv[i]+a1);
    }
    gmp_printf(" SWQ = %Zd\n", r);

    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(r);
    mpz_clear(t);
    delete phe;
}

void emulater(const std::string& filepath, const std::vector<int> q) {
    auto dataset = utility::read_dataset(filepath);
    const int n = dataset.size();
    const int d = q.size();
    phe = new FHE::PHE(256);
    phe->encrypt(zero.ptr, 0ul);
    phe->encrypt(one.ptr, 1ul);

    clock_t st, et;
    
    // dataset encryption
    st = clock();
    std::vector<MPZ_VECTOR> chi(n, MPZ_VECTOR(d));
    for (int i(0); i < n; ++i) {
        for (int j(0); j < d; ++j) {
            phe->encode_encrypt(chi[i][j], dataset[i][j]);
        }
    }
    et = clock();
    cout << "dataset encryption time: " << EXECUTE_SECOND(st, et) << "s." << endl;

    // query encryption
    st = clock();
    MPZ_VECTOR eq(d);
    for (int i(0); i < d; ++i) {
        phe->encode_encrypt(eq[i], q[i]);
    }
    et = clock();
    cout << "query encryption time: " << EXECUTE_SECOND(st, et) << "s." << endl;

    // search 
    st = clock();
    auto res = PPRSQ(chi, eq);
    et = clock();
    cout << "search time: " << EXECUTE_SECOND(st, et) << "s." << endl;
    
    // end
    cout << "result: " << "{";
    for (int i : res) {
        cout << i << ", ";
    }
    cout << "}\n";
    delete phe;
}

void submain()
{
    protocol_test();
}