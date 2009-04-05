#include <iostream>
#include <fstream>
#include "timing.h"
//#include "array.h"

using namespace std;

int main() {
  ifstream af("/home/bill/testbed/data/artery/artery.vtk");

  char POINTS[6];
  int np=0;
  char type[5];
  char  c;
  for (int i=0; i<4; i++) {
  while(af.good()) {
    af.get(c) ;
    if (c == '\n') {
      cout << endl;
      break;
    }
    cout << c;
  }
  }
  af >> POINTS >> np >> type;
  cout << POINTS << ", " << np << ", " << type << endl;

  //Array ax = new Array("x", FLOAT, np);
  //Array ay = new Array("x", FLOAT, np);
  //Array az = new Array("x", FLOAT, np);
  

  float s = gettime();
  float x,y,z;
  for (int i=0; i<np; i++) {
    af >> x >> y >> z;
  }
  cout << gettime() - s << endl;
}
