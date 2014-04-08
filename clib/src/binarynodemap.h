#ifndef _BINARYNODEMAP_H
#define _BINARYNODEMAP_H

#include "nodemap.h"

namespace GF {

class BinaryNodeMap {

 public:

  virtual Node map(Node, Node)=0;

  //virtual ~BinaryNodeMap()=0; 
  // This class should have a destructor. jhrg 4//8/14
  virtual ~BinaryNodeMap() { }
 private:
};

} // namespace GF

#endif /* _BINARYNODEMAP_H */
