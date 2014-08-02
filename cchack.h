/*
  File edited by Maxime Marches and Thomas Brunner for the requirements of
  our MSc Project at the University of Kent, Canterbury, UK

  License: distributed under GPL version 3 (http://www.gnu.org/licenses/gpl.html)

  Requirements:
  libnfc (>= 1.7.1)

  Compilation: 
$ gcc cchack.c -lnfc -o readnfccc

*/

#ifndef __CCHACK_H__
# define __CCHACK_H__

// Prototype to remove the warning because it is not included in nfc.h

#define MAX_FRAME_LEN 300

typedef uint8_t byte_t;

// Struct containing an application info
struct application {
  byte_t priority;
  
};
typedef struct application app_t;

#endif // __CCHACK_H__
