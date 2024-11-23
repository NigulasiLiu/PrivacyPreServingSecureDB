/*
 * @Author: paangyubok 2837839125@qq.com
 * @Date: 2023-05-30 15:30:12
 * @LastEditors: paangyubok 2837839125@qq.com
 * @LastEditTime: 2023-06-03 23:11:19
 * @FilePath: /wlby/globcom/src/mine.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "globcom.h"
#include "utility.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <gmp.h>
#include <vector>
#include <Eigen/Dense>
#include "phe.h"

using utility::MPZ_T;
using utility::MPZ_VECTOR;

extern FHE::PHE *phe;
extern MPZ_T zero, one;


void SRDC(mpz_t result, MPZ_VECTOR& xu, MPZ_VECTOR& xv, MPZ_VECTOR& q) {
    const int d = xu.size();
    MPZ_T delta1, delta2;
    MPZ_T left, right, tmp;
    MPZ_T sl, sr;
    for (int i(0); i < d; ++i){
        // SM(left.ptr, xu[i], xu[i]);
        // SM(tmp.ptr, xu[i], xv[i]);
        // phe->mul(tmp.ptr, tmp.ptr, 2);
        // phe->sub(left.ptr, left.ptr, tmp.ptr);

        // SM(right.ptr, q[i], q[i]);
        // SM(tmp.ptr, q[i], xv[i]);
        // phe->mul(tmp.ptr, tmp.ptr, 2);
        // phe->sub(right.ptr, right.ptr, tmp.ptr);
        
        SAV(left.ptr, xu[i], xv[i]);
        SAV(right.ptr, q[i], xv[i]);

        phe->add(sl.ptr, sl.ptr, left.ptr);
        phe->add(sr.ptr, sr.ptr, right.ptr);

        SLEQ(tmp.ptr, left.ptr, right.ptr);
        phe->add(delta1.ptr, delta1.ptr, tmp.ptr);
    }
    MPZ_T encd;
    phe->mul(encd.ptr, one.ptr, d);
    SEQ(delta1.ptr, delta1.ptr, encd.ptr);
    
    SLESS(delta2.ptr, sl.ptr, sr.ptr);

    SM(result, delta1.ptr, delta2.ptr);
}

const uint32_t K0 = 20;
const uint32_t K1 = 90;
const uint32_t K2 = 128;

vector<vector<int>> dataset;
vector<int> plain_q;

vector<int> My_PRSQ(std::vector<MPZ_VECTOR>& chi, MPZ_VECTOR& q,
                std::vector<Eigen::MatrixXd>& mchi) {
    const int n = chi.size();
    const int d = q.size();
    assert(mchi.size() == n);
    assert(mchi[0].rows() == 2);
    MPZ_T tmp, tmp1;
    // (1, lb+ub, lb*ub)
    // lb = xi-|qi-xi|; ub = xi+|qi-xi| 
    MPZ_T t1, t2, t3;
    MPZ_T alpha, r, dst, lb, ub;
    MPZ_T delta1, delta2, delta;
    MPZ_T d_enc_const;
    phe->mul(d_enc_const.ptr, one.ptr, d);
    vector<int> ret;
    for (int i(0); i < n; ++i) {
        vector<vector<Eigen::RowVector4d>> m_xi(d, vector<Eigen::RowVector4d>(2));
        vector<MPZ_VECTOR> homo_xi(d, MPZ_VECTOR(2));
        auto& xi = chi[i];
        for (int k(0); k < d; ++k) {
            // S1
            phe->random_nbit(alpha.ptr, K1);
            mpz_setbit(alpha.ptr, K1-1);
            phe->random_nbit(r.ptr, K0);
            mpz_add(t1.ptr, alpha.ptr, r.ptr); // t1 = alpha + r
            phe->mul(t1.ptr, one.ptr, t1.ptr);

            phe->mul(alpha.ptr, one.ptr, alpha.ptr);
            SAV(dst.ptr, q[k], xi[k]);
            phe->sub(lb.ptr, xi[k], dst.ptr);
            phe->add(ub.ptr, xi[k], dst.ptr);
            // =============================================
            // phe->decode_decrypt(tmp.ptr, dst.ptr);
            // cout << "dst = " << tmp.ptr << endl;
            // phe->decode_decrypt(tmp.ptr, lb.ptr);
            // cout << "lb = " << tmp.ptr << endl;
            // phe->decode_decrypt(tmp.ptr, ub.ptr);
            // cout << "ub = " << tmp.ptr << endl;
            // =============================================
            
            phe->random_nbit(r.ptr, K0);
            phe->mul(r.ptr, one.ptr, r.ptr);
            phe->add(t2.ptr, lb.ptr, ub.ptr);
            SM(t2.ptr, alpha.ptr, t2.ptr);
            phe->add(t2.ptr, t2.ptr, r.ptr); // t2 = alpha * (a+b) + r

            phe->random_nbit(r.ptr, K0);
            phe->mul(r.ptr, one.ptr, r.ptr);
            SM(t3.ptr, lb.ptr, ub.ptr);
            SM(t3.ptr, alpha.ptr, t3.ptr);
            phe->add(t3.ptr, t3.ptr, r.ptr); // t3 = alpha * (a*b) + r

            // ============================================
            // mpz_set(t1.ptr, one.ptr);
            // phe->add(t2.ptr, lb.ptr, ub.ptr);
            // SM(t3.ptr, lb.ptr, ub.ptr);
            // ============================================


            // S2
            phe->decode_decrypt(t1.ptr, t1.ptr);
            phe->decode_decrypt(t2.ptr, t2.ptr);
            phe->decode_decrypt(t3.ptr, t3.ptr);
            auto t1d = mpz_get_d(t1.ptr);
            auto t2d = mpz_get_d(t2.ptr);
            auto t3d = mpz_get_d(t3.ptr);
            // ========================================
            // cout << "x = " << dataset[i][k] << ", q = " << plain_q[k] << endl;
            // cout << "a+b = " << t2d << ", a*b = " << t3d << endl;
            // ========================================
            
            auto ralpha = utility::nbit_random_force(K1);
            auto rbeta = utility::nbit_random_force(K2);
            auto rr = utility::nbit_random_array(K0, 3);
            int s2 = rand()&1 ? 1 : -1;
            ralpha = s2 * ralpha;
            rbeta = s2 * rbeta;
            m_xi[k][0] << -ralpha*t1d+rr[0], ralpha*t2d+rr[1], -ralpha*t3d+rr[2], +rbeta;
            if (s2 == 1) {
                mpz_set(homo_xi[k][0], zero.ptr);
            } else {
                mpz_set(homo_xi[k][0], one.ptr);
            }

            ralpha = utility::nbit_random_force(K1);
            rbeta = utility::nbit_random_force(K2);
            rr = utility::nbit_random_array(K0, 3);
            s2 = rand()&1 ? 1 : -1;
            ralpha = s2 * ralpha;
            rbeta = s2 * rbeta;
            m_xi[k][1] << -ralpha*t1d+rr[0], ralpha*t2d+rr[1], -ralpha*t3d+rr[2], -rbeta;
            if (s2 == 1) {
                mpz_set(homo_xi[k][1], zero.ptr);
            } else {
                mpz_set(homo_xi[k][1], one.ptr);
            }
        }


        mpz_set(delta.ptr, zero.ptr);
        for (int j(0); j < n; ++j) {
            if (i == j) continue;
            auto& mxj = mchi[j];
            mpz_set(delta1.ptr, zero.ptr);
            mpz_set(delta2.ptr, zero.ptr);
            for (int k(0); k < d; ++k) {
                auto res1 = mxj.row(k).dot(m_xi[k][0]);
                if (res1 > 0) {
                    phe->sub(tmp.ptr, one.ptr, homo_xi[k][0]);
                } else {
                    mpz_set(tmp.ptr, homo_xi[k][0]);
                }
                auto res2 = mxj.row(k).dot(m_xi[k][1]);
                if (res2 > 0) {
                    phe->sub(tmp1.ptr, one.ptr, homo_xi[k][1]);
                } else {
                    mpz_set(tmp1.ptr, homo_xi[k][1]);
                }
                phe->add(delta1.ptr, delta1.ptr, tmp.ptr);
                phe->add(delta2.ptr, delta2.ptr, tmp1.ptr);
                
                // ======================================================
                // int left = abs(dataset[j][k]-dataset[i][k]);
                // int right = abs(plain_q[k]-dataset[i][k]);
                // phe->decode_decrypt(tmp.ptr, tmp.ptr);
                // printf("i = %d; j = %d; k = %d; %d <= %d -> %d -- %ld\n", i, j, k, left, right, left<=right, mpz_get_si(tmp.ptr));
                // phe->decode_decrypt(tmp.ptr, tmp1.ptr);
                // printf("i = %d; j = %d; k = %d; %d < %d -> %d -- %ld\n", i, j, k, left, right, left<right, mpz_get_si(tmp.ptr));
                // ======================================================
            }
            SEQ(tmp.ptr, delta1.ptr, d_enc_const.ptr);
            SM(tmp.ptr, tmp.ptr, delta2.ptr);
            phe->add(delta.ptr, delta.ptr, tmp.ptr);
        }
        
        phe->decode_decrypt(tmp.ptr, delta.ptr);
        if (mpz_cmp_si(tmp.ptr, 0) == 0) {
            ret.push_back(i);
        }
    }
    return ret;
}

vector<int> SRSQ(std::vector<MPZ_VECTOR>& chi, MPZ_VECTOR& q) {
    const int n = chi.size();
    const int d = q.size();
    MPZ_T delta, tmp;
    vector<int> ret;
    for (int i(0); i < n; ++i) {
        mpz_set(delta.ptr, zero.ptr);
        for (int j(0); j < n; ++j) {
            SRDC(tmp.ptr, chi[j], chi[i], q);
            phe->add(delta.ptr, delta.ptr, tmp.ptr);
        }
        SEQ(tmp.ptr, delta.ptr, zero.ptr);
        phe->decode_decrypt(tmp.ptr, tmp.ptr);
        if (mpz_cmp_si(tmp.ptr, 1) == 0) {
            ret.push_back(i);
        }
    }
    return ret;
}
    
void my_emulater(const std::string& filepath, const std::vector<int> q) {
    dataset = utility::read_dataset(filepath);
    plain_q = q;
    const int n = dataset.size();
    const int d = q.size();
    phe = new FHE::PHE(256);
    phe->encrypt(zero.ptr, 0ul);
    phe->encrypt(one.ptr, 1ul);

    clock_t st, et;

    // dataset encryption
    st = clock();
    std::vector<MPZ_VECTOR> chi(n, MPZ_VECTOR(d));
    std::vector<Eigen::MatrixXd> mchi(n, Eigen::MatrixXd(d, 4));
    for (int i(0); i < n; ++i) {
        for (int j(0); j < d; ++j) {
            phe->encode_encrypt(chi[i][j], dataset[i][j]);
            auto& num = dataset[i][j];
            auto alpha = utility::nbit_random_force(K1);
            auto beta = utility::nbit_random_force(K2);
            auto rr = utility::nbit_random_array(K0, 3);
            mchi[i].row(j) << alpha*num*num+rr[0], alpha*num+rr[1], alpha+rr[2], beta; 
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
    // auto res = SRSQ(chi, eq);
    auto res = My_PRSQ(chi, eq, mchi);
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

   