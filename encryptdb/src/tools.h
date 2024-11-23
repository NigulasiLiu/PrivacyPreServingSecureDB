#pragma once
#include <string>
#include <sys/time.h>

int get_n_from_filename(const std::string& filename);
std::string sha1_wrapper(const std::string& data);
std::string sha256_wrapper(const std::string & data);

std::string byte_to_hex(const unsigned char* buf, int len);
std::string hex_to_bytes(const std::string& hex);
void LOG(const char* level, const std::string& info);
#define LOG_FATAL(info) LOG("Fatal", info)
#define LOG_WARN(info) LOG("Warn", info)
#define LOG_INFO(info) LOG("Info", info)

class TimeCounter
{
    timeval st;
    timeval et;
public:
    enum TimeType {us, ms, s};
    void Start() {
        gettimeofday(&st, nullptr);
    }
    void End() {
        gettimeofday(&et, nullptr);
    }
    void PrintElapsed(TimeType t, const std::string& info = "") {
        unsigned long uselapsed = (et.tv_sec-st.tv_sec)*1000000 +
		(et.tv_usec-st.tv_usec);
        std::string word;
        double div;
        switch (t) {
            case us: word = "us"; div = 1.0; break;
            case ms: word = "ms"; div = 1000.0; break;
            case s: word = "s"; div = 1000000.0; break;
        }
        printf("%s %.2f %s\n",info.c_str(), uselapsed/div, word.c_str());
    }
    void EndAndPrintSec(const std::string& info) {
        End();
        PrintElapsed(s, info);
    }

    void EndAndPrintMs(const std::string& info) {
        End();
        PrintElapsed(ms, info);
    }
};