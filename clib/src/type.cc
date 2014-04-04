
#include "config_gridfields.h"

#include <cstdlib>
#include "type.h"
#include "iostream"

namespace GF {


char typeformat(Type t) {char nty='f';
  switch (t) {
    case FLOAT:
      nty= 'f';
    case INT:
      nty= 'i';
    case OBJ:
      nty= 'p';
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

