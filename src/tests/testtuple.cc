#include <iostream>
#include "tuple.h"
#include "expr.h"

int main(int argc, char **argv) {

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

  tup.print();

  TupleFunction tf;
  tf.Parse("z=x+b");
  Tuple tout(tf.ReturnType());
  char *block = tout.Allocate();
  tf.Eval(tup, tout);
  tout.print();
}

