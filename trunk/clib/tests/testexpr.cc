#include <iostream>
#include "tuple.h"
#include "expr.h"

using namespace std;
using namespace GF;

int main(int argc, char **argv) {
  TupleFunction tf;
  tf.Parse("x=a+b; y=a-b");
  tf.ReturnType()->print();
  tf.InputType()->print();

  Scheme in;
  in.addAttribute("a", FLOAT);
  in.addAttribute("b", FLOAT);

  Scheme out;
  out.addAttribute("x", FLOAT);
  out.addAttribute("y", FLOAT);

  float indata[2] = {2,3};
  float outdata[2] = {0,0};
  Tuple tin(&in);
  tin.assign((char *) indata);
  tin.print();
  Tuple tout(&out);
  tout.assign((char *) outdata);
  tout.print();

  tf.Eval(tin, tout);
   
  cout << outdata[0] << ", " << outdata[1] << endl;
  tout.print();
}
