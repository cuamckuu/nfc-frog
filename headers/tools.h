#ifndef TOOLS_H
#define TOOLS_H

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>

#include <nfc/nfc-types.h>

#define MAX_FRAME_LEN 300

// Macro to print unsigned chars in hexadecimal
#define HEX(c)                                                                 \
    std::hex << std::uppercase << std::setw(2) << std::setfill('0')            \
             << (unsigned int)c << std::dec

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

    #define GREEN(x) x
    #define RED(x) x

#else

    std::string const red("\033[0;31m");
    std::string const green("\033[0;32m");
    std::string const reset("\033[0m");

    #define GREEN(x) green << x << reset
    #define RED(x) red << x << reset

#endif


typedef unsigned char byte_t;

struct APDU {
    size_t size;
    byte_t data[MAX_FRAME_LEN];
};

template<class DestT, class SrcT>
byte_t parse_TLV(DestT *dest, SrcT *src, size_t &idx) {
    byte_t len = src[++idx];
    std::memcpy(dest, &src[++idx], len);
    idx += len - 1;

    return len;
}

#endif // TOOLS_H
