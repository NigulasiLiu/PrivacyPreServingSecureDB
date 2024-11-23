#include "emulator_struct.h"
#include <gmp.h>

using namespace std;

long EdbElement::lowbit_len = 11;

mpz_class EdbElement::toMpzClass(bool is_debug) const {
    long lowbit_shift = lowbit_len;

    string gs_str = gs.toString(true);
    auto gs_hex_str = byte_to_hex((const unsigned char*)gs_str.c_str(), gs_str.size());
    mpz_class mpz_gs;
    mpz_gs.set_str(gs_hex_str, 16);
    long num_shift = mpz_sizeinbase(num.get_mpz_t(), 2);
    // mpz_class ret = (mpz_gs << (num_shift + lowbit_shift)) | (num << lowbit_shift) | num_shift;
    int lowest_bit = num < 0 ? 1 : 0;
    mpz_class num_abs = abs(num);
    mpz_class high, mid, low;
    mpz_mul_2exp(high.get_mpz_t(), mpz_gs.get_mpz_t(), num_shift + lowbit_shift);
    mpz_mul_2exp(mid.get_mpz_t(), num_abs.get_mpz_t(), lowbit_shift);
    low = (num_shift << 1) | lowest_bit;
    mpz_class ret = high + mid + low;
    int element_size_bits = mpz_sizeinbase(ret.get_mpz_t(), 2);
    if (element_size_bits  > 1100) {
        LOG_WARN("EdbElement's size bigger than 1100, size = " + to_string(mpz_sizeinbase(ret.get_mpz_t(), 2)));
    }else {
        cout << "ele_size:" << element_size_bits << " bits" << endl;
    }
    // if (ret < 0) {
    //     cout << "============== toMpzClass =============" << endl;
    //     cout << "ret = " << ret.get_str(16) << endl;
    //     cout << "mpz_gs = " << mpz_gs.get_str(16) << endl;
    //     cout << "mpz_gs after shift = " << ((mpz_class)(mpz_gs << (num_shift+lowbit_shift))).get_str(16) << endl;
    //     cout << "gs_hex_str = " << gs_hex_str << endl;
    //     printf("num_shift = %ld\n", num_shift);
    //     cout << "=======================================" << endl;
    // }
    
    // =============== test ====================
    if (is_debug) {
        cout << "toMpzClass, mpz_gs = " << mpz_gs << endl;
        cout << "toMpzClass, gs_hex_str = " << gs_hex_str << endl;
        cout << "toMpzClass, gs_str = " << gs_str << "End, size = "<< gs_str.size() << endl;
        cout << "toMpzClass, lowest_bit = " << lowest_bit << endl;
        cout << "toMpzClass, num = " << num.get_str(16) << endl;
        cout << "toMpzClass, num_abs = " << num_abs.get_str(16) << endl;
        cout << "toMpzClass, num_shift = " << num_shift << endl;
    }
    // =========================================
    return ret;
}

void EdbElement::fromMpzClass(const Pairing& e, const mpz_class& mpz_obj, bool is_debug)
{
    long lowbit_shift = lowbit_len;
    mpz_class high, mid, low;
    mpz_mod_2exp(low.get_mpz_t(), mpz_obj.get_mpz_t(), lowbit_shift);
    long low_num = low.get_si();
    long num_shift = low_num >> 1;
    long lowest_bit = low_num & 1;

    mpz_div_2exp(mid.get_mpz_t(), mpz_obj.get_mpz_t(), lowbit_shift);
    mpz_mod_2exp(mid.get_mpz_t(), mid.get_mpz_t(), num_shift);
    if (lowest_bit == 1) {
        mpz_neg(num.get_mpz_t(), mid.get_mpz_t());
    } else {
        num = mid;
    }

    mpz_div_2exp(high.get_mpz_t(), mpz_obj.get_mpz_t(), num_shift + lowbit_shift);
    mpz_class mpz_gs = high;
    string gs_hex_str = mpz_gs.get_str(16);
    if (gs_hex_str.size() < 130) {
        // cout << "plus prefix" << endl;
        string prefix;
        prefix.insert(0, 130-gs_hex_str.size(), '0');
        gs_hex_str = prefix + gs_hex_str;
    }
    string gs_str = hex_to_bytes(gs_hex_str);
    
    // ================ test ==================
    if (is_debug) {
        cout << "fromMpzClass, mpz_gs = " << mpz_gs << endl;
        cout << "fromMpzClass, gs_hex_str = " << gs_hex_str << endl;
        cout << "fromMpzClass, gs_str = " << gs_str << "End, size = "<< gs_str.size() << endl;
        cout << "fromMpzClass, lowest_bit = " << lowest_bit << endl;
        cout << "fromMpzClass, num = " << num.get_str(16) << endl;
        cout << "fromMpzClass, mid = " << mid.get_str(16) << endl;
        cout << "fromMpzClass, num_shift = " << num_shift << endl;
    }
    // ========================================
    try {
        this->gs = G1(e, (const unsigned char*)gs_str.c_str(), gs_str.size(), true);
    } catch (CorruptDataException& e) {
        cout << e.what() << endl;
        // cout << "fromMpzClass, mpz_gs = " << mpz_gs << endl;
        // cout << "fromMpzClass, gs_hex_str = " << gs_hex_str << endl;
        // cout << "fromMpzClass, gs_str = " << gs_str << "End" << endl << gs_str.size() << endl;
    }
}
