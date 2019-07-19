#ifndef DEVICE_NFC_H
#define DEVICE_NFC_H

extern "C" {
    #include <nfc/nfc-types.h>
    #include <nfc/nfc.h>

    #ifndef PN32X_TRANSCEIVE
    #define PN32X_TRANSCEIVE
    int pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx,
                         const size_t szTx, uint8_t *pbtRx, const size_t szRxLen,
                         int timeout);
    #endif // PN32X_TRANSCEIVE
}

#include <list>

#include "tools.h"
#include "applicationhelper.h"

class DeviceNFC {
  public:
    DeviceNFC();
    bool pool_target();
    std::string get_name();
    void print_target_info();

    ~DeviceNFC();

    std::list<Application> load_applications_list();
    APDU execute_command(byte_t const *command, size_t size, char const *name);

  private:
    byte_t abtRx[MAX_FRAME_LEN] = {};
    int szRx = 0;

  public:
    nfc_target nt;
    nfc_device *pnd = nullptr;
    static nfc_context *context;
};


#endif //DEVICE_NFC_H
