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

#ifndef __CCINFO_HH__
# define __CCINFO_HH__

#include <map>

#include "applicationhelper.h"

class CCInfo {

public:
  CCInfo();

public:
  int extractAppResponse(Application const&, APDU const&);
  int extractLogEntries();
  int extractBaseRecords();

  void printAll() const;
  void printPaylog() const;
  void printTracksInfo() const;

  int getProcessingOptions() const;

private:
  Application _application;
  char _languagePreference[56]; // Handle quite a lot of languages if needed...
  char _cardholderName[56]; // Max size should never exceed 26

  // Information to print or to temporary save
  // Not real APDUS!! The structure is used to save the length
  APDU _pdol; // Size unknown
  APDU _track1DiscretionaryData; // Size unknown
  APDU _track2EquivalentData; // Max size should never exceed 19

  // SFI and number of log entries
  byte_t _logSFI;
  byte_t _logCount;

  APDU _logFormat; // Format of log entries
  APDU _logEntries[0x20]; // Maximum 32 entries

private:
  APDU _select_app_response;
  static const std::map<unsigned short, byte_t const*> PDOLValues;
  static const std::map<unsigned short, std::string> _logFormatTags;
  static const std::map<unsigned short, std::string> _currencyCodes;
  static const std::map<unsigned short, std::string> _countryCodes;

  static const byte_t _FROM_SFI = 1;
  static const byte_t _TO_SFI = 2;
  static const byte_t _FROM_RECORD = 1;
  static const byte_t _TO_RECORD = 2;
};

#endif // __CCINFO_HH__
