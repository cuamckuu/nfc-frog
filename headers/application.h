#ifndef APPLICATION_H
#define APPLICATION_H

#include "tools.h"

struct Application {
    byte_t priority = 0;
    byte_t aid[7] = {};
    char name[128] = {};
};

#endif // APPLICATION_H
