#include "emulator.h"
#include "tools.h"
#include <cstddef>
#include <gmp.h>
#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/vector.h>
#include <gmpxx.h>
#include <sstream>

using namespace std;
using namespace NTL;

void xxx()
{
    ZZ a;
    a = 123456789123456789;
    mpz_class ma;
    stringstream ss;
    ss << a;
    ss >> ma;
    cout << ma << endl;

    ZZ_p::init(GenPrime_ZZ(10));
    vec_ZZ_p x;
    vec_ZZ_p y;
    int len = 3;
    x.SetLength(len);
    y.SetLength(len);
    for (int i(0); i < len; ++i) {
      x[i] = i;
      y[i] = (i+1)*(i+2);
    }


    ZZ_pX poly;
    interpolate(poly, x, y);
    for (int i(0); i < len; ++i)
      cout << eval(poly, ZZ_p(i)) << endl;
}

int main(int argc, char **argv)
{
    // xxx();
    // return 0;
    if (argc > 1) {
      EmulatorParams ep;
      // ep.filename = argv[1];
      ep.n = strtol(argv[1], nullptr, 0);
      emulator(&ep);
    } else {
      emulator(nullptr);
    }
    // testcode();
    return 0;
}
