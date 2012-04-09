#include "crosscellarray.h"
#include "crossnodemap.h"
#include "normnodemap.h"
#include "timing.h"
#include <assert.h>
#include <iterator>
#include <ext/algorithm> 

Cell *CrossCellArray::getCell(int x) {
  int n = this->left->getsize();
  int m = this->right->getsize();
  int i = x / n;
  int j = x % n;
  Cell *c = this->left->getCell(i);
  Cell *e = this->right->getCell(j);
  return c->Cross(e,this->crosser);
}

Node *CrossCellArray::getCellNodes(int i) {
  return this->getCell(i)->nodes;
}

int CrossCellArray::getOrd(const Cell &c) {
  // for each node, compute the inverse?
  
}

