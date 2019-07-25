#include <iomanip>
#include <iostream>

#include "headers/tools.h"

/*
  CLASS Tools
*/

void Tools::printHex(APDU const &apdu, std::string const &label) {
    printHex(apdu.data, apdu.size, label);
}

void Tools::printHex(byte_t const *str, size_t size, std::string const &label) {
    if (label.size() > 0) {
        std::cout << label << ": ";
    }

    for (size_t i = 0; i < size; ++i) {
        std::cout << HEX(str[i]) << " ";
    }
    std::cout << std::endl;
}
