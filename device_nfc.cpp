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
        Tools::printHex(ret.data + 1, ret.size - 1, std::string(std::string("Answer from ") + name));
    }

    return ret;
}

APDU DeviceNFC::select_application(Application &app) {
    // Prepare the SELECT command
    byte_t select_app[256] = {0};
    byte_t size = sizeof(Command::SELECT_APP_HEADER);

    // SELECT by name first or only occurence
    memcpy(select_app, Command::SELECT_APP_HEADER, size);

    // Lc (Length) block
    select_app[size++] = sizeof(app.aid);

    // AID
    memcpy(select_app + size, app.aid, sizeof(app.aid));
    size += sizeof(app.aid);

    // Increment size to have extra 0x00 in the end
    size += 1;

    return execute_command(select_app, size, "SELECT APP");
}

std::vector<Application> DeviceNFC::load_applications_list() {
    return ApplicationHelper::getAll(pnd);
}

DeviceNFC::~DeviceNFC() {
    nfc_close(pnd);
    nfc_exit(context);
}
