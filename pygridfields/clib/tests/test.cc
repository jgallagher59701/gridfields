#include "test.h"
#include <iostream>
#include <string>

using namespace std;
using namespace GF;

Agg::Agg(Big *b) {
  this->bigs.push_back(b);
}

typedef struct {
  int x;
  int empty;
} AsInt;

int main(int argc, char **argv) {
  int x = 4;
  float y = 3.2;
  void *bad = 0;
 
  *(int *) &bad = x;
  cout << x << ", " << y << ", " << (*(AsInt *) &bad).x << endl;
/*
  char foo[4] = "123";
  string s(foo);
  foo[0] = 'p';
  cout << s.c_str() << endl;
  cout << foo << endl;
*/
}
