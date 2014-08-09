
#include <list>
#include <iostream>
#include <iomanip>
#include <cstring>

extern "C" {
#include <nfc/nfc.h>

#ifndef PN32X_TRANSCEIVE
# define PN32X_TRANSCEIVE
  int    pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout);
#endif // PN32X_TRANSCEIVE
}

#include "application.hh"
#include "cchack.hh"

byte_t ApplicationHelper::abtRx[MAX_FRAME_LEN];
int ApplicationHelper::szRx;

bool ApplicationHelper::checkTrailer() {
  if (szRx < 2)
    return true;

  if (abtRx[szRx - 2] == 0x90 && abtRx[szRx - 1] == 0)
    return false;

  return true;
}

AppList ApplicationHelper::getAll() {
  AppList list;

  // SELECT PPSE to retrieve all SFI
  std::cout << "Select PPSE...";  
  if ((szRx = pn53x_transceive(pnd,
			       Command::SELECT_PPSE, sizeof(Command::SELECT_PPSE),
			       abtRx, sizeof(abtRx),
			       0)) < 0 || checkTrailer()) {
    nfc_perror(pnd, "SELECT_PPSE");
    return list;
  }
  std::cout << "OK" << std::endl;
#ifdef DEBUG
  Tools::printHex(abtRx, szRx, "Answer from SELECT_PPSE");
  //  Tools::printChar(abtRx, szRx, "Answer from SELECT_PPSE");
#endif

  for (size_t i = 0; i < szRx; ++i) {
    if (abtRx[i] == 0x61) { // Application template
      Application app;
      ++i;
      while (i < szRx && abtRx[i] != 0x61) { // until the end of the buffer or the next entry

	if (abtRx[i] == 0x4F) { // Application ID
	  byte_t len = abtRx[++i];
	  i++;
	  
	  if (len != 7)
	    std::cerr << "Application id larger then 7 bytes, wtf. Continue anyway. Its gonna crash" << std::endl;
	  memcpy(app.aid, &abtRx[i], len);
	  i += len - 1;
	}

	if (abtRx[i] == 0x87) { // Application Priority indicator
	  i += 2;
	  app.priority = abtRx[i];
	}
	++i;

	if (abtRx[i] == 0x50) { // Application label
	  byte_t len = abtRx[++i];
	  i++;
	  memcpy(app.name, &abtRx[i], len);
	  app.name[len] = 0;
	  i += len - 1;
	}
	
      }
      list.push_back(app);
      --i;
    }
  }

  return list;
}

APDU ApplicationHelper::selectByPriority(AppList const& list, byte_t priority) {
  
  Application app;

  // Pick the application with the given priority
  for (Application a : list) {
    if (a.priority == priority) {
      app = a;
      break;
    }
  }
  
  // Prepare the SELECT command
  byte_t select_app[256];
  byte_t size = 0;
  bzero(select_app, 256);
  
  // Header
  memcpy(select_app, Command::SELECT_APP_HEADER, sizeof(Command::SELECT_APP_HEADER));
  size = sizeof(Command::SELECT_APP_HEADER);
  
  // Len
  select_app[size] = sizeof(app.aid);
  size += 1;
  
  // AID
  memcpy(select_app + size, app.aid, sizeof(app.aid));
  size += sizeof(app.aid);
  
  // Implicit 0x00 at the end due to bzero
  size += 1;
  
  std::cout << "Select app: " << app.name << " ...";
  // EXECUTE SELECT
  if ((szRx = pn53x_transceive(pnd,
			       select_app, size,
			       abtRx, sizeof(abtRx),
			       0)) < 0 || checkTrailer()) {
    nfc_perror(pnd, std::string(std::string("SELECT_APP_") + app.name).c_str());
    return {0, {0}};
  }
  std::cout << "OK" << std::endl;
#ifdef DEBUG
  Tools::printHex(abtRx, szRx, "Answer from SELECT_APP");
  //  Tools::printChar(abtRx, szRx, "Answer from SELECT_APP");
#endif

  APDU ret;
  ret.size = szRx - 1;
  memcpy(ret.data, abtRx+1, szRx - 1);
  return ret;
}

APDU ApplicationHelper::executeCommand(byte_t const* command, size_t size, char const* name) {
  szRx = pn53x_transceive(pnd,
			  command, size,
			  abtRx, sizeof(abtRx),
			  0);
#ifdef DEBUG
  if (szRx > 0) {
    Tools::printHex(abtRx + 1, szRx - 1, std::string(std::string("Answer from ") + name));
    //    Tools::printChar(abtRx, szRx, std::string(std::string("Answer from ") + name));
  }
#endif

  if (szRx < 0 || checkTrailer()) {
    nfc_perror(pnd, name);
    return {0, {0}};
  }
    
  APDU ret;
  ret.size = szRx - 1;
  memcpy(ret.data, abtRx + 1, szRx - 1);
  return ret;
}

void ApplicationHelper::printList(AppList const& list) {
  std::cout << list.size() << " Application(s) found:" << std::endl;

  std::cout << "-----------------" << std::endl;

  for (Application a : list) {
    std::cout << "Name: " << a.name << std::endl;
    std::cout << "Priority: " << (char)('0' + a.priority) << std::endl;
    Tools::printHex(a.aid, sizeof(a.aid), "AID");

    std::cout << std::endl << "-----------------" << std::endl;
  }
}
