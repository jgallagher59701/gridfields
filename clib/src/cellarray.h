#ifndef _CELLARRAY_H
#define _CELLARRAY_H 

#include "config_gridfields.h"

#include <iostream>

#include <vector>
#include <set>
#include <map>
#ifdef HAVE_UNORDERED_MAP
#include <unordered_map>
#define HASH_MAP std::unordered_map
#else
#include <ext/hash_map>
#define HASH_MAP hash_map
#endif

//TODO Switch to unordered_map? jhrg 4/16/12

#include "cell.h"
#include "abstractcellarray.h"

namespace GF {
using namespace std;

#ifndef HAVE_UNORDERED_MAP
// The new unordered_map class, the replacement for hash_map, is in
// std. jhrg 2/13/14
using namespace __gnu_cxx;
#endif

class CellArray : public AbstractCellArray {
 public:
  typedef map<Cell, int, ltCell> SortedCellIndex;
  typedef HASH_MAP<Cell, int, SimpleCellHash> InvertedCellIndex;
  // typedef hash_map<Cell, int, SimpleCellHash> InvertedCellIndex;
  // jhrg 2/13/14 
  //typedef map<Cell, int> InvertedCellIndex;
  // typedef hash_map<Node, set<CellId> > IncidenceIndex;
  // jhrg 2/13/14
  typedef HASH_MAP<Node, set<CellId> > IncidenceIndex;
  
  CellArray() : cleanup_node_array(false), 
                nodecount(0),
                node_array(NULL), 
                UseAdjacencyIndex(false)  
  { this->ref(); };
  
  CellArray(std::vector<Cell> vec) : cells(vec),
                                     cleanup_node_array(false), 
                                     node_array(NULL), 
                                     UseAdjacencyIndex(false)
  { this->ref(); };
  
  virtual int whoami() { return 1; };
  CellArray(Node *cells, int cellcount, int nodespercell);
  CellArray(Node *cells, int cellcount);
  ~CellArray();
  idx getsize();
  void addCell(Cell &c);
  void addCell(Cell *c);
  Cell *addCellNodes(Node *nodes, int size);
  vector< vector<int> > makeArrayInts();
  Cell *getCell(idx i);
  Cell getCellCopy(idx i);
  Node *getCellNodes(idx i);

  //void Edges(CellArray *onecells);

  bool contains(const Cell &c); 
  idx getOrd(const Cell &c);
  idx getOrd(Node n);
  int bytes();
  void ref();
  void unref();

  void setNodeArray(Node *na, unsigned int ns);
  void getIncidentCells(Node n, set<CellId> &out);
  void getIncidentCells(const Cell &c, set<CellId> &out);
  void getAdjacentCells(CellId c, vector<CellId> &out);
  unsigned int getNodeCount() { return this->nodecount; };
  
  void print(size_t indent);
  void print();

  void toNodeSet(set<Node> &outset);

  //CellArray *nodeFilter(vector<Node> nodes);
  CellArray *Intersection(AbstractCellArray *othercells);
  CellArray *Cross(AbstractCellArray *othercells, CrossNodeMap &h);
  void Append(AbstractCellArray *othercells);

  void mapNodes(UnaryNodeMap &h);
  CrossNodeMap makeCrossNodeMap(AbstractCellArray *other);
  void buildInvertedIndex();
  void buildIncidenceIndex();
  void buildAdjacencyIndex();

  vector<Cell> *getCellVector() { return &cells; }
  
 private:
  vector<Cell> cells;
 public:
  bool cleanup_node_array;
 private:
  unsigned int nodecount;
  Node *node_array;
  //hash_map<Cell, int, CellHash, eqCell>  hashed_cells;
  InvertedCellIndex inverted_cells;
  IncidenceIndex incidence;
  vector<vector<CellId> > adj;
  bool UseAdjacencyIndex;
};

} // namespace GF

#endif /* _CELLARRAY_H */
