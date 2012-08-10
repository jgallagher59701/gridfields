#include "src/grid.h"
#include "src/gridfield.h"
#include "src/array.h"
#include "src/arrayreader.h"
#include "src/arraywriter.h"
#include "src/tonetcdf.h"

using namespace GF;

Grid *makeGrid(int scale,const char *name) {
  CellArray *twocells;
  CellArray *onecells;
  CellArray *zerocells;
  Grid *grid;
  Node triangle[3];
  Node segment[2];
  Node node;

  bool wf;
  int i;
  twocells = new CellArray();
  for (i=0; i<scale/2; i++) {
    triangle[0] = i;
    triangle[1] = i+1;
    triangle[2] = i+2;
    twocells->addCellNodes(triangle, 3);
  }
  //twocells->print();
  //getchar(); 
  onecells = new CellArray();
  for (i=0; i<scale-1; i++) {
    segment[0] = i;
    segment[1] = i+1;
    onecells->addCellNodes(segment, 2);
  }
  //onecells->print();
  
  //getchar(); 
  grid = new Grid(name, 2);
  grid->setImplicit0Cells(scale);
  grid->setKCells(onecells, 1);
  grid->setKCells(twocells, 2);
  //grid->print(0);
  //getchar();
  return grid; 
}


Array *makeFloatArray(int size,const char *name) {
  Array *arr;
  arr = new Array(name, FLOAT, size);
  float *data;
  arr->getData(data);
  int i;

  for (i=0; i<size; i++) {
      data[i] = 2*i-10;
  }
  return arr;  
}
GridField *makeGridField(unsigned int size,const char *gridname,const char *datname, int k) {

  Grid *G;
  GridField *GF;
  Array *data;
  
  G = makeGrid(size, gridname);
  k = 0;
  data = makeFloatArray(size, datname);
  Array *y = makeFloatArray(size, "y");
  Array *node = makeFloatArray(size, "node");

  GF = new GridField(G);
  GF->AddAttribute(k, data);
  GF->AddAttribute(k, y);
  GF->AddAttribute(k, node);
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

int main(int argc, char **argv) {
  GridField *Z, *XYZ, *XY;
  GridField *Result;

  XY = makeGridField(5, "XY", "x", 2);
  Z = makeGridField(3, "Z", "level", 0);
  XYZ = makeGridField(15, "salt", "salt", 0);
  Array *temp = makeFloatArray(15, "temp");
  Array *zcor = makeFloatArray(15, "zcor");
  Array *u = makeFloatArray(15, "u");
  Array *v = makeFloatArray(15, "v");

  XYZ->AddAttribute(0, zcor);
  XYZ->AddAttribute(0, temp);
  XYZ->AddAttribute(0, u);
  XYZ->AddAttribute(0, v);

  //XYZ->print();
  
  NcFile *ncdf = new NcFile("data.nc", NcFile::Replace);

  GridFieldOperator *opx = new OutputNetCDFDim(ncdf, 2, "node", XY);
  GridFieldOperator *opy = new OutputNetCDFDim(ncdf, 0, "z", Z);
  GridFieldOperator *opv = new OutputNetCDFVars(ncdf, Scheme("node,z"), 0, XYZ, 0, 900);

  Result = opx->getResult();
  Result = opy->getResult();
  Result = opv->getResult();
  
}

