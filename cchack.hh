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

// Prototype to remove the warning because it is not included in nfc.h

#define MAX_FRAME_LEN 300

typedef uint8_t byte_t;

struct nfc_device* pnd;

class Command {

public:
  // Command list
  static const byte_t START_14443A[];
  static const byte_t SELECT_PPSE[];
  static byte_t READ_RECORD[];

};

const byte_t Command::START_14443A[] = {0x4A, 0x01, 0x00};
const byte_t Command::SELECT_PPSE[] = {0x40,0x01,
				       0x00,0xA4,0x04,0x00, // CLA - INS - P1 - P2
				       0x0e, // Length
				       0x32,0x50,0x41,0x59,0x2e,0x53,0x59,0x53,0x2e,0x44,0x44,0x46,0x30,0x31, // 2PAY.SYS.DDF01 (PPSE)
				       0x00};
byte_t Command::READ_RECORD[] = {0x40,0x01,
				 0x00,0xb2,0x01,0x14,0x00};

#endif // __CCHACK_HH__
