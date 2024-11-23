#include "tools.h"
#include <cstddef>
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "pbcwrapper/PBC.h"
#include <openssl/sha.h>

using namespace std;

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

string hex_to_bytes(const std::string& hex) {
  string bytes;
  for (unsigned int i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    char byte = (char) strtol(byteString.c_str(), nullptr, 16);
    bytes.push_back(byte);
  }
  return bytes;
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