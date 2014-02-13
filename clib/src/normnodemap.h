#ifndef _NORMNODEMAP_H
#define _NORMNODEMAP_H

#ifdef HAVE_UNORDERED_MAP
#include <unordered_map>
#define HASH_MAP std::unordered_map
#else
#include <ext/hash_map>
#define HASH_MAP hash_map
#endif

#include "unarynodemap.h"
#include "cellarray.h"
#include "assert.h"

namespace GF {

class NormNodeMap : public UnaryNodeMap {

 public: 
  NormNodeMap(AbstractCellArray *zcs) : UnaryNodeMap() { 
    zerocells = zcs;
    Cell *c;
    int x;
    // jhrg 2/13/14
#ifdef HAVE_UNORDERED_MAP
    nodemap.rehash(zerocells->getsize());
#else
    nodemap.resize(zerocells->getsize());
#endif
    for (unsigned int i=0; i<zerocells->getsize(); i++) {
      c = zerocells->getCell(i);
      x = c->getnodes()[0];
      nodemap[x] = i;
    }
  };
  virtual Node map(Node x) {
    return nodemap[x];
  };
  
  Node inv(Node o) { 
    Cell *c;
    c = zerocells->getCell(o);
    assert(c->getsize() == 1);
    return c->getnodes()[0];
  };
  
 private:
  HASH_MAP<int, int> nodemap;
  //std::map<int, int> nodemap;
  AbstractCellArray *zerocells;
};

} // namespace GF

#endif /* _CROSSNODEMAP_H */
