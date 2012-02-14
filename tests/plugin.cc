#include <fstream>
#include <iostream>
#include <sstream>
#include "elio.h"
#include "array.h"
#include "grid.h"
#include "arrayreader.h"
#include "arraywriter.h"
#include "gridfield.h"
#include "assignments.h"
#include "aggregations.h"
#include "merge.h"
#include "restrict.h"
#include "apply.h"
#include "read63.h"
#include "write63.h"
#include "corierecipes.h"

using namespace std;

int main(int argc, char **argv) {

  //check arguments
  string infile("source_salt.63");
  string outfile("1_salt.63");
  int outstep = 0;
  int start = outstep;
  int stop = outstep + 2;
  int skip = 1;
  stringstream label;
  string surf("surf");
  string scalar("scalar");
  string region("myzoom");
  //----------------------------
  
  ElcircHeader h;
  GetHeader(&h, (char *) infile.c_str());

  GridField *H = gfH(h);
  GridField *V = gfV(h);
  
  int isize = sizeof(int);
  int fsize = sizeof(float);
  int timesize = isize+fsize;
  int surfsize = H->Card(0)*isize;
  int timestepsize = h.ssize;
  int headersize = h.hsize;
  float stepsecs = h.timestep;
  
  computeColumnPositions(H,V);
  AccumulateOp *ordV = new AccumulateOp(V, "vpos", "vpos+1", "0");
  AccumulateOp *ordH = new AccumulateOp(H, "hpos", "hpos+1", "0");

  GridField *cutH = ordH->getResult();
  GridField *cutV = ordV->getResult();

  // convenience function for restricting the 
  // horizontal and vertical grids to a specified region
  Zoom(cutH,cutV,region);

 /* 
  cutV->recordOrdinals("newpos");
  Assign::nearest n("b","zpos");
  vector<string> attrs;
  attrs.push_back("newpos");
  attrs.push_back("zpos");
  Aggregate::project p(attrs);
  GridField *newb = AggregateOp::Aggregate(cutH, n, p, cutV);
  cutH = MergeOp::Merge(cutH, newb);
  //cutH->getAttribute("newpos")->print();
  //cutH->getAttribute("zpos")->print();
  cutH->getAttribute("newpos")->print();
*/
  
  //XMVIS expects node ids to be contiguous
  cutH->grid->normalize();
  cutV->grid->normalize();

  //scaleZ(cutV,scale);
 
  // construct the 3D grid
  GridField *cut = makeWetGrid(cutH, cutV);

//  cut->print();
  
  /*
  // cut away the grid above the surface
  Condition *surf = new Condition("zpos", "surf", "<");
  cut = RestrictOp::Restrict(surf, cut); 
  */

  //compute the location of each value in the new, smaller 
  //grid as an address into the old, larger grid
  //The general operation is NOT currently expressible in the algebra
  computeAddresses(cut);
  
  // Our output grid is an aggregate over the vertical direction,
  // so to satisfy the .63 file format, we use a dummy vertical
  // grid consisting of just one layer.
  GridField *onelayer = cutV;
  onelayer = 
     RestrictOp::Restrict("z=4825.0", 0, cutV);
  cout << "onelayer: " << cutV->Card(0) << endl;
  
  // we have to fix indices into V, since V might have changed
  // the bathymetry and the surface values both can be wrong;
  // (This step is important for 3d output grids, but
  // the single-layer grid has a trivial bathymetry: '0')
  Assign::unify u;
  Aggregate::statistics stats("zpos");
  GridField *unit = new GridField(new UnitGrid);
  GridField *vstats = AggregateOp::Aggregate(unit, 0, u, stats, onelayer, 0);
  int max = vstats->GetIntAttributeVal(0, "max_zpos", 0);
  int min = vstats->GetIntAttributeVal(0, "min_zpos", 0);
  cout << max << ", " << min << endl;

  // the index-mapping expression below is used for both.
  // indices below the new minimum are now 0, indices above
  // the new maximum are now max + 1
  stringstream ss;
  ss << "((temp>" << max << ")*" << max + 1 << 
     " + (temp<" << min + 1 << ")*" << min << 
     " + ((temp<" << max+1 << ")&(temp>"<< min <<"))*temp) - " << min;
  
  cutH = ApplyOp::Apply(cutH, "temp", "b");
  cutH = ApplyOp::Apply(cutH, "hackb", ss.str());
  cutH->GetAttribute(0, "hackb")->print();
  cutH = ApplyOp::Apply(cutH, "b", "hackb");
  cutH->GetAttribute(0, "b")->cast(INT);

  // reduce H further, by removing those cells 
  // that have no vertical extent
  cout << cutH->card() << endl;
  //cutH = RestrictOp::Restrict(
 //     new Condition("b", onelayer->card() - 1, "<"), cutH);
  cout << cutH->card() << endl;
  // read the chosen timesteps
  string surfattr;
  Assign::cross cross(cutV, 0);
  Assign::match m("hpos");
  Aggregate::statistics *stat;
  
//  cutH->print();
//  cutV->print();
  for (int t=start; t<stop; t+=skip) {
    label.str("");  label << t;
    surfattr = surf + label.str();

    //read surface indices, a gridfield over H
    readArray(cutH, 0, infile, INT, 
              headersize + t*timestepsize + timesize, 
              surfattr, "hpos"); 

    // map the surface indices to the new vertical grid
    cutH = ApplyOp::Apply(cutH, "temp", surfattr);

    //now we use our index-mapping expression
    cutH = ApplyOp::Apply(cutH, surfattr, ss.str());
    cutH->getAttribute(surfattr.c_str())->cast(INT);
    
    // read data, a gridfield over r(HxV)
    readArray(cut, 0, infile, FLOAT, 
              headersize + t*timestepsize + timesize + surfsize, 
              scalar + label.str(), "wetpos"); 
  
    // compute min and max of each vertical column
    stat = new Aggregate::statistics(scalar+label.str());
    cutH = MergeOp::Merge(cutH, AggregateOp::Aggregate(cutH, cross, *stat, cut)); 
  }
  

  //We're ready to write out the gridfield
  ofstream *f = prepfile(outfile, 0);
  h.nsteps = (stop-start)/skip;
  writeHeader(h, *f);
  writeVGrid_bin(onelayer, *f);
  writeHGrid_bin(cutH, *f);

  ArrayWriter *ar = new ArrayWriter(f);
  for (int t=start; t<stop; t+=skip) {
    writeTime(10*(1+t), (1+t)*stepsecs, *f);
    label.str("");  label << t;
    ar->Write(cutH, surf + label.str());
    ar->Write(cutH, "max_" + scalar + label.str());
  }
  f->close();
}

