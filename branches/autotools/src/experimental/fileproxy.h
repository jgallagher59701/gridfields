#ifndef _FILEPROXY_H
#define _FILEPROXY_H

#include <string>
#include "expr.h"

using namespace std;

class Array;
class Grid;
class GridField;

class FileReader {

 public:
  FileReader(string fname);
  
  Tuple *ReadTuple(Scheme *sch);
  float *ReadFloatArray(int n);
  int *ReadIntArray(int n);
  char *ReadCharArray(int n);
  float ReadFloat();
  int ReadInt();
  char ReadChar();
  string ReadString(int n);
 
 private:
  ifstream f;
};

class FileWriter {
  
 public:
  FileWriter(string fname);
  
  void WriteTuple(Tuple *t);
  void WriteFloatArray(float *data);
  void WriteIntArray(int *data);
  void WriteCharArray(char *data);
  void WriteFloat(float f);
  void WriteInt(int i);
  void WriteChar(char c);
  
 private:
  ofstream f;
};
#endif /* _FILEPROXY_H */
