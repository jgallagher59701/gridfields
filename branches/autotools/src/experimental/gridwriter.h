#ifndef _GRIDWRITER_H
#define _GRIDWRITER_H

#include <string>

using namespace std;

class Grid;

class GridWriter {

 private:
  string filename;
  long offset;
 public:
  GridWriter(string fn, long off);

  void Write(Grid *G);

};

#endif /* _GRIDWRITER_H */
