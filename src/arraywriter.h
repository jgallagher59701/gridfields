#ifndef _ARRAYWRITER_H
#define _ARRAYWRITER_H

#include <string>
#include "dataset.h"


using namespace std;

class Array;
class Grid;

class ArrayWriter {

 public:
  ArrayWriter(string fn, long off, string pa);
  ArrayWriter(string fn, long off=0);
  ArrayWriter(ofstream *f);

  void Write(const Dataset &ds, string attr);
  
 private:
  string filename;
  long offset;
  ofstream *f;
  string addrAttribute;
  void prepFile();
  void setOffset(long off);
};

#endif /* _ARRAYWRITER_H */
