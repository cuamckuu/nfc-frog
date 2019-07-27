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
        std::cerr << std::endl;
    }

    for (size_t sfi = CCInfo::_FROM_SFI; sfi <= CCInfo::_TO_SFI; ++sfi) {
        bool file_exist = false;
        for (size_t record = CCInfo::_FROM_RECORD; record <= CCInfo::_TO_RECORD; ++record) {
            APDU res = device.read_record(sfi, record);

            byte_t sw1 = res.data[res.size-2];
            byte_t sw2 = res.data[res.size-1];

            if (sw1 == 0x90 && sw2 == 0x00) {
                file_exist = true;
            } else if (sw1 == 0x6A && sw2 == 0x82) {
                break; // FileNotExist at this SFI
            }
        }
        if (file_exist) {
            std::cerr << std::endl;
        }
    }
}

void walk_through_gpo_files(DeviceNFC &device, Application &app) {
    device.select_application(app);
    APDU gpo = device.get_processing_options(app);

    // Find AFL tag to locate files on card
    size_t i = 0;
    while (i < gpo.size && gpo.data[i] != 0x94) {
        i++;
    }

    if (i >= gpo.size) {
        std::cerr << "Can't get AFL" << std::endl;
        return;
    }

    APDU afl = {0, {0}};
    afl.size = parse_TLV(afl.data, gpo.data, i);

    std::cerr << std::endl;
    for (size_t j = 0; j < afl.size; j+=4) {
        byte_t sfi = afl.data[j] >> 3;
        byte_t from_record = afl.data[j+1];
        byte_t to_record = afl.data[j+2];

        for (size_t record = from_record; record <= to_record; ++record) {
            APDU res = device.read_record(sfi, record);
        }
        std::cerr << std::endl;
    }
}

enum Mode { fast, full, GPO, UNKNOWN };

int main(int argc, char *argv[]) {
    Mode mode = Mode::UNKNOWN;

    if (argc == 1) {
        std::cerr << GREEN("[Info]") << " Use mode 'fast', 'full' or 'GPO'" << std::endl;
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
        std::cerr << RED("[Error]") << " Unknown mode" << std::endl;
        return 0;
    }

    try {
        DeviceNFC device;
        std::cerr << GREEN("[Info]") << " NFC reader: " << device.get_name() << " opened.\n";

        while (!device.pool_target()) {
            std::cerr << GREEN("[Info]") << " Searching card..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cerr << std::endl;

        std::vector<Application> list = device.load_applications_list();

        if (mode == Mode::fast || mode == Mode::full) {
            brute_device_records(device, list);
        } else if (mode == Mode::GPO) {
            for (Application &app : list) {
                walk_through_gpo_files(device, app);
            }
        }

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
