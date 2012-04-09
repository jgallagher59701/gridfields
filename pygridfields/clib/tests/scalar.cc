#include "gridfield.h"
#include "array.h"
#include "read63.h"
#include "arrayreader.h"
#include "refrestrict.h"
#include "cross.h"
#include "join.h"
#include <sstream>
#include <string>
#include "apply.h"
#include "timing.h"
#include "visualize.h"
#include "vtkGridField.h"

void computeColumnPositions(GridField *H,GridField*V); 
int scalar3D( GridField *H, GridField *V, const char *filename, 
              int addr, string region, string dataprod);
void HorizontalSlice(GridField *H, GridField *V, const char *filename, 
                    int addr, string region);
vtkGridField *makeBathymetry(GridField *cutH) ;
void Zoom(GridField *&H, GridField *&V, string _region);


int main( int argc, char *argv[] ) {
  
  float start = gettime();
  float secs;

  string dataprod, region;
  
  if (argc < 2) {
    cout << "supply a 63 file...\n";
    exit(1);
  }
 
 // which region to zoom to
  if (argc < 3) {
    region = "myzoom";
  } else {
    region = argv[2];
  }
  
  //which product to produce
  if (argc < 4) {
    dataprod = "";
  } else {
    dataprod = argv[3];
  }
 
  // read in the H and V grids
  secs = gettime();
  GridField *H = readHGrid(argv[1]);
  cout << gettime() - secs << tab << "( read H )" << endl;

  secs = gettime();
  GridField *V = readVGrid(argv[1]);
  cout << gettime() - secs << tab << "( read V )" << endl;
   
 
  scalar3D(H, V, argv[1], 1135132 + 8 + 29602*4, region, dataprod); 
// HorizontalSlice(H,V, argv[1], 1135132 + 8 + 29602*4, region); 
}

int scalar3D( GridField *H, GridField *V, const char *filename, 
              int addr, string region, string dataprod ) {

  float start = gettime();
  float secs;
  
  computeColumnPositions(H,V);
  
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
  
  GridField *cutH = H;
  GridField *cutV = V;

  Zoom(cutH,cutV,region);
 
  
 // scale the z direction for easier viewing 
  secs = gettime();
  float vertscale = 20;
  stringstream ss;
  ss << vertscale; 
  //ApplyOp::Apply(cutH, "h", ss.str()+"*(4825.1-h)");
  ApplyOp::Apply(cutV, "z", "z*"+ss.str());
  cout << gettime() - secs << tab << "( Apply h,z  )" << endl;

 
  // Form the 3D grid, and cut away the underground portion  
  Condition *bottom = new Condition("b", "zpos", "<");

  // Method 1: Join
  secs = gettime(); 
  GridField *cut = JoinOp::Join(bottom, cutH, cutV);
  cout << gettime() - secs << tab << "( Join  )"  << endl;; 
/*
  // Method 2: Cross + Restrict
  secs = gettime(); 
  GridField *cut = CrossOp::Cross(cutH, cutV);
  cout << gettime() - secs << tab << "( cross product  )"  << endl;; 

  secs = gettime(); 
  cut = RefRestrictOp::Restrict(bottom, cut);
  cout << gettime() - secs << tab << "( river bottom )" << endl;   
*/
  
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
  ArrayReader *ar = new ArrayReader(filename, addr, "wetpos");
  Array *salt = new Array("salt", FLOAT);
  ar->Read(cut, 0, salt);
  cut->Bind(salt);
  cout << gettime() - secs << tab << "( bind salt ) " << endl;
 
  
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
  cout << "Selectivity: " << cut->card() << ", " << (829852) << endl;
  vtkGridField *bath = makeBathymetry(cutH);
 
  // call the data product visualization function
  Visualize(vtkgrid->GetOutput(), bath->GetOutput(), dataprod);
}

vtkGridField *makeBathymetry(GridField *cutH) {

  // show the river bottom for context  
 
  // scale the z direction for easier viewing 
  stringstream ss;
  ss << 20; 
  ApplyOp::Apply(cutH, "h", ss.str()+"*(4825.1-h)");
 
  cutH->grid->normalize();
  vtkGridField *bath = vtkGridField::New();
  bath->UseNamedPerspective();
  bath->SetGridField(cutH);
  bath->SetActiveAttribute("h");
  bath->Update();
  return bath;
}

void HorizontalSlice(GridField *H, GridField *V, const char *filename, 
                     int addr, string region) {

  // find V-index from z value here
  float secs;
  int v = 52;

  computeColumnPositions(H,V);
  
  Condition *p = new Condition("b", v, "<");
  GridField *cutH = RestrictOp::Restrict(p, H);

  stringstream expr;
  expr << "addr + " << v << " - b";
  ApplyOp::Apply(cutH, "sliceaddr", expr.str());
  
  Array *salt = new Array("salt", FLOAT);
  ArrayReader *ar = new ArrayReader(filename, 
                                    addr, 
                                    "sliceaddr");
  ar->Read(cutH, 0, salt);
 
  ApplyOp::Apply(cutH, "z", "20*4825.1");
  cutH->Bind(salt); 
  
  cutH->grid->normalize();
  // create a vtk object from the gridfield
  secs = gettime(); 
  vtkGridField *vtkgrid = vtkGridField::New();
  vtkgrid->UseNamedPerspective();
  vtkgrid->SetGridField(cutH);
  vtkgrid->SetActiveAttribute("salt");
  vtkgrid->Update(); 
  cout << gettime() - secs << tab << "( to VTK )" << endl; 
    
  vtkGridField *bath = makeBathymetry(H);
  DirectVis(vtkgrid->GetOutput(), bath->GetOutput());
}

void computeColumnPositions(GridField *H, GridField *V) {

  // record column addresses for file striding
  // (indices into V representing batymetry 
  // are provided as a gridfield over H)
  float secs = gettime();
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
  cout << gettime() - secs << tab << "( compute column positions )" << endl;
}

void Zoom(GridField *&H, GridField *&V, string _region) {
  float secs;
  // create the conditions by which to zoom
  
//  GridField *H = *pH;
//  GridField *V = *pV;
  string region("estuary");
  if (_region != "") region = _region;

  float *bounds = ZoomBox(region);
/*
  for (int i=0; i<6; i++) {
    cout << bounds[i] << ", ";
  }
  cout << endl;
*/

 /* 
  Condition *xmin = new Condition("x", bounds[0], ">");
  Condition *xmax = new Condition("x", bounds[1], "<");
  Condition *ymin = new Condition("y", bounds[2], ">");
  Condition *ymax = new Condition("y", bounds[3], "<");
  Condition *zmax = new Condition("z", bounds[4], ">");
  Condition *zmin = new Condition("z", bounds[5], "<");
 

 // Now zoom in to the desired grid
  secs = gettime();
  GridField *cutH = RefRestrictOp::Restrict(ymin,
                    RefRestrictOp::Restrict(ymax,
                    RefRestrictOp::Restrict(xmax,
                    RefRestrictOp::Restrict(xmin,H))));
  cout << gettime() - secs << tab << "( restrict x,y )"  << endl;;
  
  secs = gettime();
  GridField *cutV = RefRestrictOp::Restrict(zmax,
                    RefRestrictOp::Restrict(zmin, V));
  cout << gettime() - secs << tab << "( restrict z )" << endl;
  */

  secs = gettime();
  stringstream xystr;
  xystr << setprecision(10);
  xystr << "(x>" << bounds[0] << ") & (x<" << bounds[1] << ")&";
  xystr << "(y>" << bounds[2] << ") & (y<" << bounds[3] << ")";
  GridField *cutH = ApplyOp::Apply(H, "filter", xystr.str());
            
  Condition *xycond = new Condition("filter", 1.0f, "=");
  cutH = RefRestrictOp::Restrict(xycond,cutH);
  cout << gettime() - secs << tab << "( restrict x,y )"  << endl;;
              
  secs = gettime();
  stringstream zstr;
  zstr << setprecision(10);
  zstr << "(z>" << bounds[4] << ") & (z<" << bounds[5] << ")";
  GridField *cutV = ApplyOp::Apply(V, "filter", zstr.str());
            
  Condition *zcond = new Condition("filter", 1.0f, "=");
  cutV = RefRestrictOp::Restrict(zcond,cutV);
  cout << gettime() - secs << tab << "( restrict z )" << endl;
  
  H = cutH;
  V = cutV;
}
