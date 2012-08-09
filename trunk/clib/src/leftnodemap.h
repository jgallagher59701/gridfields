#ifndef _LEFTNODEMAP_H
#define _LEFTNODEMAP_H

#include "binarynodemap.h"

namespace GF {

class CrossNodeMap : public BinaryNodeMap {

 public: 
  LeftNodeMap() : BinaryNodeMap() { };
  virtual Node map(Node a, Node b) { return a; };
  Node inv_b(Node c, Node a) { assert(c==a); return a; }
  Node inv_a(Node c, Node b) { assert(c==b); return b; }
  
 private:
};

} // namespace GF

#endif /* _LEFTNODEMAP_H */
