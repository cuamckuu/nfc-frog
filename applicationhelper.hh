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

#ifndef __APPLICATIONHELPER_HH__
# define __APPLICATIONHELPER_HH__

#include <list>
#include <cstdio>

#include "tools.hh"

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

#endif // __APPLICATIONHELPER_HH__
