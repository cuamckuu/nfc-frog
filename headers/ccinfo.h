#ifndef __CCINFO_HH__
#define __CCINFO_HH__

#include <map>

#include "device_nfc.h"

class CCInfo {

  public:
    CCInfo();

  public:
    int extractLogEntries(DeviceNFC &device);

    void printPaylog() const;

    int getProcessingOptions() const;

    static void set_full_mode();


  private:
    Application _application;

    // Information to print or to temporary save
    // Not real APDUS!! The structure is used to save the length
    APDU _track1DiscretionaryData; // Size unknown
    APDU _track2EquivalentData;    // Max size should never exceed 19

    APDU _logFormat;        // Format of log entries
    APDU _logEntries[0x20]; // Maximum 32 entries

  private:
    static const std::map<unsigned short, byte_t const *> PDOLValues;
    static const std::map<unsigned short, std::string> _logFormatTags;
    static const std::map<unsigned short, std::string> _currencyCodes;
    static const std::map<unsigned short, std::string> _countryCodes;

  public:
    static byte_t _FROM_SFI;
    static byte_t _TO_SFI;

    static byte_t _FROM_RECORD;
    static byte_t _TO_RECORD;
};

#endif // __CCINFO_HH__
