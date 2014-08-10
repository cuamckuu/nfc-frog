/*
  File edited by Alexis Guillard, Maxime Marches and Thomas Brunner for the requirements
  of our MSc Project at the University of Kent, Canterbury, UK
  
  Retrieves information available from EMV smartcards via an RFID/NFC reader.
  Both tracks are printed then track2 is parsed to retrieve PAN and expiry date.
  The paylog is parsed and showed as well.
  
  All these information are stored in plaintext on the card and available to anyone.

  License: distributed under GPL version 3 (http://www.gnu.org/licenses/gpl.html)

  Requirements:
  libnfc (>= 1.7.1) -> For later versions, please update the pn52x_transceive() prototype as it is not included in nfc.h

*/

#ifndef __APPLICATION_HH__
# define __APPLICATION_HH__

#include <list>
#include <cstdio>

#include "cchack.hh"

// For debug now
extern void show(const size_t, const byte_t*);

struct Application {
  byte_t priority;
  byte_t aid[7];
  char name[128];
};

struct APDU {
  int size;
  byte_t data[MAX_FRAME_LEN];
};

typedef std::list<Application> AppList;

class ApplicationHelper {

public:
  static bool checkTrailer();
  static AppList getAll();
  static void printList(AppList const& list);
  static APDU selectByPriority(AppList const& list, byte_t priority);
  static APDU executeCommand(byte_t const* command, size_t size, char const* name);

private:
  static byte_t abtRx[MAX_FRAME_LEN];
  static int szRx;
};


#endif // __APPLICATION_HH__
