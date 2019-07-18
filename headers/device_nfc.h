#ifndef DEVICE_NFC_H
#define DEVICE_NFC_H

extern "C" {
    #include <nfc/nfc-types.h>
    #include <nfc/nfc.h>
}

class DeviceNFC {
  public:
    DeviceNFC();
    ~DeviceNFC();

  private:
    nfc_device *pnd = nullptr;
    static nfc_context *context;
};


#endif //DEVICE_NFC_H
