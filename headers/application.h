#ifndef APPLICATION_H
#define APPLICATION_H

#include "tools.h"

struct Application {
    byte_t priority = 0;
    byte_t aid[7] = {}; // Length should be from 5 up to 16
    char name[128] = {};
    APDU pdol = {0, {0}};
    APDU log_entry = {0, {0}};
};

#endif // APPLICATION_H
