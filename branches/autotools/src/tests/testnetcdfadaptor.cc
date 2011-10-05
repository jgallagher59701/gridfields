#include <iostream>
#include "netcdfadaptor.h"
#include "gridfield.h"
#include "grid.h"
#include "array.h"
#include "restrict.h"

using namespace std;

int main(int argc, char **argv) {
  NetCDFAdaptor ncf("example.nc");
  ncf.Open();
  
  Grid *g = new Grid("test");

  ncf.NodesFromDim("corners", g);
  cout << "got corners" << endl;
  ncf.WellSupportedPolygonsFromVars("cell_corners", "cell_edges", g);
  //ncf.HomogeneousCellsFromVar(2, "cell_corners", g);
 
  GridField *gf = new GridField(g); 
  ncf.AttributeFromVar(2, "grid_center_lon", gf);
  ncf.AttributeFromVar(2, "grid_center_lat", gf);
  ncf.AttributeFromVar(0, "grid_corner_lon2", gf);
  ncf.AttributeFromVar(0, "grid_corner_lat2", gf);
  Array *a = gf->GetAttribute(0, "grid_corner_lon");

  a->print();

  ncf.Close();

  gf->print();

  RestrictOp R("grid_corner_lon2>1.4", 0, gf);

  gf = R.getResult();

  gf->print();
  NetCDFAdaptor nco("out.nc");
  nco.Open("w");
  nco.DimFromDim("cells", gf, 2);
  nco.DimFromDim("corners", gf, 0);
  nco.CreateDim("cellcorners", 6);
  nco.CreateDim("celledges", 6);

  vector<string> centerdims;
  centerdims.push_back("cells");
  vector<string> cornerdims;
  cornerdims.push_back("corners");

  nco.VarFromAttribute("grid_center_lon", gf, 2, centerdims);
  nco.VarFromAttribute("grid_center_lat", gf, 2, centerdims);
  nco.VarFromAttribute("grid_corner_lat", gf, 0, cornerdims);
  nco.VarFromAttribute("grid_corner_lon", gf, 0, cornerdims);
  nco.VarFromIncidence("cell_corners", gf, 2, 0, "cells", "cellcorners");
  nco.VarFromIncidence("cell_edges", gf, 2, 1, "cells", "celledges");
  nco.Close();
  
//  nco.VarFromAttribute

}
