/*
  Copyright (C) 2014 Alexis Guillard, Maxime Marches, Thomas Brunner

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  File written for the requirements of our MSc Project at the University of Kent, Canterbury, UK

  Retrieves information available from EMV smartcards via an RFID/NFC reader.
  Both tracks are printed then track2 is parsed to retrieve PAN and expiry date.
  The paylog is parsed and showed as well.

  All these information are stored in plaintext on the card and available to anyone.

  Requirements:
  libnfc (>= 1.7.1) -> For later versions, please update the pn52x_transceive() prototype if needed, as it is not included in nfc.h

*/

extern "C" {

#include <nfc/nfc.h>
#include <nfc/nfc-types.h>

#ifndef PN52X_TRANSCEIVE
# define PN52X_TRANSCEIVE
  int    pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout);
#endif // PN52X_TRANSCEIVE

}

#include <iostream>

#include "headers/tools.h"
#include "headers/applicationhelper.h"
#include "headers/ccinfo.h"

nfc_device* pnd = NULL;
static nfc_context *context;

static void	init() {

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

  std::cout << "NFC reader: " << nfc_device_get_name(pnd) << " opened." << std::endl;
}

static void print_nfc_target(const nfc_target *pnt)
{
  char *s;
  str_nfc_target(&s, pnt, true);
  std::cout << s;
  nfc_free(s);
}

static void startTransmission() {
  const uint8_t uiPollNr = 1;
  const uint8_t uiPeriod = 2;
  const nfc_modulation nmModulations[2] = {
    { NMT_ISO14443A, NBR_106 },
    { NMT_ISO14443B, NBR_106 },
  };
  const size_t szModulations = 2;

  nfc_target nt;
  int res = 0;

  std::cout << "NFC device will poll during " << (unsigned long) uiPollNr * szModulations * uiPeriod * 150 << " ms" << std::endl;
  if ((res = nfc_initiator_poll_target(pnd, nmModulations, szModulations, uiPollNr, uiPeriod, &nt)) < 0) {
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
      std::cerr << "Unable to select application " << app.name << std::endl;
      continue;
    }
    infos[i].extractAppResponse(app, res);

    /* Prepare PDOL, print optional interesting fields (e.g. the prefered language) and send the GPO
       THIS COMMAND ADDS AN ENTRY IN THE PAYLOG, BEWARE OF THIS
    */
    // if (infos[i].getProcessingOptions())
    //   ;

    infos[i].extractBaseRecords();
    infos[i].extractLogEntries();

    std::cerr << "App" << (char) ('0' + app.priority) << " finished" << std::endl;

    i++;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    infos[i].printAll();
  }

  return 0;
}

int	main(__attribute__((unused)) int argc,
	     __attribute__((unused)) char **argv) {

  init();

  while (1) {

    startTransmission();

    std::cout << "========================= NEW CARD =========================" << std::endl;
    selectAndReadApplications();

    std::cout << "========================= DONE =========================" << std::endl;
  }

  return 0;
}
