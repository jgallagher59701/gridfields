#ifndef _SUBGRIDORDMAP_H
#define _SUBGRIDORDMAP_H

#include "ordmap.h"
#include "grid.h"
#include "cellarray.h"

namespace GF {

class SubgridOrdMap : public OrdMap {

 public: 
  SubgridOrdMap(Grid *basegrid) : OrdMap() { 
    Base = basegrid;
  };

  virtual int getBaseSize(int d) {
    return Base->ordmap->getBaseSize(d);
  }
  
  virtual int getBaseOrd(Cell *c, int k) { 
    return Base->ordmap->getBaseOrd(c, k);
  };
 
 private:
  Grid *Base;
};

} // namespace GF

#endif /* _SUBGRIDORDMAP_H */
