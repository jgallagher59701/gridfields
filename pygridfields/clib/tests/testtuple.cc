#include <iostream>
#include "src/tuple.h"
#include "src/expr.h"

using namespace GF;

int main(int argc, char **argv) {

  bool verbose = false;
  // replace this with getopt? jhrg 9/30/11
  if (argc == 2 && strncmp(argv[1], "-v", 2) == 0)
    verbose = true;

  Scheme sch;
  sch.addAttribute("x", FLOAT);
  sch.addAttribute("y", FLOAT);
  sch.addAttribute("h", FLOAT);
  sch.addAttribute("b", INT);

  //sch.print();
  
  Tuple tup(&sch);
  float x = 1.4;
  float y = 1.2;
  float h = 45.6;
  int b = 3;
  
  tup.set("x", &x);
  tup.set("y", &y);
  tup.set("h", &h);
  tup.set("b", &b);

  if (verbose) tup.print();

  TupleFunction tf;
  tf.Parse("z=x+b");
  Tuple tout(tf.ReturnType());
  char *block = tout.Allocate();
  tf.Eval(tup, tout);
  if (verbose) tout.print();
}

