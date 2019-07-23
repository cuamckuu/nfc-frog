#include <cstring>
#include <iomanip>
#include <iostream>
#include <list>

extern "C" {
#include <nfc/nfc.h>

#ifndef PN32X_TRANSCEIVE
#define PN32X_TRANSCEIVE
int pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx,
                     const size_t szTx, uint8_t *pbtRx, const size_t szRxLen,
                     int timeout);
#endif // PN32X_TRANSCEIVE
}

#include "headers/applicationhelper.h"
#include "headers/tools.h"

byte_t ApplicationHelper::abtRx[MAX_FRAME_LEN];
int ApplicationHelper::szRx;

std::vector<Application> ApplicationHelper::getAll(nfc_device *pnd) {
    std::vector<Application> list;

    // SELECT PPSE to retrieve all applications
    APDU res = executeCommand(pnd, Command::SELECT_PPSE, sizeof(Command::SELECT_PPSE), "SELECT PPSE");
    if (res.size == 0)
        return list;

    size_t i = 0;
    while (i < res.size - 2) {
        if (res.data[i] == 0x61) { // Application template
            byte_t app_len = res.data[++i];
            Application app;

            for (size_t j = i; j < i + app_len; j++) {
                if (res.data[j] == 0x4F) {
                    parse_TLV(app.aid, res.data, j);
                } else if (res.data[j] == 0x87) {
                    parse_TLV(&app.priority, res.data, j);
                }
            }
            list.push_back(app);
            i += app_len;
        }
        i++;
    }

    return list;
}

APDU ApplicationHelper::executeCommand(nfc_device *pnd, byte_t const *command, size_t size, char const *name) {
    szRx = pn53x_transceive(pnd, command, size, abtRx, sizeof(abtRx), 0);

    if (szRx > 3) {
        Tools::printHex(abtRx + 1, szRx - 1, std::string(std::string("Answer from ") + name));
    }

    APDU ret = {0, {0}};
    ret.size = szRx - 1;
    memcpy(ret.data, abtRx + 1, szRx - 1);
    return ret;
}
