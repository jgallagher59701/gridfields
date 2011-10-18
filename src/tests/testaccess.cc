#include <iostream>
#include <cstring>
#include "access.h"

using namespace std;

int main(int argc, char **argv) {
  
  MMapIterator fmi("/home/bill/testbed/data/1_salt.63", startpos);
  PrimitiveIterator<float> fpi(fmi);
  SliceIterator<float> fsi(fpi, startpos, h.ssize*nsteps/sizeof(int), h.ssize/sizeof(int));
  tstamp->fill(fsi);
                                                                                
  MMapIterator mi(this->filename, startpos+sizeof(float));
  PrimitiveIterator<int> pi(mi);
  SliceIterator<int> si(pi, startpos, h.ssize*nsteps/sizeof(int), h.ssize/sizeof(int));
  tstep->fill(si);
}
