#include "gridfield.h"
#include "read63.h"
#include "arrayreader.h"
#include "refrestrict.h"
#include "cross.h"
#include "join.h"
#include <sstream>
#include "apply.h"
#include "timing.h"
#include "visualize.h"
#include "vtkGridField.h"

using namespace GF;

int main( int argc, char *argv[] ) {

  float secs;
  if (argc < 2) {
    cout << "supply a 63 file...\n";
    exit(1);
  }
  
  // read in the H and V grids
  secs = gettime();
  GridField *H = readHGrid(argv[1]);
  cout << gettime() - secs << tab << "( read H )" << endl;

  secs = gettime();
  GridField *V = readVGrid(argv[1]);
  cout << gettime() - secs << tab << "( read V )" << endl;
  
  float start = gettime();
  // record column addresses for file striding
  // (indices into V representing batymetry 
  // are provided as a gridfield over H)
  secs = gettime();
  int *b;
  H->getAttribute("b")->getData(b);
  int *pos = new int[H->card()];
  pos[0] = 0;
  
  // todo: We have 'map'; we need 'scan' and 'fold'
  for (int i=1; i<H->card(); i++) {
    pos[i] = pos[i-1] + V->card() - b[i-1]; 
  }
  Array *addr = new Array("addr", INT);
  addr->shareData(pos, H->card());
  H->Bind(addr);
 
  /*
  // Read in the indices representing the water's surface
  ArrayReader *ar1 = new ArrayReader(argv[1], 1135132 + 8);
  Array *surf = new Array("surf", INT);
  ar1->Read(H, 0, surf);
  H->Bind(surf);
  //ApplyOp::Apply(H, "surf", "surf+1");
  */
 
  // add a data column representing the order in this grid
  V->recordOrdinals("zpos");
  cout << gettime() - secs << tab << "( bind new attributes )" << endl; 
  
  // create the conditions by which to zoom, but don't apply them yet
  string region("myzoom");
  if (argc >= 3) region = argv[2];

  float *bounds = ZoomBox(region);
  
  Condition *xmin = new Condition("x", bounds[0], ">");
  Condition *xmax = new Condition("x", bounds[1], "<");
  Condition *ymin = new Condition("y", bounds[2], ">");
  Condition *ymax = new Condition("y", bounds[3], "<");
  Condition *zmax = new Condition("z", bounds[4], ">");
  Condition *zmin = new Condition("z", bounds[5], "<");

  GridField *cutH = H;
  GridField *cutV = V;
  
  // Form the 3D grid, and cut away the underground portion  
  Condition *bottom = new Condition("b", "zpos", "<");
  /*  
 // Method 1: Join
  secs = gettime(); 
  GridField *cut = JoinOp::Join(bottom, cutH, cutV);
  cout << gettime() - secs << tab << "( Join  )"  << endl;; 
  */
 
  // Method 2: Cross + Restrict
  secs = gettime(); 
  GridField *cut = CrossOp::Cross(cutH, cutV);
  cout << gettime() - secs << tab << "( cross product  )"  << endl;; 

  secs = gettime(); 
  cut = RefRestrictOp::Restrict(bottom, cut);
  cout << gettime() - secs << tab << "( river bottom )" << endl;   
 /* 
  // cut away invalid salinity values
  secs = gettime(); 
  Condition *saltnulls = new Condition("zpos", "surf", "<");
  cut = RestrictOp::Restrict(saltnulls, cut);
  cout << gettime() - secs << tab << "( restrict air cells )" << endl; 
*/
  // Calculate the file addresses for each data value
  secs = gettime(); 
  cut = ApplyOp::Apply(cut, "wetpos", "addr+(zpos-b)");
  cout << gettime() - secs << tab << "( compute addresses )" << endl; 

  
  // Read and Bind the salinity values using the computed addresses
  // (actually, the addresses match the ordinals here...)
  secs = gettime(); 
  ArrayReader *ar = new ArrayReader(argv[1], 
                                    1135132 + 8 + 29602*4, 
                                   "wetpos");
  Array *salt = new Array("salt", FLOAT);
  
  ar->Read(cut, 0, salt);
  cut->Bind(salt);
  cout << gettime() - secs << tab << "( bind salt ) " << endl;
 
 // Now zoom in to the desired grid
  secs = gettime();
  stringstream xyzstr;
  xyzstr << setprecision(10);
  xyzstr << "(x>" << bounds[0] << ") & (x<" << bounds[1] << ")&";
  xyzstr << "(y>" << bounds[2] << ") & (y<" << bounds[3] << ")&";
  xyzstr << "(z>" << bounds[4] << ") & (z<" << bounds[5] << ")";
  cut = ApplyOp::Apply(cut, "filter", xyzstr.str());
  
  Condition *xyzcond = new Condition("filter", 1.0f, "=");
  cut = RefRestrictOp::Restrict(xyzcond,cut);
  
  cout << gettime() - secs << tab << "( restrict x,y,z )"  << endl;;
  
 // scale the z direction for easier viewing 
  secs = gettime();
  float vertscale = 20;
  stringstream ss;
  ss << vertscale; 
  ApplyOp::Apply(cut, "h", ss.str()+"*(4825.1-h)");
  ApplyOp::Apply(cut, "z", "z*"+ss.str());
  cout << gettime() - secs << tab << "( Apply h,z  )" << endl;

  
/* 
  // cut away invalid salinity values
  secs = gettime(); 
  Condition *saltnulls = new Condition("salt", 1, ">");
  cut = RestrictOp::Restrict(saltnulls, cut);
  cout << gettime() - secs << tab << "( restrict air cells )" << endl; 
*/
  // normalize the node ids to prepare for vtk
  secs = gettime(); 
  cut->grid->normalize();
  cout << gettime() - secs << tab << "( normalize )" << endl; 
 
  
  // create a vtk object from the gridfield
  secs = gettime(); 
  vtkGridField *vtkgrid = vtkGridField::New();
  vtkgrid->UseNamedPerspective();
  vtkgrid->SetGridField(cut);
  vtkgrid->Update(); 
  vtkgrid->GetOutput()->GetPointData()->SetActiveScalars("salt");
  cout << gettime() - secs << tab << "( to VTK )" << endl; 

  cout << gettime() - start << tab << "( Total )" << endl; 
  // show the river bottom for context
  cutH = RefRestrictOp::Restrict(ymin,
         RefRestrictOp::Restrict(ymax,
         RefRestrictOp::Restrict(xmax,
         RefRestrictOp::Restrict(xmin,H))));
  ApplyOp::Apply(cutH, "h", ss.str()+"*(4825.1-h)");
  cutH->grid->normalize();
  vtkGridField *bath = vtkGridField::New();
  bath->UseNamedPerspective();
  bath->SetGridField(cutH);
  bath->Update();
  bath->GetOutput()->GetPointData()->SetActiveScalars("h");
 

  // call the data product visualization function
  string dataprod;
  if (argc >=4) {
    dataprod = argv[3];
  } else {
    dataprod = "";
  } 
  Visualize(vtkgrid->GetOutput(), bath->GetOutput(), dataprod);

} 
