#include <iostream>
#include <sstream>
#include <cstring>

#include "headers/device_nfc.h"

nfc_context *DeviceNFC::context = nullptr;
std::map<unsigned short, byte_t const *> DeviceNFC::PDOLValues = {
    {0x9F59, new byte_t[3]{0xC8, 0x80, 0x00}}, // Terminal Transaction Information
    {0x9F5A, new byte_t[1]{0x00}}, // Terminal transaction Type. 0 = payment, 1 = withdraw
    {0x9F58, new byte_t[1]{0x01}}, // Merchant Type Indicator
    {0x9F66, new byte_t[4]{0x79, 0x00, 0x40, 0x80}}, // Terminal Transaction Qualifiers
    {0x9F02, new byte_t[6]{0x00, 0x00, 0x00, 0x10, 0x00, 0x00}}, // amount, authorised
    {0x9F03, new byte_t[6]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}, // Amount, Other
    {0x9F1A, new byte_t[2]{0x01, 0x24}}, // Terminal country code
    {0x5F2A, new byte_t[2]{0x01, 0x24}}, // Transaction currency code
    {0x95,   new byte_t[5]{0x00, 0x00, 0x00, 0x00, 0x00}}, // Terminal Verification Results
    {0x9A,   new byte_t[3]{0x19, 0x01, 0x01}}, // Transaction Date
    {0x9C,   new byte_t[1]{0x00}},             // Transaction Type
    {0x98,   new byte_t[20]{0}},             // Transaction Cert
    {0x9F37, new byte_t[4]{0x82, 0x3D, 0xDE, 0x7A}}, // Unpredictable number
    {0x9F66, new byte_t[4]{0xC8, 0x80, 0x00, 0x00}} // Terminal Transaction Qualifiers (TTQ) 
};


DeviceNFC::DeviceNFC() {
    nfc_init(&context);
    if (context == nullptr) {
        throw std::runtime_error("Unable to init libnfc (malloc)");
    }

    pnd = nfc_open(context, nullptr);
    if (pnd == nullptr) {
        nfc_exit(context);

        throw std::runtime_error("Unable to open NFC device");
    }

    int err = nfc_initiator_init(pnd);
    if (err < 0) {
        nfc_close(pnd);
        nfc_exit(context);

        throw std::runtime_error("Can't init card");
    }
}

bool DeviceNFC::pool_target() {
    uint8_t const uiPollNr = 1;
    uint8_t const uiPeriod = 2;

    size_t const szModulations = 1;
    nfc_modulation const nmModulations[szModulations] = {
        {NMT_ISO14443A, NBR_106}
    };

    int err = nfc_initiator_poll_target(pnd, nmModulations, szModulations, uiPollNr, uiPeriod, &nt);

    if (err > 0) {
        return true;
    }

    return false;
}

void DeviceNFC::print_target_info() {
    char *info;
    str_nfc_target(&info, &nt, true);

    std::cerr << info;
    nfc_free(info);
}

std::string DeviceNFC::get_name() {
    return std::string(nfc_device_get_name(pnd));
}

APDU DeviceNFC::execute_command(byte_t const *command, size_t size, char const *name) {
    APDU ret = {0, {0}};
    ret.size = pn53x_transceive(pnd, command, size, ret.data, sizeof(ret.data), 0);
    // Be careful, ret.data[0] == 0x00, due to libnfc, then real data comes

    if (ret.size > 3) {
        std::cerr << GREEN("[Info]" << " Response from " << name << ": ");
        for (size_t i = 1; i < ret.size; ++i) {
            if (i >= ret.size - 2) {
                // Print colored status word to cerr
                std::cerr << GREEN(HEX(ret.data[i])) << " ";
            } else {
                std::cout << HEX(ret.data[i]) << " ";
            }
        }
        std::cout << std::endl;
    }

    return ret;
}

APDU DeviceNFC::select_application(Application &app) {
    byte_t const SELECT_APP_HEADER[] = {
        0x40, 0x01, // Pn532 InDataExchange
        0x00, 0xA4, // SELECT application
        0x04, 0x00  // P1:By name, P2:First or only occurence
    };

    // Prepare the SELECT command
    byte_t command[256] = {0};
    byte_t size = sizeof(SELECT_APP_HEADER);

    // Copy command
    memcpy(command, SELECT_APP_HEADER, size);

    // Copy AID
    command[size++] = sizeof(app.aid); // Lc (Length) block
    memcpy(command + size, app.aid, sizeof(app.aid));
    size += sizeof(app.aid);

    // Increment size to have extra 0x00 in the end
    size += 1;

    APDU res = execute_command(command, size, "SELECT APP");

    // Extract Application info from SELECT response
    for (size_t i = 0; i+1 < res.size; i++) {
        if (res.data[i] == 0x9F && res.data[i+1] == 0x38) { // PDOL Tag
            app.pdol.size = parse_TLV(app.pdol.data, res.data, ++i);
        } else if (res.data[i] == 0x50) { // Application name Tag (Card name)
            parse_TLV(app.name, res.data, i);
        } else if (res.data[i] == 0xBF && res.data[i+1] == 0x0C) { // File Control Information Tag
            i += 2;
            byte_t len = res.data[i++];

            for (size_t j = 0; j+1 < len; ++j) {
                if (res.data[i+j] == 0x9F && res.data[i+j+1] == 0x4D) { // LogEnrty Tag
                    // Hard to use parse_TLV here, because we need to pass rvalue (i+j) by ref as third param
                    app.log_entry.size = 2;
                    app.log_entry.data[0] = res.data[i+j+2]; // LogEntry SFI
                    app.log_entry.data[1] = res.data[i+j+3]; // MaxRecords in LogEntry file
                }
            }
        }
    }

    return res;
}

APDU DeviceNFC::read_record(byte_t sfi, byte_t record_number) {
    byte_t const sfi_param = (sfi << 3) | (1 << 2);

    byte_t const command[] = {
        0x40, 0x01, // Pn532 InDataExchange
        0x00, 0xB2, // READ RECORD
        record_number, sfi_param, // P1:record_number and P2:SFI
        0x00 // Le
    };

    // Log string to be printed
    std::stringstream ss;
    ss << "READ RECORD from SFI" << (int)sfi << " record" << (int)record_number;

    return execute_command(command, sizeof(command), ss.str().c_str());
}

std::vector<Application> DeviceNFC::load_applications_list() {
    byte_t const command[] = {
        0x40, 0x01, // Pn532 InDataExchange
        0x00, 0xA4, // SELECT ppse
        0x04, 0x00, // P1:By name, P2:_

        0x0e, // Lc: Data length
        0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, // Data string:
        0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, // 2PAY.SYS.DDF01 (PPSE)
        0x00 // Le
    };

    APDU res = execute_command(command, sizeof(command), "SELECT PPSE");

    std::vector<Application> list;
    if (res.size == 0) {
        return list;
    }

    size_t i = 0;
    while (i < res.size - 2) {
        if (res.data[i] == 0x61) { // Application template
            byte_t app_len = res.data[++i];
            Application app;

            for (size_t j = i; j < i + app_len; j++) {
                if (res.data[j] == 0x4F) {
                    parse_TLV(app.aid, res.data, j);
                } else if (res.data[j] == 0x87) {
                    parse_TLV(&app.priority, res.data, j);
                }
            }
            list.push_back(app);
            i += app_len;
        }
        i++;
    }

    return list;
}

APDU DeviceNFC::get_data(GetDataParam param2) {
    // Don't forget to select app before usage

    byte_t const command[] = {
        0x40, 0x01, // Pn532 InDataExchange
        0x80, 0xCA, // GET DATA command
        0x9F, (byte_t)param2, // P2: Look at GetDataParam enum in header
        0x00 // Le
    };

    return execute_command(command, sizeof(command), "GET DATA");
}

APDU DeviceNFC::get_PDOL_related_data(APDU pdol) {
    APDU res = {0, {0}};

    for (size_t i = 0; i < pdol.size; i++) {
        unsigned short tag = pdol.data[i];

        if (PDOLValues.find(tag) != PDOLValues.end()) {
            // Tag with length 1 found
            byte_t len = pdol.data[++i];
            std::memcpy(res.data+res.size, PDOLValues[tag], len);
            res.size += len;
            continue;
        }

        // Tag with length 1 not found, construct new tag
        tag = tag << 8 | pdol.data[++i];

        if (PDOLValues.find(tag) != PDOLValues.end()) {
            // Tag with length 2 found
            byte_t len = pdol.data[++i];
            byte_t const *dummy_data = PDOLValues[tag];
            std::memcpy(res.data+res.size, dummy_data, len);
            res.size += len;
            continue;
        }

        // Unknown tag, fill data with zeros
        byte_t len = pdol.data[++i];
        std::memset(res.data+res.size, 0, len);
        res.size += len;
    }

    return res;
}

APDU DeviceNFC::get_processing_options(Application &app) {
    // Select application is required to get PDOL and call GPO, can't use GPO without it

    byte_t gpo_command_template[] = {
        0x40, 0x01, // PN532 Specific
        0x80, 0xA8, 0x00, 0x00, // GET PROCESSING OPTIONS
    };

    byte_t command[256] = {0};
    byte_t size = sizeof(gpo_command_template);

    std::memcpy(command, gpo_command_template, size);

    APDU data = get_PDOL_related_data(app.pdol);
    command[size++] = 0x02 + data.size;
    command[size++] = 0x83;
    command[size++] = data.size;
    std::memcpy(command+size, data.data, data.size);
    size += data.size + 1;

    return execute_command(command, size, "GET PROCESSING OPTIONS");
}

DeviceNFC::~DeviceNFC() {
    nfc_close(pnd);
    nfc_exit(context);
}
