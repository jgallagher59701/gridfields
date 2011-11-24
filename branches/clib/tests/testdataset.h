#ifndef _TESTDATASET_H
#define _TESTDATASET_H

#include "testarray.h"

void mkTestDataset(Dataset &D, string sch, int size) {
  
  Scheme s(sch);
  
  for (int i=0; i<s.size(); i++) {
    D.AddAttribute(mkTestArray(s.getAttribute(i), s.getType(i), size, i));
  }
}

#endif
