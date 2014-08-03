/*
  File edited by Maxime Marches and Thomas Brunner for the requirements of
  our MSc Project at the University of Kent, Canterbury, UK

  License: distributed under GPL version 3 (http://www.gnu.org/licenses/gpl.html)

  Requirements:
  libnfc (>= 1.7.1)

*/

#include <list>

#include "cchack.hh"

// For debug now
extern void show(const size_t, const byte_t*);

class Application {

public:
  byte_t priority;
  byte_t aid[7];
};

typedef std::list<Application> AppList;

class ApplicationHelper {

public:
  static bool checkTrailer() {
    if (szRx < 2)
      return true;

    if (abtRx[szRx - 2] == 0x90 && abtRx[szRx - 1] == 0)
      return false;

    return true;
  }

  static AppList getAll() {
    AppList list;

    // SELECT PPSE to retrieve all SFI
    if ((szRx = pn53x_transceive(pnd,
				 Command::SELECT_PPSE, sizeof(Command::SELECT_PPSE),
				 abtRx, sizeof(abtRx),
				 0)) < 0 || checkTrailer()) {
      nfc_perror(pnd, "SELECT_PPSE");
      return list;
    }
    puts("Answer from SELECT_PPSE");
    show(szRx, abtRx);

    for (size_t i = 0; i < szRx; ++i) {
      std::cout << "i = " << i << std::endl;
      if (abtRx[i] == 0x61) { // Application template
	Application app;
	std::cout << "NEW APP" << std::endl;
	++i;
	while (i < szRx && abtRx[i] != 0x61) { // until the end of the buffer or the next entry
	  if (abtRx[i] == 0x4F) { // Application ID
	    ++i;
	    byte_t len = abtRx[i++];
	    if (len != 7)
	      puts("Application id larger then 7 bytes, wtffffffffffffffffffff");
	    memcpy(app.aid, &abtRx[i], len);
	    i += len - 1;
	  }
	  if (abtRx[i] == 87) { // Application Priority indicator
	    i += 2;
	    app.priority = abtRx[i];
	  }
	  ++i;
	}
	list.push_back(app);
	--i;
      }
    }
    return list;
  }

private:
  static byte_t abtRx[MAX_FRAME_LEN];
  static int szRx;
};

  byte_t ApplicationHelper::abtRx[MAX_FRAME_LEN];
  int ApplicationHelper::szRx;
