
#include "config_gridfields.h"

#include <cstdlib>
#include "type.h"
#include "iostream"

namespace GF {


char typeformat(Type t) {
	char nty='f';
	// Added break statements. Added a default: case. jhrg 4/4/1/4
  switch (t) {
    case FLOAT:
      nty= 'f';
      break;
    case INT:
      nty= 'i';
      break;
    case OBJ:
      nty= 'p';
      break;
    default:
    	break;
  }
return nty;
};

Type typeval(std::string typestring) {Type nty=FLOAT;
  if (typestring == "f" || typestring == "F") {
    nty=FLOAT;
  }
  if (typestring == "i" || typestring == "I") {
    nty= INT;
  }
  if (typestring == "p" || typestring == "P") {
    nty= OBJ;
  }
  if (typestring == "o" || typestring == "O") {
    nty= OBJ;
  }
return nty;
}

} // namespace GF

