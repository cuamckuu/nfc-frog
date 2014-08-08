
#ifndef __CCINFO_HH__
# define __CCINFO_HH__

#include <map>

#include "application.hh"

class CCInfo {

public:
  /* Param is the result from SELECT APP
     Contains the Processing Data Object List (PDOL)
     The PDOL is used to perform the GET PROCESSING OPTION command which is the next step
     before performing READ RECORDS
  */
  CCInfo(Result const&);

public:
  int getProcessingOptions() const;

private:
  Result _select_app_response;
  static const std::map<unsigned short, byte_t const*> PDOLValues;
};

#endif // __CCINFO_HH__
