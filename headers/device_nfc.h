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

#include <vector>

#include "application.h"
#include "tools.h"

class DeviceNFC {
  public:
    DeviceNFC();
    ~DeviceNFC();
    std::string get_name();
    void print_target_info();

  public:
    bool pool_target();
    std::vector<Application> load_applications_list();
    APDU execute_command(byte_t const *command, size_t size, char const *name);

  public: // Command Wrappers
    APDU select_application(Application &app);
    APDU read_record(byte_t sfi, byte_t record_number);

  private:
    nfc_target nt;
    nfc_device *pnd = nullptr;
    static nfc_context *context;
};


#endif //DEVICE_NFC_H
