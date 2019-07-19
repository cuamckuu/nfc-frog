#ifndef __APPLICATIONHELPER_HH__
#define __APPLICATIONHELPER_HH__

#include <cstdio>
#include <list>

#include "tools.h"

typedef std::list<Application> AppList;

class ApplicationHelper {

  public:
    static bool checkTrailer();
    static AppList getAll(nfc_device *pnd);
    static void printList(AppList const &list);
    static APDU selectByPriority(nfc_device *pnd, AppList const &list, byte_t priority);
    static APDU executeCommand(nfc_device *pnd, byte_t const *command, size_t size,
                               char const *name);

  private:
    static byte_t abtRx[MAX_FRAME_LEN];
    static int szRx;
};

#endif // __APPLICATIONHELPER_HH__
