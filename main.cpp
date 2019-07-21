extern "C" {
    #include <nfc/nfc-types.h>
    #include <nfc/nfc.h>
}

#include <iostream>
#include <vector>

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
        infos[i].extractLogEntries(device);
    }

    return infos;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    try {
        DeviceNFC device;
        std::cout << "NFC reader: " << device.get_name() << " opened.\n";

        bool is_ok = device.pool_target();
        if (is_ok) {
        //    device.print_target_info();
        }

        std::vector<Application> list = device.load_applications_list();

        std::vector<CCInfo> infos = extract_information(device, list);
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
