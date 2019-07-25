extern "C" {
    #include <nfc/nfc-types.h>
    #include <nfc/nfc.h>
}

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "headers/application.h"
#include "headers/ccinfo.h"
#include "headers/device_nfc.h"
#include "headers/tools.h"

void brute_device_records(DeviceNFC &device, std::vector<Application> &list) {
    // Select application is required before READ RECORD calls
    for (size_t i = 0; i < list.size(); i++) {
        Application &app = list[i];
        APDU res = device.select_application(app);
        std::cout << std::endl;
    }

    for (size_t sfi = CCInfo::_FROM_SFI; sfi <= CCInfo::_TO_SFI; ++sfi) {
        for (size_t record = CCInfo::_FROM_RECORD; record <= CCInfo::_TO_RECORD; ++record) {
            APDU res = device.read_record(sfi, record);

            if (res.size >= 2 && res.data[1] == 0x6A && res.data[2] == 0x82) {
                break; // FileNotExist at this SFI
            }
        }
    }
}

void get_processing_options(DeviceNFC &device, Application &app) {
    // Select application is required to get PDOL and call GPO, can't use GPO without it

    byte_t gpo_command_template[] = {
        0x40, 0x01, // PN532 Specific
        0x80, 0xA8, 0x00, 0x00, // GET PROCESSING OPTIONS
    };

    byte_t command[256] = {0};
    byte_t size = sizeof(gpo_command_template);

    std::memcpy(command, gpo_command_template, size);

    APDU data = device.get_PDOL_related_data(app.pdol);
    command[size++] = 0x02 + data.size;
    command[size++] = 0x83;
    command[size++] = data.size;
    std::memcpy(command+size, data.data, data.size);
    size += data.size + 1;

    APDU gpo = device.execute_command(command, size, "GET PROCESSING OPTIONS");

    size_t i = 0;
    while (i < gpo.size && gpo.data[i] != 0x94) {
        i++;
    }

    if (i >= gpo.size) {
        std::cout << "Can't get AFL" << std::endl;
        return;
    }

    APDU afl = {0, {0}};
    afl.size = parse_TLV(afl.data, gpo.data, i);
    std::cout << "\nGPO Results: \n";

    for (size_t j = 0; j < afl.size; j+=4) {
        byte_t sfi = afl.data[j] >> 3;
        byte_t from_sfi = afl.data[j+1];
        byte_t to_sfi = afl.data[j+2];

        std::cout << "SFI " << HEX(sfi);
        std::cout << " with records from " << HEX(from_sfi);
        std::cout << " to " << HEX(to_sfi) << std::endl;
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
            brute_device_records(device, list);
        } else if (mode == Mode::GPO) {
            Application app = list[0];
            device.select_application(app);
            get_processing_options(device, app);
        }

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
