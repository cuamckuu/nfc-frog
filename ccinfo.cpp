#include <cstring>
#include <iostream>
#include <sstream>

#include "headers/ccinfo.h"
#include "headers/tools.h"

byte_t CCInfo::_FROM_SFI = 1;
byte_t CCInfo::_TO_SFI = 10; // Fast mode, 10 is max by spec
//byte_t CCInfo::_TO_SFI = 31; // Max SFI is 2^5 - 1

byte_t CCInfo::_FROM_RECORD = 1;
byte_t CCInfo::_TO_RECORD = 16; // Fast mode, usual is's enought
//byte_t CCInfo::_TO_RECORD = 255; // Full mode

CCInfo::CCInfo()
    : _pdol({0, {0}}), _track1DiscretionaryData({0, {0}}),
      _track2EquivalentData({0, {0}}), _logSFI(0), _logCount(0),
      _logFormat({0, {0}}), _logEntries({{0, {0}}})
{ }

int CCInfo::parse_response(Application const &app, APDU const &appResponse) {
    _application = app;

    byte_t const *buff = appResponse.data;
    size_t size = appResponse.size;

    for (size_t i = 0; i < size; ++i) {
        if (i + 1 < size && buff[i] == 0x5F && buff[i + 1] == 0x2D) { // Language preference
            parse_TLV(_languagePreference, buff, ++i);

        } else if (i + 1 < size && buff[i] == 0x9F && buff[i + 1] == 0x38) { // PDOL
            _pdol.size = parse_TLV(_pdol.data, buff, ++i);

        } else if (i + 1 < size && buff[i] == 0xBF && buff[i + 1] == 0x0C) { // File Control Information
            i += 2;
            byte_t len = buff[i++];

            // Extract the LOG ENTRY
            for (size_t j = 0; j < len; ++j) {
                if (j + 1 < len && buff[i + j] == 0x9F && buff[i + j + 1] == 0x4D) { // Log Entry
                    j += 2 + 1; // Size = 2 so we don't save it
                    _logSFI = buff[i + j++];
                    _logCount = buff[i + j];
                }
            }
            i += len - 1;
        } else if (buff[i] == 0x50) { // Card name
            parse_TLV(_application.name, buff, i);
        }
    }

    return 0;
}

int CCInfo::extractLogEntries(DeviceNFC &device) {
    // First we get the log format
    _logFormat = device.execute_command(Command::GET_DATA_LOG_FORMAT, sizeof(Command::GET_DATA_LOG_FORMAT), "GET DATA LOG FORMAT");

    if (_logFormat.size == 0) {
        std::cerr << "Unable to get the log format. Reading aborted." << std::endl;
        return 1;
    }

    byte_t readRecord[sizeof(Command::READ_RECORD)];
    memcpy(readRecord, Command::READ_RECORD, sizeof(readRecord));

    // Param 2: First 5 bits = SFI.
    //          Three other bits must be set to 1|0|0 (P1 is a record number)
    readRecord[5] = (_logSFI << 3) | (1 << 2);

    for (size_t i = 0; i < _logCount; ++i) {
        // Param 1: record number
        readRecord[4] = i + 1; // Starts from 1 and not 0

        _logEntries[i] = device.execute_command(readRecord, sizeof(readRecord), "READ RECORD: LOGFILE");
        if (_logEntries[i].size == 0)
            return 1;
    }

    return 0;
}

int CCInfo::read_record(DeviceNFC &device) {
    APDU readRecord = {0, {0}};
    readRecord.size = sizeof(Command::READ_RECORD);
    memcpy(readRecord.data, Command::READ_RECORD, readRecord.size);

    APDU res = {0, {0}};
    for (size_t sfi = _FROM_SFI; sfi <= _TO_SFI; ++sfi) {
        // Param 2: First 5 bits = SFI.
        //          Three other bits must be set to 1|0|0 (P1 is a record
        //          number)
        readRecord.data[5] = (sfi << 3) | (1 << 2);

        for (size_t record = _FROM_RECORD; record <= _TO_RECORD; ++record) {

            readRecord.data[4] = record; // Param 1: record number

            std::stringstream ss;
            ss << "READ RECORD from SFI" << (int)sfi << " record" << (int)record;
            res = device.execute_command(readRecord.data, readRecord.size, ss.str().c_str());

            if (res.size >= 2 && res.data[1] == 0x6A && res.data[2] == 0x82) { // File Place error
                break;
            }

            for (size_t i = 0; i < res.size; ++i) {
                if (res.data[i] == 0x57) { // Track 2 equivalent data
                    _track2EquivalentData.size = parse_TLV(_track2EquivalentData.data, res.data, i);

                } else if (i + 1 < res.size && res.data[i] == 0x5F && res.data[i + 1] == 0x20) { // Cardholder name
                    parse_TLV(_cardholderName, res.data, ++i);

                } else if (i + 1 < res.size  && res.data[i] == 0x9F && res.data[i + 1] == 0x1F) { // Track 1 discretionary data
                    _track1DiscretionaryData.size = parse_TLV(_track1DiscretionaryData.data, res.data, ++i);
                }
            }
        }

    }

    return 0;
}

void CCInfo::printPaylog() const {

    std::cout << "-----------------" << std::endl;
    std::cout << "-- Paylog --" << std::endl;
    std::cout << "-----------------" << std::endl;
    // Data are not formatted. We must read the logFormat to parse each entry
    byte_t const *format = _logFormat.data;
    size_t size = _logFormat.size;
    size_t index = 0;
    for (APDU entry : _logEntries) {
        if (entry.size == 0)
            break;

        std::cout << index++ << ": ";
        size_t e = 0;
        // Read the log format to deduce what is in the log entry
        for (size_t i = 0; i < size; ++i) {
            if (format[i] == 0x9A) { // Date
                size_t len = format[++i];
                std::cout << _logFormatTags.at(0x9A) << ": ";
                for (size_t j = 0; j < len; ++j) {
                    std::cout << (j == 0 ? "" : "/") << (j == 0 ? "20" : "")
                              << HEX(entry.data[e++]);
                }
                std::cout << "; ";
            } else if (format[i] == 0x9C) { // Type
                size_t len = format[++i];
                (void)len;
                std::cout << _logFormatTags.at(0x9C) << ": "
                          << (entry.data[e++] ? "Withdrawal" : "Payment")
                          << "; ";
            } else if (i + 1 < size) {
                if (format[i] == 0x9F && format[i + 1] == 0x21) { // Time
                    i += 2;
                    size_t len = format[i];
                    std::cout << _logFormatTags.at(0x9F21) << ": ";
                    for (size_t j = 0; j < len; ++j)
                        std::cout << (j == 0 ? "" : ":")
                                  << HEX(entry.data[e++]);
                    std::cout << "; ";
                } else if (format[i] == 0x5F && format[i + 1] == 0x2A) { // Currency
                    i += 2;
                    size_t len = format[i];
                    std::cout << _logFormatTags.at(0x5F2A) << ": ";
                    unsigned short value =
                        entry.data[e] << 8 | entry.data[e + 1];
                    // If the code is unknown, we print it. Otherwise we print
                    // the 3-char equivalent
                    if (_currencyCodes.find(value) == _currencyCodes.end()) {
                        for (size_t j = 0; j < len; ++j)
                            std::cout << HEX(entry.data[e++]);
                    } else {
                        std::cout << _currencyCodes.at(value);
                        e += 2;
                    }
                    std::cout << "; ";
                } else if (format[i] == 0x9F && format[i + 1] == 0x02) { // Amount
                    i += 2;
                    size_t len = format[i]; // Len should always be 6
                    std::cout << _logFormatTags.at(0x9F02) << ": ";
                    // First 4 bytes = value without comma
                    // 5th byte - value after the comma
                    // 6th byte = dk what it is
                    bool flagZero = true;
                    for (size_t j = 0; j < len; ++j) {
                        if (j < 4 && flagZero &&
                            entry.data[e] ==
                                0) { // We dont print zeros before the value
                            e++;
                            continue;
                        } else
                            flagZero = false;
                        std::cout << HEX(entry.data[e++]);
                        if (j == 4)
                            std::cout << ".";
                    }
                    std::cout << "; ";
                } else if (format[i] == 0x9F && format[i + 1] == 0x4E) { // Merchant
                    i += 2;
                    size_t len = format[i];
                    std::cout << _logFormatTags.at(0x9F4E) << ": ";
                    for (size_t j = 0; j < len; ++j)
                        std::cout << (char)entry.data[e++];
                    std::cout << "; ";
                } else if (format[i] == 0x9F &&
                           format[i + 1] == 0x36) { // Counter
                    i += 2;
                    size_t len = format[i];
                    std::cout << _logFormatTags.at(0x9F36) << ": ";
                    for (size_t j = 0; j < len; ++j)
                        std::cout << HEX(entry.data[e++]);
                    std::cout << "; ";
                } else if (format[i] == 0x9F &&
                           format[i + 1] == 0x1A) { // Terminal country code
                    i += 2;
                    size_t len = format[i];
                    std::cout << _logFormatTags.at(0x9F1A) << ": ";
                    unsigned short value =
                        entry.data[e] << 8 | entry.data[e + 1];
                    // If the code is unknown, we print it. Otherwise we print
                    // the 3-char equivalent
                    if (_countryCodes.find(value) == _countryCodes.end()) {
                        for (size_t j = 0; j < len; ++j)
                            std::cout << HEX(entry.data[e++]);
                    } else {
                        std::cout << _countryCodes.at(value);
                        e += 2;
                    }
                    std::cout << "; ";
                } else if (format[i] == 0x9F &&
                           format[i + 1] == 0x27) { // Crypto info data
                    i += 2;
                    size_t len = format[i];
                    std::cout << _logFormatTags.at(0x9F27) << ": ";
                    for (size_t j = 0; j < len; ++j)
                        std::cout << HEX(entry.data[e++]);
                    std::cout << "; ";
                }
            }
        }
        std::cout << std::endl;
    }
}

void CCInfo::set_full_mode() {
    _TO_SFI = 31; // Max SFI is 2^5 - 1
    _TO_RECORD = 255; // Max records is 255
}

/* The following PDOL values insert a payment in the paylog, be careful when
   using it via getProcessingOptions()
 */
const std::map<unsigned short, byte_t const *> CCInfo::PDOLValues = {
    {0x9F59, new byte_t[3]{0xC8, 0x80, 0x00}}, // Terminal Transaction Information
    {0x9F5A, new byte_t[1]{ 0x00}}, // Terminal transaction Type. 0 = payment, 1 = withdraw
    {0x9F58, new byte_t[1]{0x01}}, // Merchant Type Indicator
    {0x9F66, new byte_t[4]{0xB6, 0x20, 0xC0, 0x00}}, // Terminal Transaction Qualifiers
    {0x9F02, new byte_t[6]{0x00, 0x00, 0x10, 0x00, 0x00, 0x00}}, // amount, authorised
    {0x9F03, new byte_t[6]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}, // Amount, Other
    {0x9F1A, new byte_t[2]{0x01, 0x24}}, // Terminal country code
    {0x5F2A, new byte_t[2]{0x01, 0x24}}, // Transaction currency code
    {0x95, new byte_t[5]{0x00, 0x00, 0x00, 0x00, 0x00}},             // Terminal Verification Results
    {0x9A, new byte_t[3]{0x15, 0x01, 0x01}}, // Transaction Date
    {0x9C, new byte_t[1]{0x00}},             // Transaction Type
    {0x9F37, new byte_t[4]{0x82, 0x3D, 0xDE, 0x7A}}}; // Unpredictable number

const std::map<unsigned short, std::string> CCInfo::_logFormatTags = {
    {0x9A, "Date"},      {0x9C, "Type"},          {0x9F21, "Time"},
    {0x9F1A, "Country"}, {0x9F27, "Crypto info"}, {0x5F2A, "Currency"},
    {0x9F02, "Amount"},  {0x9F4E, "Merchant"},    {0x9F36, "Counter"}};

const std::map<unsigned short, std::string> CCInfo::_countryCodes = {
    {0x756, "CHE"},
    {0x250, "FRA"},
    {0x826, "GBR"},
    {0x124, "CAN"},
    {0x840, "USA"}};

const std::map<unsigned short, std::string> CCInfo::_currencyCodes = {
    {0x756, "CHF"},
    {0x978, "EUR"},
    {0x826, "GBP"},
    {0x124, "CAD"},
    {0x840, "USD"}};
