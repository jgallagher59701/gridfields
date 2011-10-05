#include <iostream>
#include <string>
#include <sstream>
#include "array.h"
#include "dataset.h"
#include "type.h"
#include "expr.h"

#include "testdataset.h"

using namespace std;


int main(int argc, char **argv) {
  
  Dataset D(10);
  mkTestDataset(D, "x,y", 10);
  D.print();
  
  //Scheme s;
  //s.addAttribute("z", FLOAT);
  //D.CoerceScheme(s);

  D.Apply("z=x+y");
  D.PrintTo(cout, 0);

  //D.Apply("z=z+3*x*y");
  //D.PrintTo(cout, 0);
  
  //D.Apply("mask=y");
  //D.PrintTo(cout, 0);
}
