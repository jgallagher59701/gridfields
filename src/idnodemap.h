#ifndef _IDNODEMAP_H
#define _IDNODEMAP_H

#include "unarynodemap.h"

class IdNodeMap : public UnaryNodeMap {

 public: 
  IdNodeMap() : UnaryNodeMap() { };
  virtual Node map(Node a) { return a };
  Node inv(Node o) { return o; }
  
 private:
};

#endif /* _IDNODEMAP_H */
