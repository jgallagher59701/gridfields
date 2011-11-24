#include "visualize.h"
#include "vtkGridField.h"
#include "grid.h"
#include "onegrid.h"
#include "gridfield.h"
#include "array.h"
#include "arrayreader.h"
#include "tag.h"
#include "iterate.h"
#include "state.h"
#include "apply.h"
#include "merge.h"
#include "bind.h"
#include "cross.h"
#include "sift.h"
#include "restrict.h"
#include "elcircfile.h"
#include "accumulate.h"
#include "aggregations.h"
#include "assignments.h"
#include <sstream>
#include "timing.h"
#include "connect.h"
#include "catalog.h"
#include "bind2.h"
#include "bindconstant.h"
#include "datadump.h"
#include "output.h"
#include "scaninternal.h"
                                                                                     
#include "vtkGlyph3D.h"
#include "vtkSphereSource.h"


main(int argc, char *argv[]) {

  string filename = "/home/bill/testbed/data/1_salt.63";
  string boundingbox = "(x<350000)&(x>330000)&(y>280000)&(y<310000)";
  if (argc == 3) {
    filename = string(argv[1]);
    boundingbox = string(argv[2]);
  }

  
  ElcircFile ef(filename);
  GridField *H = ef.readHGrid();
  float start = gettime();
  RestrictOp *zoom = new RestrictOp(boundingbox, H);
  zoom->getResult();
  cout << gettime() - start << ", " << zoom->getResult()->grid->getKCells(0)->getsize() << ", " << zoom->getResult()->grid->getKCells(2)->getsize() << endl;
 /* 
  vtkGridField *vtkS = toVTK(zoom->getResult(), "b");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  DirectVis(vtkS->GetOutput(), renWin);
  */
}
