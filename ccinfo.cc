
#include <iostream>
#include <cstring>

#include "cchack.hh"
#include "ccinfo.hh"

CCInfo::CCInfo() {
  bzero(_languagePreference, sizeof(_languagePreference));
}

int CCInfo::extractAppResponse(APDU const& appResponse) {
  
  byte_t const* buff = appResponse.data;
  size_t size = appResponse.size;

  for (size_t i = 0; i < size; ++i) {
    if (i + 1 < size &&
	buff[i] == 0x5F && buff[i + 1] == 0x2D) { // Language preference
      i += 2;
      byte_t len = buff[i++];
      memcpy(_languagePreference, &buff[i], len);
      i += len - 1;
    }
    else if (i + 1 < size &&
	     buff[i] == 0x9F && buff[i + 1] == 0x38) { // PDOL
      i += 2;
      // We just store it to parse it later
      _pdol.size = buff[i++];;
      memcpy(_pdol.data, &buff[i], _pdol.size);
      i += _pdol.size - 1;
    }
    else if (i + 1 < size &&
	     buff[i] == 0xBF && buff[i + 1] == 0x0C) { // File Control Information
      i += 2;
      byte_t len = buff[i++];;
      // Parse it now. Extract the LOG ENTRY
      for (size_t j = 0; j < len; ++j) {
	if (j + 1 < len &&
	    buff[i + j] == 0x9F && buff[i + j + 1] == 0x4D) { // Log Entry
	  j += 3; // Size = 2 so we don't save it
	  _logSFI = buff[i + j++];
	  _logCount = buff[i + j];
	}
      } // End read LOG ENTRY
      i += len - 1;
    }
    else if (buff[i] == 0x57) { // Track 2 equivalent data
      i++;
      _track2EquivalentData.size = buff[i++];
      memcpy(_track2EquivalentData.data, &buff[i], _track2EquivalentData.size);
      i += _track2EquivalentData.size - 1;      
    }
    else if (i + 1 < size &&
	     buff[i] == 0x5F && buff[i + 1] == 0x20) { // Cardholder name
      i += 2;
      byte_t len = buff[i++];
      memcpy(_cardholderName, &buff[i], len);
      i += len - 1;
    }
    else if (i + 1 < size &&
	     buff[i] == 0x9F && buff[i + 1] == 0x1F) { // Track 1 discretionary data
      i += 2;
      // We just store it to parse it later
      _pdol.size = buff[i++];;
      memcpy(_pdol.data, &buff[i], _pdol.size);
      i += _pdol.size - 1;
    }    
  }
}

void CCInfo::printAll() const {
  std::cout << "-- Print All Information --" << std::endl;
  Tools::print(_languagePreference, "Language Preference");
  Tools::print(_cardholderName, "Cardholder Name");
  Tools::printHex(_pdol, "PDOL");
  Tools::printHex(_track1DiscretionaruData, "Track 1");
  Tools::printHex(_track2EquivalentData, "Track 2");
  byte_t tmp = _logSFI << 3 | (1 << 2);
  Tools::printHex(&tmp, 1, "Query byte for LOG");
  Tools::printHex(&_logCount, 1, "Log count");
}

int CCInfo::readRecords() {

  return 0;
}

int CCInfo::getProcessingOptions() const {

  size_t pdol_response_len = 0;
  size_t size = _pdol.size;
  byte_t const* buff = _pdol.data;

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

  APDU gpo;
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
  APDU res = ApplicationHelper::executeCommand(gpo.data, gpo.size, "GPO");
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
								   {0x9F02, new byte_t[6] {0x00,0x00,0x10,0x00,0x00,0x00}},
								   {0x9F03, new byte_t[6] {0x00,0x00,0x10,0x00,0x00,0x00}},
								   {0x9F1A, new byte_t[2] {0x01,0x24}},
								   {0x5F2A, new byte_t[2] {0x01,0x24}},
								   {0x95, new byte_t[5] {0x00,0x00,0x00,0x00,0x00}},
								   {0x9A, new byte_t[3] {0x15,0x01,0x01}},
								   {0x9C, new byte_t[1] {0x00}},
								   {0x9F37, new byte_t[4] {0x82,0x3D,0xDE,0x7A}}};
