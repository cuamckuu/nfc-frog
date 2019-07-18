extern "C" {

#include <nfc/nfc-types.h>
#include <nfc/nfc.h>

#ifndef PN52X_TRANSCEIVE
#define PN52X_TRANSCEIVE
int pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx,
                     const size_t szTx, uint8_t *pbtRx, const size_t szRxLen,
                     int timeout);
#endif // PN52X_TRANSCEIVE
}

#include <iostream>

#include "headers/applicationhelper.h"
#include "headers/ccinfo.h"
#include "headers/tools.h"

nfc_device *pnd = NULL;
static nfc_context *context;

static void init() {

    nfc_init(&context);
    if (context == NULL) {
        std::cerr << "Unable to init libnfc (malloc)" << std::endl;
        exit(EXIT_FAILURE);
    }

    pnd = nfc_open(context, NULL);

    if (pnd == NULL) {
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

static void print_nfc_target(const nfc_target *pnt) {
    char *s;
    str_nfc_target(&s, pnt, true);
    std::cout << s;
    nfc_free(s);
}

static void startTransmission() {
    const uint8_t uiPollNr = 1;
    const uint8_t uiPeriod = 2;
    const nfc_modulation nmModulations[2] = {
        {NMT_ISO14443A, NBR_106},
        {NMT_ISO14443B, NBR_106},
    };
    const size_t szModulations = 2;

    nfc_target nt;
    int res = 0;

    std::cout << "NFC device will poll during "
              << (unsigned long)uiPollNr * szModulations * uiPeriod * 150
              << " ms" << std::endl;
    if ((res = nfc_initiator_poll_target(pnd, nmModulations, szModulations,
                                         uiPollNr, uiPeriod, &nt)) < 0) {
        nfc_perror(pnd, "nfc_initiator_poll_target");
        nfc_close(pnd);
        nfc_exit(context);
        exit(EXIT_FAILURE);
    }

    if (res > 0) {
        print_nfc_target(&nt);
    } else {
        std::cout << "No target found." << std::endl;
    }
}

static int selectAndReadApplications() {
    // Retrieve all available applications
    AppList list = ApplicationHelper::getAll();

    if (list.size() == 0) {
        std::cerr << "No application found using PPSE" << std::endl;
        return 1;
    }

#ifdef DEBUG
    ApplicationHelper::printList(list);
#endif

    /* Create CCinfo object then extract all information.
     */
    CCInfo infos[list.size()];
    size_t i = 0;
    for (Application app : list) {
        APDU res = ApplicationHelper::selectByPriority(list, app.priority);
        if (res.size == 0) {
            std::cerr << "Unable to select application " << app.name
                      << std::endl;
            continue;
        }
        infos[i].extractAppResponse(app, res);

        /* Prepare PDOL, print optional interesting fields (e.g. the prefered
           language) and send the GPO THIS COMMAND ADDS AN ENTRY IN THE PAYLOG,
           BEWARE OF THIS
        */
        // if (infos[i].getProcessingOptions())
        //   ;

        infos[i].extractBaseRecords();
        infos[i].extractLogEntries();

        std::cerr << "App" << (char)('0' + app.priority) << " finished"
                  << std::endl;

        i++;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        infos[i].printAll();
    }

    return 0;
}

int main(__attribute__((unused)) int argc,
         __attribute__((unused)) char **argv) {

    init();

    while (1) {

        startTransmission();

        std::cout
            << "========================= NEW CARD ========================="
            << std::endl;
        selectAndReadApplications();

        std::cout << "========================= DONE ========================="
                  << std::endl;
    }

    return 0;
}
