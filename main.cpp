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

    for (int i = 0; i < list.size(); i++) {
        Application &app = list[i];
        APDU res = ApplicationHelper::select_application(device.pnd, app);

        infos[i].parse_response(app, res);
        infos[i].read_record(device);
        //infos[i].extractLogEntries(device);
    }


    return infos;
}

void get_processing_options(DeviceNFC &device) {
    APDU gpo;
    byte_t select_app[256] = {0};

    byte_t size = 0;
    for (byte_t len = 0; len <= 32; len++) {
        byte_t command[] = { 0x40, 0x01, 0x80, 0xA8, 0x00, 0x00, len+2, 0x83, len};
        size = sizeof(command) + len + 1;
        memcpy(select_app, command, size);

        APDU res = device.execute_command(select_app, size, "GET PROCESSING OPTIONS");

        byte_t status[2] = {res.data[res.size-2], res.data[res.size-1]};
        if (status[0] == 0x90 && status[1] == 0x00) {
            gpo = res;
            break;
        }
    }

}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    try {
        DeviceNFC device;
        std::cout << "NFC reader: " << device.get_name() << " opened.\n";

        while (!device.pool_target()) {
            std::cout << "Searching..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::vector<Application> list = device.load_applications_list();

        std::vector<CCInfo> infos = extract_information(device, list);
        //get_processing_options(device);
/*
        for (Application &app: list) {
            std::cout << "Name: " << app.name << std::endl;
            std::cout << "Priority: " << HEX(app.priority) << std::endl;
            Tools::printHex(app.aid, sizeof(app.aid), "AID");
        }

        for (CCInfo &info: infos) {
            info.printAll();
        }
*/
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
