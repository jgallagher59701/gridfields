#ifndef _TINDEX_H
#define _TINDEX_H 

#include <vector>
#include <set>
#include <map>
#include <ext/hash_map>
#include <iostream>

#include "cell.h"

class TIndex : public Object {
 private:
  typedef map<CellId, CellId> index;
 public:
  insert(CellId c, CellId d) { index[c] = d; }
  CellId operator[](CellId c) { return index[c]; }
};

#endif /* _TINDEX_H */
