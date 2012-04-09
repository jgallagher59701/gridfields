#include "gridfield.h"
#include "array.h"
#include "read63.h"
#include "arrayreader.h"
#include "refrestrict.h"
#include "cross.h"
#include "merge.h"
#include "join.h"
#include "onegrid.h"
#include <sstream>
#include <string>
#include "apply.h"
#include "assignments.h"
#include "aggregations.h"
#include "timing.h"
#include "visualize.h"
#include "vtkGridField.h"
#include "expr.h"
#include "dataprods.h"
#include "corierecipes.h"

struct Point {
  float x;
  float y;
};

int main( int argc, char *argv[] ) {
  cout << setprecision(3);
  cout.setf(ios::fixed);
  cout << endl;

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
  
  //which visualiztion to produce
  if (argc < 4) {
    dataprod = "";
  } else {
    dataprod = argv[3];
  }

  //which recipes to execute
  int recipe = 0;
  if (argc >= 5) {
    stringstream ss(argv[4]);
    ss >> recipe;
  }
  
  
  //string vslice(argv[2]);
  
  // read in the H and V grids
  secs = gettime();
  GridField *H = readHGrid(argv[1]);
  cout << gettime() - secs << tab << "( read H )" << endl;

  secs = gettime();
  GridField *V = readVGrid(argv[1]);
  cout << gettime() - secs << tab << "( read V )" << endl;
  cout << ">>>" << recipe << endl; 
  switch (recipe) {
    case 0:
      cout << "3D " << recipe << endl;
      scalar3D(H, V, argv[1], 1135132 + 8 + 29602*4, region, dataprod); 
      break;
    case 1:
      cout << "Horiz " << recipe << endl;
      HorizontalSlice(H,V, argv[1], 1135132 + 8 + 29602*4, region, 60); 
      break;
    case 2:
      cout << "Vert " << recipe << endl;
      VerticalSlice(H,V, argv[1], 1135132 + 8 + 29602*4, region); 
      break;
    case 3:
      cout << "IVert " << recipe << endl;
      InterpolateVertSlice(H,V, argv[1], 1135132 + 8 + 29602*4, region); 
      break;
      
    default:
      scalar3D(H, V, argv[1], 1135132 + 8 + 29602*4, region, dataprod); 
  }

}

void InterpolateVertSlice(GridField *H, GridField *V, const char *filename,
                     int offset, string region) {

  float start = gettime();
  GridField *Pn = readCleanStations("../tests/reference/vslice_mchann.bpclean", 97);
  
  Assign::pointpoly3 pp3;
  Assign::pointpoly pp;
  Assign::neighbors neigh;
  Aggregate::first f;
  Aggregate::count cnt;
  Aggregate::mkvector clump("poly");
/*
  Pn = ApplyOp::Apply(Pn, "z", "4825.1*100");
  Pn->print();
  vtkGridField *vtkgrid = toVTK(Pn, "x");


  vtkGridField *bath = makeBathymetry(H,100);
  DirectVis(vtkgrid->GetOutput(), bath->GetOutput());
*/

  computeColumnPositions(H,V);
  V->recordOrdinals("zpos");
  
  Zoom(H, V, region);
  
  readArray(H, 0, filename, INT, 1135132 + 8, "surf", ""); 


  GridField *H2 = new GridField(H->grid, 2);
  H2 = AggregateOp::Aggregate(H2, neigh, clump, H); 
  
  GridField *Hmark = MergeOp::Merge(H2,AggregateOp::Aggregate(H2, pp3, cnt, Pn)); 
  Condition *marks = new Condition("count", 1, ">");
  GridField *Hslice2 = RestrictOp::Restrict(marks, Hmark);
  //cout << "SIZE: " << Hslice2->card() << endl;
 
  Hslice2->recordOrdinals("hpos");
  Hslice2->Prune();

  Assign::pointpoly2 pp2;
  vector<string> attrs;
  attrs.push_back("hpos");
  attrs.push_back("b");
  attrs.push_back("surf");
  
  Aggregate::project proj(attrs);
  Aggregate::max<int> maxb("b");
  Aggregate::min<int> minsurf("surf");
  
  GridField *Hslice0 = MergeOp::Merge(H, Hslice2);
  Hslice2  = MergeOp::Merge(Hslice2, 
      AggregateOp::Aggregate(Hslice2, neigh, maxb, Hslice0));
  Hslice2  = MergeOp::Merge(Hslice2, 
      AggregateOp::Aggregate(Hslice2, neigh, minsurf, Hslice0));
  Hslice2->getScheme()->print();
  Pn = MergeOp::Merge(Pn,AggregateOp::Aggregate(Pn, pp2, proj, Hslice2));
 
  Condition *ztrim = new Condition("zpos", 20, ">");
  V = RestrictOp::Restrict(ztrim, V);
  scaleZ(V,100);
 
  //prune the one-cells
  Grid *gV0 = new Grid("V0");
  gV0->setKCells(V->grid->getKCells(0), 0);
  Grid *gV = V->grid;
  V->grid = gV0;
  GridField *Hslice0V = CrossOp::Cross(Hslice0, V);

  Condition *cond = new Condition("zpos", "b", ">");
  Hslice0V = RestrictOp::Restrict(cond, Hslice0V);  
 
  //read salt 
  computeAddresses(Hslice0V);
  readArray(Hslice0V, 0, filename, FLOAT, offset, "salt", "wetpos");
 
  Hslice0V->grid->normalize();  
  
  V->grid = gV;
  GridField *PnV = CrossOp::Cross(Pn, V);
  PnV = RestrictOp::Restrict(cond, PnV);  
  Condition *surf = new Condition("zpos", "surf", "<");
  PnV = RestrictOp::Restrict(surf, PnV);
  PnV->grid->normalize();  
  
  Aggregate::interpolate2D inter("salt");
  
  GridField *Pout = MergeOp::Merge(PnV,
      AggregateOp::Aggregate(PnV, pp, inter, Hslice0V));
 
  vtkGridField *vtkgrid = toVTK(Pout, "salt");
  cout << gettime() - start << tab << "( Total )" << endl; 
  //ShowEdges(vtkgrid->GetOutput());
  DirectVis(vtkgrid->GetOutput());
}

void BadInterpolateVertSlice(GridField *H, GridField *V, const char *filename,
                     int offset, string region) {

  // NONFUNCTIONAL
  int i;
  int s = 5; //nodes->size();
  

  float start = gettime();
  float tpost;
  
  tpost = gettime();
  GridField *Pn = readCleanStations("../tests/reference/vslice_mchann.bpclean", 97);
  //int ns[5] = {10, 100, 1000, 10000, 29000};
  //nodes->copyData(ns, 5);
  cout << gettime() - tpost << tab << "( read Stations )" << endl; 

 /*
  * 1) H2->(ord)
  * 2) H0->(x,y,h,b)
  * 3) P0->(x,y)
  * 4) Agg(H2, neighbors(2,0), mkpoly, H0)
  * 5) Agg(H, pnpoly, idset)
  * 6) Restrict(H2, "picks=1")
  * 6) Merge(K,H2)
  */  
  //Pn->print();
  
//  H->recordOrdinals("hpos");
 
  Assign::pointpoly pp;
  //Assign::pointpoly2 pp2;
  Aggregate::first f;
  Aggregate::count c;
  
  /*
  vector<string> hpos;
  hpos.push_back("hpos");
  Aggregate::project p_hpos(hpos);
  */

  //get the cell ids for 2-cells as data
  GridField *H2 = new GridField(H->grid, 2);
/*
  Assign::neighbors neigh;
  H2 = AggregateOp::Aggregate(H2, neigh, mkvector, H);
  PAggregateOp::Aggregate(Pn, pp2, f, H2);
*/
  /*
  H2->recordOrdinals("hack");
  
  Pn->recordOrdinals("hack"); 
  
  Pn->getAttribute("hack")->print();
  tpost = gettime();
  computeColumnPositions(H,V);
  GridField *Px = AggregateOp::Aggregate(Pn, pp, f, H); 
  cout << gettime() - tpost << tab << "( Find Cells )" << endl; 
 
  Pn->getAttribute("hack")->print();
  

  
  Assign::match m("hack");
  Assign::neighbors neigh;
  Aggregate::max<int> maxc("count");

  GridField *H2ids = AggregateOp::Aggregate(H2, m, c, Pn); 
  H2ids->getAttribute("count")->print();
  GridField *HIdsOnNodes = AggregateOp::Aggregate(H, neigh, maxc, H2ids); 
  HIdsOnNodes->getAttribute("count")->print();
  H = MergeOp::Merge(H, HIdsOnNodes);
  H->getScheme()->print();
  H->getAttribute("count")->print();
  */
 /* 
  Condition *countcond = new Condition("count", 1, "=");
  GridField *Hstrip = RestrictOp::Restrict(countcond, H2ids);

  Assign::neighbors nodes; 

  GridField *FoundCells = MergeOp::Merge(H2ids, Hnodes);
  GridField *FoundCellsWithGeom = MergeOp::Merge(H, FoundCells);
  
  vtkGridField *vtkgrid = toVTK(FoundCellsWithGeom, "salt");
  ShowEdges(vtkgrid->GetOutput());
  */

  /*
  tpost = gettime();
  V->recordOrdinals("zpos");
  cout << gettime() - tpost << tab << "( reify Ordinals )" << endl; 

  //ApplyOp::Apply(Px, "x", "x");
  //ApplyOp::Apply(Px, "y", "y");

  tpost = gettime();
  
  Condition *cond = new Condition("zpos", 34, ">");
  V = RestrictOp::Restrict(cond, V);  
  
  scaleZ(V,100);
  cout << gettime() - tpost << tab << "( scale Z )" << endl; 
 
  tpost = gettime();
  GridField *PV = CrossOp::Cross(Px, V);
//  PV->getAttribute("b")->print();
//  PV->getAttribute("zpos")->print();
  //PV->print();
  cout << gettime() - tpost << tab << "( UserGrid `cross` V )" << endl; 
*/
 /* 
  Condition *debug = new Condition("x", 330900.0f, "<");
  PV = RestrictOp::Restrict(debug, PV);
  
  //PV->getAttribute("z")->print();
  tpost = gettime();
  Condition *bot = new Condition("zpos", "b", ">");
  PV = RestrictOp::Restrict(bot, PV);
  cout << gettime() - tpost << tab << "( restrict bathymetry )" << endl; 
 // PV->getAttribute("z")->print();
  */
  /*
  computeAddresses(PV);
  
  
  readArray(PV, 0, filename, FLOAT, offset, "salt", "wetpos");
 */
  //PV->print();
  /* 
  tpost = gettime();
  Condition *top = new Condition("salt", 0.1f, ">");
  PV->getAttribute("z")->print();
  PV = RestrictOp::Restrict(top, PV);
  cout << gettime() - tpost << tab << "( restrict surface )" << endl; 
 */
  /*
  vtkGridField *vtkgrid = toVTK(PV, "salt");
  
  cout << gettime() - start << tab << "( Total )" << endl; 
 
  ShowEdges(vtkgrid->GetOutput());
  //DirectVis(vtkgrid->GetOutput());
  //Visualize(vtkgrid->GetOutput(), vtkgrid->GetOutput(), "edges");
*/
}

void VerticalSlice(GridField *H, GridField *V, const char *filename,
                     int offset, string region) {
  int i;
  int s = 5; //nodes->size();
  

  float start = gettime();
  float tpost;
  
  tpost = gettime();
  GridField *Pn = readCleanStations("../tests/reference/vslice_mchann.bpclean", 97);
//  GridField *Pn = readCleanStations(region, 0);
  //int ns[5] = {10, 100, 1000, 10000, 29000};
  //nodes->copyData(ns, 5);
//  cout << gettime() - tpost << tab << "( read Stations )" << endl; 

  readArray(H, 0, filename, INT, 1135132 + 8, "surf", ""); 
  //Pn->print();
  
//  H->recordOrdinals("hpos");
 
  Assign::pointpoly pp;
  Aggregate::first f;
  
  /*
  vector<string> hpos;
  hpos.push_back("hpos");
  Aggregate::project p_hpos(hpos);
  */
  tpost = gettime();
  V->recordOrdinals("zpos");
//  cout << gettime() - tpost << tab << "( reify Ordinals )" << endl; 
  
  tpost = gettime();
  computeColumnPositions(H,V);
  Zoom(H, V, region);
  H->grid->normalize();
  GridField *Px = AggregateOp::Aggregate(Pn, pp, f, H); 
//  cout << gettime() - tpost << tab << "( Find Cells )" << endl; 
  
  

  //ApplyOp::Apply(Px, "x", "x");
  ApplyOp::Apply(Px, "y", "0");

  tpost = gettime();
  
  Condition *cond = new Condition("zpos", 22, ">");
  V = RestrictOp::Restrict(cond, V);  
  
  scaleZ(V,100);
//  cout << gettime() - tpost << tab << "( scale Z )" << endl; 
 
  
  tpost = gettime();
  GridField *PV = CrossOp::Cross(Px, V);
//  PV->getAttribute("b")->print();
//  PV->getAttribute("zpos")->print();
  //PV->print();
//  cout << gettime() - tpost << tab << "( UserGrid `cross` V )" << endl; 

  Condition *surf = new Condition("zpos", "surf", "<");
  PV = RestrictOp::Restrict(surf, PV);

  
 // Condition *debug = new Condition("x", 330900.0f, "<");
 // PV = RestrictOp::Restrict(idebug, PV);
  
  computeAddresses(PV);
  Condition *bot = new Condition("zpos", "b", ">");
  PV = RestrictOp::Restrict(bot, PV);
  cout << gettime() - tpost << tab << "( restrict bathymetry )" << endl; 
  
  readArray(PV, 0, filename, FLOAT, offset, "salt", "wetpos");
 
  //PV->print();
  
  /* 
  tpost = gettime();
  Condition *top = new Condition("salt", 0.1f, ">");
  PV->getAttribute("z")->print();
  PV = RestrictOp::Restrict(top, PV);
  cout << gettime() - tpost << tab << "( restrict surface )" << endl; 
 */
  vtkGridField *vtkgrid = toVTK(PV, "salt");
  
  cout << gettime() - start << tab << Pn->card() << endl << "( Total )" << endl; 
 
  //ShowEdges(vtkgrid->GetOutput());
  DirectVis(vtkgrid->GetOutput());
  //Visualize(vtkgrid->GetOutput(), vtkgrid->GetOutput(), "edges");
}

void HorizontalSlice(GridField *H, GridField *V, const char *filename, 
                     int offset, string region, int depth) {

  float start = gettime();
  // find V-index from z value here
  int v = depth;
  float scale = 60;
  
  stringstream zstr;
  float z = *(float *)V->getAttributeVal("z", v); //+ 4825.1;
  cout << z << ", " << v << " <-- Z, V" << endl;
  zstr << *(float *) V->getAttributeVal("z", v);
  
  GridField *cutV = V;
 
  //this one is the trick... 
  computeColumnPositions(H,V);
 
  Zoom(H, cutV, region);
  
  Condition *p = new Condition("b", v, "<");
  GridField *cutH = RestrictOp::Restrict(p, H);

  
  stringstream expr;
  expr << "addr + " << v << " - b";
  ApplyOp::Apply(cutH, "sliceaddr", expr.str());
  
  readArray(cutH, 0, filename, FLOAT,
               offset, "salt", "sliceaddr");
 
  cutH = cutSurface(cutH);
  
  ApplyOp::Apply(cutH, "z", zstr.str());

  scaleZ(cutH, scale);
  
  vtkGridField *vtkgrid = toVTK(cutH, "salt");
  cout << gettime() - start << tab << "( Total )" << endl; 
    
  vtkGridField *bath = makeBathymetry(H,scale);
  //isolines (unfilled)
  //Visualize(vtkgrid->GetOutput(), bath->GetOutput(), "");
  DirectVis(vtkgrid->GetOutput(), bath->GetOutput());
}

int scalar3D( GridField *H, GridField *V, const char *filename, 
              int addr, string region, string dataprod ) {

  float start = gettime();
  float secs;
  
  float scale = 60.0f;
  
  computeColumnPositions(H,V);
  
  // add a data column representing the order in this grid
  V->recordOrdinals("zpos");
  
//  readArray(H, 0, filename, INT, 1135132 + 8, "surf", ""); 
  
  GridField *cutH = H;
  GridField *cutV = V;

  Zoom(cutH,cutV,region);
 
  scaleZ(cutV,scale);
 
  GridField *cut = makeWetGrid(cutH, cutV);
/*
  Condition *surf = new Condition("zpos", "surf", "<");
  cut = RestrictOp::Restrict(surf, cut); 
  */
  computeAddresses(cut);
  
  readArray(cut, 0, filename, FLOAT, addr, "salt", "wetpos");
  
  vtkGridField *vtkgrid = toVTK(cut, "salt");
  
  cout << gettime() - start << tab << "( Total )" << endl; 
  cout << "Selectivity: " << cut->card() << ", " << (829852) << endl;
  vtkGridField *bath = makeBathymetry(cutH, scale);
 
  // call the data product visualization function
  Visualize(vtkgrid->GetOutput(), bath->GetOutput(), dataprod);
}
