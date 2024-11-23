/*
 * @Author: paangyubok 2837839125@qq.com
 * @Date: 2023-05-26 17:08:36
 * @LastEditors: paangyubok 2837839125@qq.com
 * @LastEditTime: 2023-05-30 16:09:37
 * @FilePath: /wlby/globcom/src/globcom.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <string>
#include <vector>
#include <gmp.h>

void SLEQ(mpz_t, mpz_t, mpz_t);
void SM(mpz_t, mpz_t, mpz_t);
void SLESS(mpz_t, mpz_t, mpz_t);
void SAV(mpz_t, mpz_t, mpz_t);
void SEQ(mpz_t, mpz_t, mpz_t);

void submain();
void emulater(const std::string& filepath, const std::vector<int> q);
void my_emulater(const std::string& filepath, const std::vector<int> q);