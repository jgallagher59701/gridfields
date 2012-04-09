#ifndef _UNARYNODEMAP_H
#define _UNARYNODEMAP_H

#include "nodemap.h"

class UnaryNodeMap : public NodeMap {

 public:
  virtual Node map(Node)=0;
  //virtual ~UnaryNodeMap()=0;
 private:
};

#endif /* _UNARYNODEMAP_H */
