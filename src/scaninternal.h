#ifndef _SCANINTERNAL_H
#define _SCANINTERNAL_H

#include "gridfieldoperator.h"
#include "scan.h"
#include <iostream>
#include <sstream>

class ScanInternal : public ScanOp {
public:
  ScanInternal(string fn, long off);
  ScanInternal(string rawbytes);
 
  void setFileName(char *fn);
  void setRawBytes(string rawbytes);
  void Execute();
  static GridField *Scan(istream &s);

private:
  static CellArray *readCellArray(istream &f);
  static Grid *readGrid(istream &f);
  static GridField *readGridField(Grid *G, istream &f);
  static void readDataset(GridField *GF, int k, istream &f);
  static string readName(istream &f);
  std::istream *bytestream;
};

#endif
