#include <iostream>
#include "src/array.h"
#include "testarray.h"

using namespace std;

int main(int argc, char **argv) {
  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  Array *ca = mkTestArray("x", FLOAT, 20);  
 
  Array *foo = new Array("foo", FLOAT);
  float *data = new float[10];
  for (int i=0; i<10; i++) {
    data[i] = i+1;
  }
  foo->copyFloatData(data, 10);
  if (verbose) foo->print();
 
  Array *ccar = ca->repeat(2);
  Array *fooe = foo->expand(2);
  if (verbose) ccar->print();
  if (verbose) fooe->print();
}
