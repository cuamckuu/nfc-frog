
#ifndef __CCINFO_HH__
# define __CCINFO_HH__

#include <map>

#include "application.hh"

class CCInfo {

public:
  CCInfo();

public:
  int extractAppResponse(APDU const&);

  void printAll() const;

  int getProcessingOptions() const;
  int readRecords();

private:
  void _extractLogEntry(byte_t const*, size_t);

  // Information to print or to temporary save
  // Not real APDUS!! The structure is used to save the length
private:
  char _languagePreference[56]; // Handle quite a lot of languages if needed...
  char _cardholderName[56]; // Max size should never exceed 26
  APDU _pdol; // Size unknown
  APDU _track1DiscretionaruData; // Size unknown
  APDU _track2EquivalentData; // Max size should never exceed 19
  byte_t _logSFI;
  byte_t _logCount;

private:
  APDU _select_app_response;
  static const std::map<unsigned short, byte_t const*> PDOLValues;
};

#endif // __CCINFO_HH__
