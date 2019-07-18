/*
  Copyright (C) 2014 Alexis Guillard, Maxime Marches, Thomas Brunner

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  File written for the requirements of our MSc Project at the University of Kent, Canterbury, UK

  Retrieves information available from EMV smartcards via an RFID/NFC reader.
  Both tracks are printed then track2 is parsed to retrieve PAN and expiry date.
  The paylog is parsed and showed as well.

  All these information are stored in plaintext on the card and available to anyone.

  Requirements:
  libnfc (>= 1.7.1) -> For later versions, please update the pn52x_transceive() prototype if needed, as it is not included in nfc.h

*/

#ifndef __TOOLS_HH__
# define __TOOLS_HH__

#include <string>
#include <cstdio>
#include <iostream>
#include <iomanip>

#include <nfc/nfc-types.h>

//#define DEBUG

#define MAX_FRAME_LEN 300

// Macro to print unsigned chars in hexadecimal
#define HEX(c) std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (unsigned int)c << std::dec

typedef unsigned char byte_t;

extern nfc_device* pnd;

struct Application {
  byte_t priority;
  byte_t aid[7];
  char name[128];
};

struct APDU {
  int size;
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
  static void print(char const* str, std::string const& label = "");
  static void printChar(byte_t const* str, size_t size, std::string const& = "");
  static void printHex(APDU const&, std::string const& = "");
  static void printHex(byte_t const* str, size_t size, std::string const& = "");
};

#endif // __TOOLS_HH__
