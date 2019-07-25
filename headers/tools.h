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
