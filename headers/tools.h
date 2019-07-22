#ifndef __TOOLS_HH__
#define __TOOLS_HH__

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>

#include <nfc/nfc-types.h>

//#define DEBUG

#define MAX_FRAME_LEN 300

// Macro to print unsigned chars in hexadecimal
#define HEX(c)                                                                 \
    std::hex << std::uppercase << std::setw(2) << std::setfill('0')            \
             << (unsigned int)c << std::dec

typedef unsigned char byte_t;

extern nfc_device *pnd;

struct Application {
    byte_t priority = 0;
    byte_t aid[7] = {};
    char name[128] = {};
};

struct APDU {
    size_t size;
    byte_t data[MAX_FRAME_LEN];
};

// Command list
class Command {
  public:
    static const byte_t START_14443A[3];
    static const byte_t SELECT_PPSE[22];
    static const byte_t SELECT_APP_HEADER[6];
    static const byte_t GPO_HEADER[6];
    static const byte_t READ_RECORD[7];
    static const byte_t GET_DATA_LOG_FORMAT[7];
};

// Misc tools for printing
class Tools {
  public:
    static void print(char const *str, std::string const &label = "");
    static void printChar(byte_t const *str, size_t size,
                          std::string const & = "");
    static void printHex(APDU const &, std::string const & = "");
    static void printHex(byte_t const *str, size_t size,
                         std::string const & = "");
};

template<class DestT, class SrcT>
byte_t parse_TLV(DestT *dest, SrcT *src, size_t &idx) {
    byte_t len = src[++idx];
    std::memcpy(dest, &src[++idx], len);
    idx += len - 1;

    return len;
}

#endif // __TOOLS_HH__
