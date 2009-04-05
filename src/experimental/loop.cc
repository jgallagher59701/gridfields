#include "gridfield.h"
#include "array.h"
#include "range.h"
#include "onegrid.h"
#include "read63.h"
#include "arrayreader.h"
#include "arraywriter.h"
#include "assignments.h"
#include "aggregations.h"
#include "refrestrict.h"
#include "cross.h"
#include "merge.h"
#include "join.h"
#include <sstream>
#include "apply.h"
#include "timing.h"
#include "visualize.h"
#include "vtkGridField.h"

int main( int argc, char *argv[] ) {

  float vertscale = 20;
  stringstream ss;
  ss << vertscale; 
  
  float start = gettime();
  float secs;
  if (argc < 2) {
    cout << "supply a 63 file...\n";
    exit(1);
  }

  secs = gettime();
  GridField *H = readHGrid(argv[1]);
  cout << gettime() - secs << tab << "( read H )" << endl;

  secs = gettime();
  GridField *V = readVGrid(argv[1]);
  cout << gettime() - secs << tab << "( read V )" << endl;
  
  secs = gettime();
  
  int *b;
  H->getAttribute("b")->getData(b);
  int *pos = new int[H->card()];
  pos[0] = 0;
  for (int i=1; i<H->card(); i++) {
    //cout << b[i-1];
    //getchar();
    pos[i] = pos[i-1] + V->card() - b[i-1]; 
  }

  int totalsize = pos[H->card()-1] + V->card() - b[H->card()-1];
  cout << "totalsize:" << totalsize << endl;;
  
  Array *addr = new Array("addr", INT);
  addr->shareData(pos, H->card());
  H->Bind(addr);
  
  H->recordOrdinals("hpos");
  V->recordOrdinals("zpos");
  cout << gettime() - secs << tab << "( bind new attributes )" << endl;

  
  // create the conditions by which to zoom
  //float *bounds = ZoomBox("testgrid");
  string region("estuary");
  if (argc >= 3) region = argv[2];
  
  float *bounds = ZoomBox(region);

  Condition *xmin = new Condition("x", bounds[0], ">");
  Condition *xmax = new Condition("x", bounds[1], "<");
  Condition *ymin = new Condition("y", bounds[2], ">");
  Condition *ymax = new Condition("y", bounds[3], "<");
  Condition *zmax = new Condition("z", bounds[4], ">");
  Condition *zmin = new Condition("z", bounds[5], "<");
  
  //zoom 
  secs = gettime();
  GridField *cutH = RefRestrictOp::Restrict(ymin,
                    RefRestrictOp::Restrict(ymax,
                    RefRestrictOp::Restrict(xmax,
                    RefRestrictOp::Restrict(xmin,H))));
  cout << gettime() - secs << tab << "( restrict H )"  << endl;;
  
  secs = gettime();
  GridField *cutV = RefRestrictOp::Restrict(zmax,
                    RefRestrictOp::Restrict(zmin, V));
  cout << gettime() - secs << tab << "( restrict V )" << endl;

  cutH->grid->normalize();
  cutV->grid->normalize();
  
  secs = gettime();
  ApplyOp::Apply(cutH, "h", ss.str()+"*(4825.1-h)");
  ApplyOp::Apply(cutV, "z", "z*"+ss.str());
  cout << gettime() - secs << tab << "( Apply h,z  )" << endl;

  Condition *bottom = new Condition("b", "zpos", "<");
  
 // Method 1: Join
  secs = gettime(); 
  GridField *cut = JoinOp::Join(bottom, cutH, cutV);
  cout << gettime() - secs << tab << "( Join  )"  << endl; 
  
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
  ArrayReader *ar1 = new ArrayReader(argv[1], 1135132 + 8);
  Array *surf = new Array("surf", INT);
  ar1->Read(H->grid, 0, surf);

  surf->print();
*/
  
  secs = gettime(); 
  cut = ApplyOp::Apply(cut, "wetpos", "addr+(zpos-b)");
  cout << gettime() - secs << tab << "( compute addresses )" << endl; 

  
  // compute the ordinals for the transposed grid.
  map<int, map<int, int> > sorted;
  int h, v; 
  for (int i=0; i< cut->card(); i++) {
    h = *(int *)cut->getAttributeVal("hpos", i);
    v = *(int *)cut->getAttributeVal("zpos", i);
    sorted[v][h] = i;
//  cout << "(" << h << ", " << v << ", " << i << ")" << endl; 
  }
  
  int transpose[cut->card()];
  map<int, map<int, int> >::iterator vi;
  map<int, int>::iterator hi;
  int i = 0;
  for (vi=sorted.begin(); vi!=sorted.end(); vi++) {
    for (hi=(*vi).second.begin(); hi!= (*vi).second.end(); hi++) {
      transpose[i] = (*hi).second;
 //     cout << transpose[i] << ", " ;
      i++;
    }
  }
  
  Array *arrtranspose = new Array("transpose", INT);
  arrtranspose->shareData(transpose, cut->card());
  
  cut->Bind(arrtranspose);
  

//  ArrayWriter *hcrossv = new ArrayWriter("HcrossV", 0, "");
//  hcrossv->Write(cut, "salt");
//  
/* 
  secs = gettime(); 
  Condition *saltnulls = new Condition("salt", 1, ">");
  cut = RestrictOp::Restrict(saltnulls, cut);
  cout << gettime() - secs << tab << "( restrict air cells )" << endl; 
*/
  
  secs = gettime(); 
  cut->grid->normalize();
  cout << gettime() - secs << tab << "( normalize )" << endl; 
 
  cout << gettime() - start << tab << "( Total )" << endl;
  cutH->grid->normalize();
  vtkGridField *bath = vtkGridField::New();
  bath->UseNamedPerspective();
  bath->SetGridField(cutH);
  bath->Update();
  bath->GetOutput()->GetPointData()->SetActiveScalars("h");
  //bath->GetOutput()->PrintSelf(cout, 1);
  
  string dataprod;
  if (argc >=4) {
    dataprod = argv[3];
  } else {
    dataprod = "";
  } 
  
  // iterate over timesteps
  int timesteps = 96;
  
  OneGrid *T = new OneGrid("T", timesteps);
  cout << "make range: " << endl;
  Range *addresses = new Range("t", timesteps);
  //( (Array *) addresses)->print();
  GridField *Tt = new GridField(T, 0, (Array *) addresses);
  int timevals = 2*4;
  int surf = H->card()*4;
  int timestephead = timevals + surf;
  int header = 1135132;
  stringstream addrexpr;
  addrexpr << "(t*(" << timestephead;
  addrexpr << " + " << totalsize*4 << "))";
  addrexpr << "+" << timestephead;

  stringstream newaddrexpr;
  newaddrexpr << cut->card() << "*4*t";
  ApplyOp::Apply(Tt, "addresses", addrexpr.str());
  ApplyOp::Apply(Tt, "newaddresses", newaddrexpr.str());
  Tt->print();


  
  ArrayReader *ar;
  ArrayWriter *aw;
  float address;
  stringstream attrname;
  attrname << "salt";
  Array *salt = new Array("salt", FLOAT);
  vtkGridField *vtkgrid;
  for (int t=0; t<timesteps; t++) {
    address = *(float *) Tt->getAttributeVal("addresses", t);
    ar = new ArrayReader(argv[1], header + int(address), "wetpos");
    ar->Read(cut, 0, salt);
    //salt->print(); 
    cut->Bind(salt);
    int addr =  (int)*(float *)Tt->getAttributeVal("newaddresses", t);
    cout << "target address: " << addr << endl;
    aw = new ArrayWriter("VcrossH", addr, "transpose");
    aw->Write(cut, "salt");
 /* 
    secs = gettime(); 
    vtkgrid = vtkGridField::New();
    vtkgrid->UseNamedPerspective();
    vtkgrid->SetGridField(cut);
    vtkgrid->Update(); 
    cout << gettime() - secs << tab << "( to VTK )" << endl; 
  
    vtkgrid->GetOutput()->GetPointData()->SetActiveScalars("salt");
  
    Visualize(vtkgrid->GetOutput(), bath->GetOutput(), dataprod);
    */
  }  
} 
