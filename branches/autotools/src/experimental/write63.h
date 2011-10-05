#ifndef _WRITE63_H
#define _WRITE63_H

extern "C" {
#include "elio.h";
}

class GridField;

class CORIEWriter {

public:
  CORIEWriter(ElcircHeader &h, string fname) : fname(fname), h(h) {};

  ofstream *prepfile(string filename, int offset);
  void writeHGrid_txt(GridField *H, ofstream &f);
  void writeVGrid_txt(GridField *V, ofstream &f);
  void writeHGrid_bin(GridField *H, ofstream &f);
  void writeVGrid_bin(GridField *V, ofstream &f);
  void writeTime(int step, float stamp, ofstream &f);
  void copyHeader(string infile, ofstream &f);
  void writeHeader(ElcircHeader &h, ofstream &f); 

private:
  string fname;
  ElcircHeader h;
  ElcircTimestep t;
};

#endif /* _WRITE63_H */
