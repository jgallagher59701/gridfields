#include <iostream>
#include <string>
#include <sstream>
#include "src/array.h"
#include "src/dataset.h"
#include "src/type.h"
#include "src/expr.h"

#include "testdataset.h"

using namespace std;

int main(int argc, char **argv) {
  
  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  Dataset D(10);
  mkTestDataset(D, "x,y", 10);
  if (verbose) D.print();
  
  //Scheme s;
  //s.addAttribute("z", FLOAT);
  //D.CoerceScheme(s);

  D.Apply("z=x+y");
  if (verbose) D.PrintTo(cout, 0);

  //D.Apply("z=z+3*x*y");
  //D.PrintTo(cout, 0);
  
  //D.Apply("mask=y");
  //D.PrintTo(cout, 0);
}
