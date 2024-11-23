#include "tools.h"
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <cstdio>
#include <gmp.h>
#include <gmpxx.h>
#include <pbc/pbc_field.h>
#include <pbc/pbc_pairing.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "pbcwrapper/PBC.h"
#include <openssl/sha.h>
#include <sstream>

using namespace std;
using namespace NTL;

int get_n_from_filename(const string& filename)
{
    auto bgn = filename.find('n');
    auto end = filename.find('-');
    if (bgn == filename.npos || end == filename.npos) {
        return -1;
    }
    bgn += 1;
    return std::stoi(filename.substr(bgn, end - bgn));
}

string byte_to_hex(const unsigned char* buf, int len) {
    string ret;
    ret.resize(len*2);
    for (int i(0); i < len; ++i) {
        sprintf(&ret[i*2], "%02x", buf[i]);
    }
    return ret;
}

string sha1_wrapper(const std::string &data)
{
    unsigned char buf[20];
    SHA1((const unsigned char*)data.c_str(), data.size(), buf);
    return byte_to_hex(buf, sizeof(buf));
}

string sha256_wrapper(const std::string & data)
{
    unsigned char buf[32];
    SHA256((const unsigned char *)data.c_str(), data.size(), buf);
    return byte_to_hex(buf, sizeof(buf));
}

void LOG(const char* level, const std::string& info)
{
    printf("[%s] %s\n", level, info.c_str());
}

mpz_class ZZtoMpzClass(const ZZ& num)
{
    stringstream ss;
    ss << num;
    mpz_class mpz_num;
    ss >> mpz_num;
    return mpz_num;
}

mpz_class ZZptoMpzClass(const ZZ_p& num)
{
    return ZZtoMpzClass(rep(num));
}

ZZ MpzClasstoZZ(const mpz_class& num)
{
    stringstream ss;
    ss << num;
    ZZ ret;
    ss >> ret;
    return ret;
}

ZZ_p MpzClasstoZZp(const mpz_class& num)
{
    stringstream ss;
    ss << num;
    ZZ_p ret;
    ss >> ret;
    return ret;
}

Zr MpzClasstoZr(const Pairing& e, mpz_class num)
{
    element_t tmp;
    element_init_Zr(tmp, const_cast<pairing_s*>(e.getPairing()));
    element_set_mpz(tmp, num.get_mpz_t());
    Zr ret;
    ret.setElement(tmp);
    element_clear(tmp);
    return ret;
}

Zr ZZptoZr(const Pairing& e, const NTL::ZZ_p& num)
{
    return MpzClasstoZr(e, ZZptoMpzClass(num));
}