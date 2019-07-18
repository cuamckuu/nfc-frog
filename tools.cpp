#include <iomanip>
#include <iostream>

#include "headers/tools.h"

/*
  CLASS Command
*/
const byte_t Command::START_14443A[3] = {0x4A, 0x01, 0x00};
const byte_t Command::SELECT_APP_HEADER[6] = {
    0x40, 0x01, 0x00, 0xA4, 0x04, 0x00}; // CLA - INS - P1 - P2

const byte_t Command::SELECT_PPSE[22] = {
    0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, // CLA - INS - P1 - P2
    0x0e,                               // Length
    0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59,
    0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, // 2PAY.SYS.DDF01 (PPSE)
    0x00};

const byte_t Command::GPO_HEADER[6] = {0x40, 0x01, 0x80,
                                       0xA8, 0x00, 0x00}; // CLA - INS - P1 - P2

const byte_t Command::READ_RECORD[7] = {
    0x40, 0x01, 0x00, 0xB2, // READ RECORD
    0x00, 0x00,             // count + (SFI/flags),  edited dynamically
    0x00};

const byte_t Command::GET_DATA_LOG_FORMAT[7] = {
    0x40, 0x01, 0x80, 0xCA, // GET DATA
    0x9F, 0x4F,             // 9F4F asks for the log format
    0x00};

/*
  CLASS Tools
*/

void Tools::print(char const *str, std::string const &label) {
    std::cout << label << ": " << str << std::endl;
}

void Tools::printHex(APDU const &apdu, std::string const &label) {
    printHex(apdu.data, apdu.size, label);
}

void Tools::printChar(byte_t const *str, size_t size,
                      std::string const &label) {
    if (label.size() > 0)
        std::cout << label << ": ";

    for (size_t i = 0; i < size; ++i)
        std::cout << (isprint((char)str[i]) ? (char)str[i] : '.');

    std::cout << std::endl;
}

void Tools::printHex(byte_t const *str, size_t size, std::string const &label) {
    if (label.size() > 0)
        std::cout << label << ": ";

    std::cout << std::hex << std::uppercase;
    for (size_t i = 0; i < size; ++i)
        std::cout << std::setw(2) << std::setfill('0') << (unsigned int)str[i];
    std::cout << std::dec;

    std::cout << std::endl;
}
