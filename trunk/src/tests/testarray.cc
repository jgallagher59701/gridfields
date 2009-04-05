#include <iostream>
#include "array.h"
#include "testarray.h"

using namespace std;

int main(int argc, char **argv) {

  Array *ca = mkTestArray("x", FLOAT, 20);  
 
  Array *foo = new Array("foo", FLOAT);
  float *data = new float[10];
  for (int i=0; i<10; i++) {
    data[i] = i+1;
  }
  foo->copyFloatData(data, 10);
  foo->print();
 
  Array *ccar = ca->repeat(2);
  Array *fooe = foo->expand(2);
  ccar->print();
  fooe->print();
}
