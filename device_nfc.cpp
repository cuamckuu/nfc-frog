#include <iostream>
#include <sstream>
#include <cstring>

#include "headers/device_nfc.h"

nfc_context *DeviceNFC::context = nullptr;

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

    std::cout << info;
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
        std::cout << "Answer from " << name << ": ";
        for (size_t i = 0; i < ret.size; ++i) {
            std::cout << HEX(ret.data[i]) << " ";
        }
        std::cout << std::endl;
    }

    return ret;
}

APDU DeviceNFC::select_application(Application &app) {
    byte_t const SELECT_APP_HEADER[] = {
        0x40, 0x01, // Pn532 InDataExchange
        0x00, 0xA4, // SELECT application
        0x04, 0x00 // P1:By name, P2:_
    };

    // Prepare the SELECT command
    byte_t command[256] = {0};
    byte_t size = sizeof(SELECT_APP_HEADER);

    // SELECT by name first or only occurence
    memcpy(command, SELECT_APP_HEADER, size);

    // Lc (Length) block
    command[size++] = sizeof(app.aid);

    // AID
    memcpy(command + size, app.aid, sizeof(app.aid));
    size += sizeof(app.aid);

    // Increment size to have extra 0x00 in the end
    size += 1;

    return execute_command(command, size, "SELECT APP");
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

DeviceNFC::~DeviceNFC() {
    nfc_close(pnd);
    nfc_exit(context);
}
