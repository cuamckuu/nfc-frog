#include <iostream>

#include "headers/device_nfc.h"

nfc_context *DeviceNFC::context = nullptr;

DeviceNFC::DeviceNFC() {
    nfc_init(&context);
    if (context == nullptr) {
        std::cerr << "Unable to init libnfc (malloc)" << std::endl;
        exit(EXIT_FAILURE);
    }

    pnd = nfc_open(context, nullptr);

    if (pnd == nullptr) {
        std::cerr << "Unable to open NFC device." << std::endl;
        nfc_exit(context);
        exit(EXIT_FAILURE);
    }

    if (nfc_initiator_init(pnd) < 0) {
        nfc_perror(pnd, "nfc_initiator_init");
        nfc_close(pnd);
        nfc_exit(context);
        exit(EXIT_FAILURE);
    }

    std::cout << "NFC reader: " << nfc_device_get_name(pnd) << " opened."
              << std::endl;
}

DeviceNFC::~DeviceNFC() {
    nfc_close(pnd);
    nfc_exit(context);
}
