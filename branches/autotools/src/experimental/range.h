#ifndef _RANGE_H
#define _RANGE_H

#include "array.h"

class Range : Array {

 public:
  Range(const char *nm, int size_) : Array(nm, INT) {
    int *vals = new int[size_];
    for (int i=0; i<size_; i++) {
      vals[i] = i;
    }
    this->shareData(vals, size_);
    size = size_;
  };
};

#endif /* _RANGE_H */
