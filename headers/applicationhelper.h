#ifndef __APPLICATIONHELPER_HH__
#define __APPLICATIONHELPER_HH__

#include <cstdio>
#include <vector>

#include "tools.h"

class ApplicationHelper {

  public:
    static bool checkTrailer();
    static std::vector<Application> getAll(nfc_device *pnd);
    static void printList(std::vector<Application> const &list);
    static APDU select_application(nfc_device *pnd, Application const &app);
    static APDU executeCommand(nfc_device *pnd, byte_t const *command, size_t size,
                               char const *name);

  private:
    static byte_t abtRx[MAX_FRAME_LEN];
    static int szRx;
};

#endif // __APPLICATIONHELPER_HH__
