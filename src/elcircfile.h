#ifndef _ELCIRCFILE_H
#define _ELCIRCFILE_H

#include <string>
#include "expr.h"
#include <stdio.h>
extern "C" {
#include "elio.h"
}
#include "arrayreader.h"
#include "access.h"

class Array;
class GridField;

using namespace std;

class FileArrayReader;

class ElcircFile {

 public:
  ElcircFile(string fname);
  ~ElcircFile();

  string getVarScheme();
  ArrayReader *getSurfReader(int timestep, string posattr);
  ArrayReader *getVariableReader(const string &variable, 
                         int timestep, const string &posattr);
  
  int getSurfOffset(int index, int hpos=0);
  int getVariableOffset(int index, int hpos=0, int vpos=0);
  
  //int getOffset(string component);
  int getTimestepSize();
  int getHeaderSize();
  //FileArrayReader *getTimeseries()
  
  GridField *readHGrid();
  GridField *readDGrid();
  GridField *readVGrid();
  GridField *readTGrid();
  bool Valid() {return valid;}
  ElcircHeader *makeHeader(GridField *GF, ElcircHeader *h);
 
  ElcircHeader h;

  int i23d() { return h.i23d; };
 private:
  string filename;
  bool valid;
  int newid(int node, int *map, int size);
};

#endif /* _ELCIRCFILE_H */
