#include "emulator_struct.h"
#include "fhe.h"
#include "tools.h"
#include <gmp.h>
#include <gmpxx.h>
#include <string>
#include <vector>

using namespace std;
#define NOT_IGNORE(arg) (string(arg)!="-")

void xxx()
{
  string she_params_filename = "../params/she.json";
  FHE::SHE she(8192, 1100, 3000);
  she.save_params_to_file(she_params_filename);
  mpz_class e0, e1;
  e0 = she.encrypt(0);
  e1 = she.encrypt(1);
  cout << "e0 = " << she.decrypt(e0) << endl;
  cout << "e1 = " << she.decrypt(e1) << endl;
  mpz_class e2;
  e2 = she.add(e1, e1);
  cout << "e1 + e1 = " << she.decrypt(e2) << endl;
  e2 = she.add(e1, 1);
  cout << "e1 + 1 = " << she.decrypt(e2) << endl;
  mpz_class res;
  res = she.mul(e1, e1);
  cout << "e1 * e1 = " << she.decrypt(res) << endl;
  res = she.mul(e1, 2);
  cout << "e1 * 2 = " << she.decrypt(res) << endl;

  mpz_class bignum = she.random_nbit(1050);
  cout << "bignum = " << bignum.get_str() << endl;
  auto b1 = she.mul(e1, bignum);
  cout << "bignum * e1 = " << she.decrypt(b1) << endl;
  auto b0 = she.mul(e0, bignum);
  cout << "bignum * e0 = " << she.decrypt(b0) << endl;
  auto badd = she.add(b0, b1);
  cout << "b0 + b1 = " << she.decrypt(badd) << endl;
}

void yyy()
{
  string phe_params_filename = "../params/phe.json";
  FHE::PHE phe(1200);
  phe.save_params_to_file(phe_params_filename);
  mpz_class e0, e1;
  e0 = phe.encrypt(0);
  e1 = phe.encrypt(1);
  cout << "e0 = " << phe.decrypt(e0) << endl;
  cout << "e1 = " << phe.decrypt(e1) << endl;
  mpz_class e2;
  e2 = phe.add(e1, e1);
  cout << "e1 + e1 = " << phe.decrypt(e2) << endl;
  mpz_class res;
  res = phe.mul(e1, 2);
  cout << "e1 * 2 = " << phe.decrypt(res) << endl;
  mpz_class bignum = phe.random_nbit(1050);
  cout << "bignum = " << bignum.get_str() << endl;
  auto b1 = phe.mul(e1, bignum);
  cout << "bignum * e1 = " << phe.decrypt(b1) << endl;
  auto b0 = phe.mul(e0, bignum);
  cout << "bignum * e0 = " << phe.decrypt(b0) << endl;
  auto badd = phe.add(b0, b1);
  cout << "b0 + b1 = " << phe.decrypt(badd) << endl;

  vector<mpz_class> nums;
  for (int i(0); i < 1000; ++i) {
    nums.emplace_back(phe.random_nbit(1050));
  }
  res = e0;
  TimeCounter tc;
  int idx = 1;
  tc.Start();
  for (int i(0); i < nums.size(); ++i) {
    if (i != idx)
      res = phe.add(res, phe.mul(e0, nums[i]));
    else
      res = phe.add(res, phe.mul(e1, nums[i]));
  }
  tc.EndAndPrintMs(to_string(nums.size()) + " mul and add time");
  if (phe.decrypt(res) != nums[idx]) {
    cout << "result is wrong" << endl;
  }
}

int main(int argc, char **argv)
{
    // xxx();
    // yyy();
    // return 0;
    if (argc > 1) {
      EmulatorParams ep;
      ep.filename = argv[1];
      emulator(&ep);
    } else {
      emulator(nullptr);
    }
    // testcode();
    return 0;
}
