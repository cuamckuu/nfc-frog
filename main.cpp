extern "C" {
    #include <nfc/nfc-types.h>
    #include <nfc/nfc.h>
}

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "headers/device_nfc.h"
#include "headers/applicationhelper.h"
#include "headers/ccinfo.h"
#include "headers/tools.h"

std::vector<CCInfo> extract_information(DeviceNFC &device, std::vector<Application> &list) {
    std::vector<CCInfo> infos(list.size());

    for (size_t i = 0; i < list.size(); i++) {
        Application &app = list[i];
        APDU res = ApplicationHelper::select_application(device.pnd, app);
        std::cout << std::endl;

        infos[i].parse_response(app, res);
        infos[i].read_record(device);
        //infos[i].extractLogEntries(device);
    }


    return infos;
}

void get_processing_options(DeviceNFC &device, std::vector<Application> &list) {
    // Select application is required to get PDOL and call GPO, can't use GPO without it
    for (Application &app : list) {
        ApplicationHelper::select_application(device.pnd, app);
    }

    APDU gpo = {0, {0}};
    byte_t select_app[256] = {0};

    byte_t size = 0;

    /*
    // MIR and MasterCard GPO
    for (byte_t len = 0; len <= 32; len++) {
        byte_t lc = len + 0x02;
        byte_t command[] = { 0x40, 0x01, 0x80, 0xA8, 0x00, 0x00, lc, 0x83, len};
        size = sizeof(command) + len + 1;
        memcpy(select_app, command, size);

        APDU res = device.execute_command(select_app, size, "GET PROCESSING OPTIONS");

        byte_t status[2] = {res.data[res.size-2], res.data[res.size-1]};
        if (status[0] == 0x90 && status[1] == 0x00) {
            gpo = res;
            break;
        }
    }*/


    /// Visa card GPO
    byte_t command[] = {
        0x40, 0x01, // PN532 Specific
        0x80, 0xA8, 0x00, 0x00, 0x12, // GET PROCESSING OPTIONS

        0x83, 0x10, // Data required in PDOL
        //0x79, 0x00, 0x40, 0x80, // Terminal transaction qualifier
        0b01101000, 0x00, 0x40, 0x00, // Terminal transaction qualifier
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, // Amount
        0xE0, 0x11, 0x01, 0x02, // Unpredictable number
        0x06, 0x43, // Trasaction currency code

        0x00 // Le
    };

    gpo = device.execute_command(command, sizeof(command), "GET PROCESSING OPTIONS");

    size_t i = 0;
    while (i < gpo.size && gpo.data[i] != 0x94) {
        i++;
    }

    if (i >= gpo.size) {
        std::cout << "Can't get AFL" << std::endl;
        return;
    }

    APDU afl;
    afl.size = parse_TLV(afl.data, gpo.data, i);

    std::cout << std::endl;
    Tools::printHex(afl, "AFL data");

    for (size_t j = 0; j < afl.size; j+=4) {
        byte_t sfi = afl.data[j] >> 3;
        byte_t from_sfi = afl.data[j+1];
        byte_t to_sfi = afl.data[j+2];
        std::cout << "SFI: " << HEX(sfi);
        std::cout << " FROM: " << HEX(from_sfi);
        std::cout << " TO: " << HEX(to_sfi) << std::endl;
    }

}

enum Mode { fast, full, GPO, UNKNOWN };

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    Mode mode = Mode::UNKNOWN;

    if (argc == 1) {
        std::cout << "[Info] Use mode 'fast', 'full' or 'GPO'" << std::endl;
        return 0;
    }

    std::string mode_str(argv[1]);
    if (mode_str == "fast") {
        mode = Mode::fast;
    } else if (mode_str == "full") {
        mode = Mode::full;
        CCInfo::set_full_mode();
    } else if (mode_str == "GPO") {
        mode = Mode::GPO;
    } else {
        std::cerr << "[Error] Unknown mode" << std::endl;
        return 0;
    }

    try {
        DeviceNFC device;
        std::cout << "NFC reader: " << device.get_name() << " opened.\n";

        while (!device.pool_target()) {
            std::cout << "Searching..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::vector<Application> list = device.load_applications_list();

        if (mode == Mode::fast || mode == Mode::full) {
            std::vector<CCInfo> infos = extract_information(device, list);
        } else if (mode == Mode::GPO) {
            get_processing_options(device, list);
        }

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
