#include "gridfield.h"
#include "array.h"
#include "onegrid.h"
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
#include "corierecipes.h"
#include "arraywriter.h"

vtkGridField *toVTK(GridField *G, string active) {
  // create a vtk object from the gridfield
  
  // normalize the node ids to prepare for vtk
  float secs = gettime(); 
  G->grid->normalize();
//  cout << gettime() - secs << tab << "( normalize )" << endl; 
 
  secs = gettime(); 
  vtkGridField *vtkgrid = vtkGridField::New();
  vtkgrid->UseNamedPerspective();
  vtkgrid->SetActiveAttribute(active.c_str());
  vtkgrid->SetGridField(G);
  vtkgrid->Update(); 
//  vtkgrid->GetOutput()->GetPointData()->SetActiveScalars(active.c_str());
 // cout << gettime() - secs << tab << "( to VTK )" << endl; 
  return vtkgrid;
}

void computeAddresses(GridField *wetgrid) {
  // Calculate the file addresses for each data value
  float secs = gettime(); 
  ApplyOp::Apply(wetgrid, "wetpos", "addr+(zpos-b)");
  cout << gettime() - secs << tab << "( compute addresses )" << endl; 
}

GridField *makeWetSlice(GridField *H, int depth) {
  // cut away the dry portion of the grid
  float secs = gettime();
  Condition *bottom = new Condition("b", depth, "<");
  GridField *cutH = RestrictOp::Restrict(bottom, H);
  cout << gettime() - secs << tab << "( restrict bathymetry )"  << endl;
  return cutH;
}

GridField *makeWetGrid(GridField *H, GridField *V) {
  float start = gettime();
  float secs = gettime();
  // Form the 3D grid, and cut away the underground portion  
  Condition *bottom = new Condition("b", "zpos", "<");
//Condition *top = new Condition("surf", "zpos", ">");

  // Method 1: Join
//  GridField *cut = JoinOp::Join(bottom, H, V);
  

  // Method 2: Cross + Restrict
  secs = gettime(); 
  GridField *cut = CrossOp::Cross(H, V);
  cout << gettime() - secs << tab << "( cross product  )"  << endl;; 
 
  //ApplyOp::Apply(cut, "filter", "((zpos>b) | (zpos=b))  & ((zpos<surf) | (zpos=surf))");
  //Condition *top = new Condition("filter", 1.0f, ">");

  secs = gettime(); 
  cut = RefRestrictOp::Restrict(bottom, cut);
  cout << gettime() - secs << tab << "( Restrict Bathymetry )" << endl; 
  
/* 
  secs = gettime(); 
  cut = RefRestrictOp::Restrict(bottom, cut);
  cout << gettime() - secs << tab << "( river bottom )" << endl;   
*/
  cout << gettime() - start << tab << "( make WetGrid )" << endl; 
  return cut;
}

void scaleZ(GridField *Gg, float scale ) {
 // scale the z direction for easier viewing 
 // GridField must have an attribute named 'z'
  float secs = gettime();
  stringstream ss;
  ss << scale; 
  //ApplyOp::Apply(cutH, "h", ss.str()+"*(4825.1-h)");
  ApplyOp::Apply(Gg, "z", "z*"+ss.str(), FLOAT);
  cout << gettime() - secs << tab << "( Adjust h,z  )" << endl;
}

GridField *cutSurface(GridField *Gg) {
  GridField *Out;
  // cut away invalid salinity values
  float secs = gettime(); 
  Condition *saltnulls = new Condition("salt", 0, ">");
  Out = RestrictOp::Restrict(saltnulls, Gg);
  cout << gettime() - secs << tab << "( restrict air cells )" << endl; 
  return Out;
}

vtkGridField *makeBathymetry(GridField *cutH, float scale) {

  // show the river bottom for context  
 
  // scale the z direction for easier viewing 
  stringstream ss;
  ss << scale; 
  ApplyOp::Apply(cutH, "h", ss.str()+"*(4825.1-h)",FLOAT);
 
  cutH->grid->normalize();
  vtkGridField *bath = vtkGridField::New();
  bath->UseNamedPerspective();
  bath->SetGridField(cutH);
  bath->SetActiveAttribute("h");
  //if (cutH->isAttribute("z")) { bath->SetActiveAttribute("z"); }
  bath->Update();
  return bath;
}

GridField *readCleanStations(string fn, int size) {
  float tpost = gettime();
 
//  Pn->print();

  ifstream f;
  f.open(fn.c_str(), ios::in);
  if (f.fail()) {
    cerr << "Unable to open " << fn << endl;
    exit(1);
  }

  int s; 
  f >> s;
  f.close(); 
  
  Grid *P = new OneGrid("stations", s);
  GridField *Pn = new GridField(P, 0);
  
  readTextArray(Pn, 0, fn, FLOAT, 1, "x", "");
  readTextArray(Pn, 0, fn, FLOAT, s+1, "y", "");
  
  cout << gettime() - tpost << tab << "( read Stations: " << fn << " ) " << endl;
  return Pn;
}

GridField *readStations(string fn, string attr) {
  
  Scheme *sch = new Scheme();
  sch->addAttribute("id", FLOAT);
  sch->addAttribute("x", FLOAT);
  sch->addAttribute("y", FLOAT);

  Array *stations = new Array(attr.c_str(), sch);
  ArrayReader *ar = new TextArrayReader(fn, 2);
  
  ifstream f;
  f.open(fn.c_str(), ios::in);
  if (f.fail()) {
    cerr << "Unable to open " << fn << endl;
    exit(1);
  }
 
  int s;
  string gridname;
  f >> gridname >> s;
  
  Grid *P = new OneGrid(gridname, s);
  GridField *Pn = new GridField(P, 0);
  
  ar->Read(Pn, 0, stations);

  //stations->print();  
  return Pn;
}

void readTextArray(GridField *Gg, int k, string fn, Type t, 
               int offset, string attr, string addrs) {
  float secs = gettime(); 
  TextArrayReader *ar = new TextArrayReader(fn, offset, addrs);
  Array *tobind = new Array(attr.c_str(), t);
  ar->Read(Gg, k, tobind);
  Gg->Bind(tobind);
  cout << gettime() - secs << tab << "( bind " << attr << " ) " << endl;

}

void writeArray(GridField *Gg, string fn, int offset, string attr) {
  ArrayWriter *ar = new ArrayWriter(fn, offset);
  ar->Write(Gg, attr);
}

void readArray(GridField *Gg, int k, string fn, Type t, 
               int offset, string attr, string addrs) {
  // Read and Bind the salinity values using the computed addresses
  // (actually, the addresses match the ordinals here...)
  float secs = gettime(); 
  ArrayReader *ar = new ArrayReader(fn, offset, addrs);
  Array *tobind = new Array(attr.c_str(), t);
  ar->Read(Gg, k, tobind);
  Gg->Bind(tobind);
  cout << gettime() - secs << tab << "( bind " << attr << " ) " << endl;
  
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
  //cout << gettime() - secs << tab << "( compute column positions )" << endl;
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
  //xystr << "(x>" << bounds[0] << ") & (x<" << bounds[1] << ")";
  xystr << "(x>" << bounds[0] << ") & (x<" << bounds[1] << ")&";
  xystr << "(y>" << bounds[2] << ") & (y<" << bounds[3] << ")";
  cout << xystr.str() << endl;
  GridField *cutH = RestrictOp::Restrict(xystr.str(), H);
//  GridField *cutH = ApplyOp::Apply(H, "filter", xystr.str(), FLOAT);
  /*          
  Condition *xycond = new Condition("filter", 1.0f, "=");
  cutH = RefRestrictOp::Restrict(xycond,cutH);
  cout << gettime() - secs << tab << "( restrict x,y )"  << endl;;
  cout << cutH->card() << ", " << H->card() << endl;
    */          
  secs = gettime();
  stringstream zstr;
  zstr << setprecision(10);
  zstr << "(z>" << bounds[4] << ") & (z<" << bounds[5] << ")";
  GridField *cutV = ApplyOp::Apply(V, "filter", zstr.str(), FLOAT);
            
  Condition *zcond = new Condition("filter", 1.0f, "=");
  cutV = RefRestrictOp::Restrict(zcond,cutV);
  cout << gettime() - secs << tab << "( restrict z )" << endl;
  
  H = cutH;
  V = cutV;
}
