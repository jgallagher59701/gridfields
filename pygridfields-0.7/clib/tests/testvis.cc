#include "visualize.h"
#include "vtkGridField.h"
#include "gridfield.h"
#include "apply.h"
#include "array.h"
#include "timing.h"
#include "restrict.h"
#include "elcircfile.h"

void testSmallRestrict() {
   ElcircFile ef("/home/workspace/ccalmr/hindcasts/2000-01-16/run/1_salt.63");
   GridField *H = ef.readHGrid();
   Array *a = H->GetAttribute(0,"x");
   float start = gettime();
   RestrictOp *zoom = new RestrictOp("(x<350000)&(x>330000)&(y>280000)&(y<310000)", 0, H);
  zoom->getResult();
  cout << gettime() - start << endl;
  vtkGridField *vtkS = toVTK(zoom->getResult(), "h");
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  DirectVis(vtkS->GetOutput(), renWin);

}

int main() {
   cout << "test" << endl;
   testSmallRestrict();
}
