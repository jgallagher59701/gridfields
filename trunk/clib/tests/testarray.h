#ifndef _TESTARRAY_H
#define _TESTARRAY_H

using namespace GF;

#include "src/array.h"
Array *mkTestArray(string name, Type t, int size, int C=5) {
  Array *ca = new Array(name, t, size);
  float *data;
  ca->getData(data);
  for (int i=0; i<size; i++) {
    data[i] = i+C;
  }
  return ca;
} 

#endif
