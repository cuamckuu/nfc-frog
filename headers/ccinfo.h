#ifndef __CCINFO_HH__
#define __CCINFO_HH__

#include <map>

#include "applicationhelper.h"
#include "device_nfc.h"

class CCInfo {

  public:
    CCInfo();

  public:
    int parse_response(Application const &, APDU const &);
    int read_record(DeviceNFC &device);
    int extractLogEntries(DeviceNFC &device);

    void printAll() const;
    void printPaylog() const;
    void printTracksInfo() const;

    int getProcessingOptions() const;

  private:
    Application _application;
    char _languagePreference[56] = {0}; // Handle quite a lot of languages if needed...
    char _cardholderName[56] = {0};    // Max size should never exceed 26

    // Information to print or to temporary save
    // Not real APDUS!! The structure is used to save the length
    APDU _pdol;                    // Size unknown
    APDU _track1DiscretionaryData; // Size unknown
    APDU _track2EquivalentData;    // Max size should never exceed 19

    // SFI and number of log entries
    byte_t _logSFI;
    byte_t _logCount;

    APDU _logFormat;        // Format of log entries
    APDU _logEntries[0x20]; // Maximum 32 entries

  private:
    APDU _select_app_response;
    static const std::map<unsigned short, byte_t const *> PDOLValues;
    static const std::map<unsigned short, std::string> _logFormatTags;
    static const std::map<unsigned short, std::string> _currencyCodes;
    static const std::map<unsigned short, std::string> _countryCodes;

    static const byte_t _FROM_SFI = 1;
    static const byte_t _TO_SFI = 31; // Max SFI is 2^5 - 1
    static const byte_t _FROM_RECORD = 1;
    static const byte_t _TO_RECORD = 8;
    //static const byte_t _TO_RECORD = 255; // Max records is 255 but it's faster to check only 10
};

#endif // __CCINFO_HH__
