#ifndef _CROSSCELLARRAY_H
#define _CROSSCELLARRAY_H 

#include <vector>
#include <set>
#include <map>
#include <ext/hash_map>
#include <iostream>
#include "util.h"
#include "cellarray.h"
#include "crossnodemap.h"

class CrossCellArray : public AbstractCellArray {
 public:
  
  CrossCellArray(AbstractCellArray *l, AbstractCellArray *r) : 
                      left(l), right(r), crosser(l,r) { };
  ~CrossCellArray() {};
  idx getsize() { return this->left->getsize() * this->right->getsize(); };

  Cell *getCell(int i);
  Node *getCellNodes(int i);

  idx getOrd(const Cell &c);
  idx getOrd(Node n) { return n < this->getsize() ? n : -1; }

  void getIncidentCells(Node n, set<CellId> &out) {
  //  for (unsigned int i=0; i<
  //  NOT IMPLEMENTED TODO
  }

  void buildInvertedIndex() {};

  void print(int indent);
  void print();

  void toNodeSet(set<Node> &outset);

  //CellArray *nodeFilter(vector<Node> nodes);
  CellArray *Intersection(AbstractCellArray *othercells);
  CellArray *Cross(AbstractCellArray *othercells, CrossNodeMap &h);
  void Append(AbstractCellArray *othercells);

  void mapNodes(UnaryNodeMap &h);


 private:
  AbstractCellArray *left;
  AbstractCellArray *right;
  CrossNodeMap crosser;
};

#endif /* _CELLARRAY_H */
