/*
  File edited by Maxime Marches and Thomas Brunner for the requirements of
  our MSc Project at the University of Kent, Canterbury, UK

  License: distributed under GPL version 3 (http://www.gnu.org/licenses/gpl.html)

  Requirements:
  libnfc (>= 1.7.1)

  Compilation: 
$ gcc cchack.c -lnfc -o readnfccc

*/

#ifndef __CCHACK_HH__
# define __CCHACK_HH__

#include <string>
#include <cstdio>
#include <iostream>
#include <iomanip>

#define DEBUG

#define MAX_FRAME_LEN 300

#define HEX(c) std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (unsigned int)c << std::dec

typedef unsigned char byte_t;

extern struct nfc_device* pnd;

class Command {

public:
  // Command list
  static const byte_t START_14443A[3];
  static const byte_t SELECT_PPSE[22];
  static const byte_t SELECT_APP_HEADER[6];
  static const byte_t GPO_HEADER[6];
  static const byte_t READ_RECORD[7];
  static const byte_t GET_DATA_LOG_FORMAT[7];

};

#include "application.hh"

struct APDU;

class Tools {

public:
static void print(char const* str, std::string const& label = "");
static void printChar(byte_t const* str, size_t size, std::string const& = "");
static void printHex(APDU const&, std::string const& = "");
static void printHex(byte_t const* str, size_t size, std::string const& = "");

};

#endif // __CCHACK_HH__
