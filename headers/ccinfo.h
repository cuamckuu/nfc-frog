#ifndef __CCINFO_HH__
#define __CCINFO_HH__

#include <map>

#include "device_nfc.h"

class CCInfo {

  public:
    CCInfo();

  public:
    int parse_response(Application const &, APDU const &);
    int read_record(DeviceNFC &device);
    int extractLogEntries(DeviceNFC &device);

    void printPaylog() const;

    int getProcessingOptions() const;

    static void set_full_mode();


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

    static byte_t _FROM_SFI;
    static byte_t _TO_SFI; // 10 is max by spec
    //static byte_t _TO_SFI; // Max SFI is 2^5 - 1

    static byte_t _FROM_RECORD;
    static byte_t _TO_RECORD; // Fast mode, usual is's enought
    //static byte_t _TO_RECORD; // Max records is 255
};

#endif // __CCINFO_HH__
