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
  
#ifndef PN52X_TRANSCEIVE
# define PN52X_TRANSCEIVE
  int    pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout);
#endif // PN52X_TRANSCEIVE 

}

#include <iostream>

#include "tools.hh"
#include "applicationhelper.hh"
#include "ccinfo.hh"

struct nfc_device* pnd;

static void	init() {

  nfc_context *context;

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
}

static int startTransmission() {
  byte_t abtRx[MAX_FRAME_LEN];
  int szRx;

  if ((szRx = pn53x_transceive(pnd,
			       Command::START_14443A, sizeof(Command::START_14443A),
			       abtRx, sizeof(abtRx),
			       0)) < 0) {
    nfc_perror(pnd, "START_14443A");
    return 1;
  }
#ifdef DEBUG
  Tools::printHex(abtRx, szRx, "Answer from START_14443A");
#endif
}

static int selectAndReadApplications() {
  // Retrieve all available applications
  AppList list = ApplicationHelper::getAll();

  if (list.size() == 0) {
    std::cerr << "No application found using PPSE" << std::endl;
    return 1;
  }

  ApplicationHelper::printList(list);

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

    i++;
  }

  for (size_t i = 0; i < list.size(); ++i) {
    infos[i].printAll();
  }
  
  return 0;
}

int	main(__attribute__((unused)) int argc,
	     __attribute__((unused)) char **argv) {

  byte_t abtRx[MAX_FRAME_LEN];
  int szRx;


  init();

  while (1) {

    if (startTransmission())
      continue;

    if (selectAndReadApplications())
      ;
  }

  return 0;
}


