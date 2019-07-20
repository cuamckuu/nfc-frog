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

std::vector<CCInfo> extract_information(DeviceNFC &device) {
    std::vector<Application> list = device.load_applications_list();
    ApplicationHelper::printList(list);

    std::vector<CCInfo> infos(list.size());

    for (int i = 0; i < list.size(); i++) {
        Application &app = list[i];
        APDU res = ApplicationHelper::select_application(device.pnd, app);

        if (res.size == 0) {
            std::cerr << "Unable to select application " << app.name << std::endl;
            continue;
        }

        infos[i].extractAppResponse(app, res);
        infos[i].extractBaseRecords(device);
        infos[i].extractLogEntries(device);

        std::cerr << "App" << HEX(app.priority) << " finished" << std::endl;
    }


    return infos;
}

int main() {
    DeviceNFC device;
    std::cout << "NFC reader: " << device.get_name() << " opened.\n";

    bool is_ok = device.pool_target();
    if (is_ok) {
        device.print_target_info();
    }

    std::vector<CCInfo> infos = extract_information(device);

    for (CCInfo &info: infos) {
        info.printAll();
    }

    return 0;
}
