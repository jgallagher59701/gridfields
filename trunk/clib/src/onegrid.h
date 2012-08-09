#ifndef _ONEGRID_H
#define _ONEGRID_H

#include "grid.h"

/*
 * A Specialized constructor for one dimensional simplex grids
 *
 */

namespace GF {

class OneGrid : public Grid {

 public:

  OneGrid(string name, int size) : Grid(name) {
    setImplicit0Cells(size);
    CellArray *onecells = new CellArray;
    Node nodes[2];
    for (int i=1; i<size; i++) {
      nodes[0] = i-1;
      nodes[1] = i;
      onecells->addCellNodes(nodes, 2);
    }
    setKCells(onecells, 1);
    this->ref();
  };

};


class ZeroGrid : public Grid {
  public:
    ZeroGrid(string name, unsigned int size) : Grid(name, 0) {
      setImplicit0Cells(size);
      this->ref();
    };
};

} // namespace GF

#endif /*  _ONEGRID_H   */
