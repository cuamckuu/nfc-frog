
#include <iostream>
#include <cstring>

#include "cchack.hh"
#include "ccinfo.hh"

CCInfo::CCInfo(Result const& select_app_response)
  : _select_app_response(select_app_response) {
  // PDOL map init because of g++ segv on static initialisation
  //  PDOLValues[0x9F59] = new byte_t[6] {0xC0,0x80,0x00};
}

int CCInfo::getProcessingOptions() const {

  // Easier access to the buffer
  size_t size = _select_app_response.size;
  byte_t const* buff = _select_app_response.data;

  // Structure to store the PDOL itself
  Result pdol = {0, {0}};

  // Look for language preference and PDOL
  for (size_t i = 0; i < size; ++i) {
    if (i + 1 < size &&
	buff[i] == 0x5F && buff[i + 1] == 0x2D) { // Language preference
      i += 2;
      byte_t len = buff[i++];
      Tools::printChar(&buff[i], len, "Language Preference");
      i += len - 1;
    }
    if (i + 1 < size &&
	buff[i] == 0x9F && buff[i + 1] == 0x38) { // PDOL
      i += 2;
      byte_t len = buff[i++];
      // We just store it to parse it later
      pdol.size = len;
      memcpy(pdol.data, &buff[i], len);
      i += len - 1;
    }
  }

  size_t pdol_response_len = 0;
  size = pdol.size;
  buff = pdol.data;
  std::list<std::pair<unsigned short, byte_t> > tagList;
  // Browser the PDOL field
  // Retrieve tags required
  for (size_t i = 0; i < size; ++i) {
    // Offset on the first tag byte
    std::pair<unsigned short, byte_t> p = {0, 0}; // The tag
    if (buff[i] == 0x5F || buff[i] == 0x9F
	|| buff[i] == 0xBF) { // 2-byte Tag
      p.first = buff[i] << 8 | buff[i + 1];
      i++;
    } else { // 1-byte tag
      p.first = buff[i];
    }
    i++; // Go to the associated length
    p.second = buff[i];
    pdol_response_len += buff[i];
    tagList.push_back(p);
  }

  Result gpo;
  gpo.size = sizeof(Command::GPO_HEADER);
  memcpy(gpo.data, Command::GPO_HEADER, sizeof(Command::GPO_HEADER));

  gpo.data[gpo.size++] = pdol_response_len + 2; // Lc
  gpo.data[gpo.size++] = 0x83; // Tag length
  gpo.data[gpo.size++] = pdol_response_len;

  // Add tag values
  for (auto i : tagList) {
    memcpy(&gpo.data[gpo.size], PDOLValues.at(i.first), i.second);
    gpo.size += i.second;
  }
  
  gpo.data[gpo.size++] = 0; // Le

  std::cout << "Send " << pdol_response_len << "-byte GPO ...";
  // EXECUTE COMMAND
  Result res = ApplicationHelper::executeCommand(gpo.data, gpo.size, "GPO");
  if (res.size == 0) {
    std::cerr << "Error received when sending blank GPO" << std::endl;
    return 1;
  }    
  std::cout << "OK" << std::endl;
  
  return 0;
}


const std::map<unsigned short, byte_t const*>CCInfo::PDOLValues = {{0x9F59, new byte_t[3] {0xC8,0x80,0x00}},
								   {0x9F5A, new byte_t[1] {0x00}},
								   {0x9F58, new byte_t[1] {0x01}},
								   {0x9F66, new byte_t[4] {0xB6,0x20,0xC0,0x00}},
								   {0x9F02, new byte_t[6] {0x00,0x00,0x00,0x00,0x10,0x00}},
								   {0x9F03, new byte_t[6] {0x00,0x00,0x00,0x00,0x10,0x00}},
								   {0x9F1A, new byte_t[2] {0x01,0x24}},
								   {0x5F2A, new byte_t[2] {0x01,0x24}},
								   {0x95, new byte_t[5] {0x00,0x00,0x00,0x00,0x00}},
								   {0x9A, new byte_t[3] {0xE0,0x01,0x01}},
								   {0x9C, new byte_t[1] {0x00}},
								   {0x9F37, new byte_t[4] {0x82,0x3D,0xDE,0x7A}}};
