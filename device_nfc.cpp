#include <iostream>
#include <list>
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

    if (nfc_initiator_init(pnd) < 0) {
        nfc_close(pnd);
        nfc_exit(context);

        throw std::runtime_error(nfc_strerror(pnd));
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

    if (err < 0) {
        nfc_close(pnd);
        nfc_exit(context);

        throw std::runtime_error(nfc_strerror(pnd));
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
    return ApplicationHelper::executeCommand(pnd, command, size, name);
}

std::list<Application> DeviceNFC::load_applications_list() {
    return ApplicationHelper::getAll(pnd);
}

DeviceNFC::~DeviceNFC() {
    nfc_close(pnd);
    nfc_exit(context);
}
