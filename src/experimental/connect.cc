#include "connect.h"
#include "gridfield.h"
#include "Delaunay.h"
#include "vtkGridField.h"
#include "vtkCellArray.h"
#include "vtkDelaunay2D.h"
#include "vtkPolyData.h"
#include "visualize.h"

namespace tri {
extern "C" {
#define ANSI_DECLARATORS
#include "triangle.h"
}
}

ConnectOp::ConnectOp(GridFieldOperator *previous) : UnaryGridFieldOperator(previous) {}

void ConnectOp::Execute() {
  this->PrepareForExecution();
  this->Result = ConnectOp::Connect(this->PreviousOp->getResult());
}

GridField *ConnectOp::Connect(GridField *gf) {
 /* 
  // check some assertions
  assert(gf->isAttribute("x"));
  assert(gf->isAttribute("y"));
  
  AbstractCellArray *nodes = gf->grid->getKCells(0);
  nodes->ref();
  
  vtkGridField *vtkgf = toVTK(gf, "x");
  vtkCellLinks *links = vtkgf->GetOutput()->GetCellLinks();
  
  vtkPolyData *lines = vtkPolyData::New();
  lines->SetLines(linearray);

  vtkDelaunay2D *delaunay = vtkDelaunay2D::New();
  delaunay->SetInput(vtkgrid->GetOutput());
  delaunay->SetSource(lines);

  vtkCellArray *triangles = delaunay->GetOutput()->GetPolys();
  
  Grid *newgrid = new Grid(gf->grid->name, 2);
  newgrid->setKCells(nodes, 0);
  
  int *trinodes = new int[tset.size()*3];
  
  int sz;
  int pts[10];
  
  while(triangles->GetNextCell(sz, pts)) {
    if (sz != 3) Fatal("Wrong size for triangle: %i", sz); 
    trinodes[3*j + 0] = pts[0]; 
    trinodes[3*j + 1] = pts[1]; 
    trinodes[3*j + 2] = pts[2]; 
  }

  CellArray *newcells = new CellArray(trinodes, tset.size(), 3);
  newgrid->setKCells(newcells, 2);

  GridField *Out = new GridField(gf);
  Out->setGrid(newgrid);

  return Out;
  */
}

GridField *ConnectOp::ConnectTriangle(GridField *gf) {
  
  // check some assertions
  assert(gf->isAttribute("x"));
  assert(gf->isAttribute("y"));
  
  AbstractCellArray *nodes = gf->grid->getKCells(0);
  nodes->ref();
  
  Grid *newgrid = new Grid(gf->grid->name, 2);
  newgrid->setKCells(nodes, 0);
  
  struct tri::triangulateio in, mid, vorout;
  
  in.numberofpoints = 10; //nodes->getsize();

  in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));  
  //new REAL[in.numberofpoints * 2];
  
  for (int i=0; i<in.numberofpoints; i++) {
    REAL x = gf->getFloatAttributeVal("x", i);    
    REAL y = gf->getFloatAttributeVal("y", i);
    in.pointlist[2*i + 0] = 0.0+i;
    in.pointlist[2*i + 1] = 1.0*i*i;
//    cout << x << ", " << y << endl;
  }
  for (int i=0; i<in.numberofpoints*2; i++) {
    cout << in.pointlist[i] << endl;
  }
 /* 
  in.pointlist[0] = 0.0;
  in.pointlist[1] = 0.0;
  in.pointlist[2] = 1.0;
  in.pointlist[3] = 0.0;
  in.pointlist[4] = 1.0;
  in.pointlist[5] = 10.0;
  in.pointlist[6] = 0.0;
  in.pointlist[7] = 10.0;
  */

  in.numberofpointattributes = 0;
 /*
  in.numberofpointattributes = 1;
  in.pointattributelist = new REAL[in.numberofpoints*in.numberofpointattributes];
  for (int i=0; i<in.numberofpoints; i++) {
    in.pointattributelist[i] = float(i);
  }
  in.pointattributelist[0] = 0.0;
  in.pointattributelist[1] = 1.0;
  in.pointattributelist[2] = 11.0;
  in.pointattributelist[3] = 10.0;
  */

  in.pointmarkerlist = (int *) NULL;
  /*
  in.pointmarkerlist = new int[in.numberofpoints];
  for (int i=0; i<in.numberofpoints; i++) {
    in.pointmarkerlist[i] = 0;
  }
  in.pointmarkerlist[0] = 0;
  in.pointmarkerlist[1] = 2;
  in.pointmarkerlist[2] = 0;
  in.pointmarkerlist[3] = 0;
  */

  in.numberofsegments = 0;
  in.numberofholes = 0;
  
  in.numberofregions = 0;
 /*  
  in.regionlist = new REAL[in.numberofregions * 4]; 
  in.regionlist[0] = 0.5;
  in.regionlist[1] = 5.0;
  in.regionlist[2] = 7.0;  
  // Regional attribute (for whole mesh).
  in.regionlist[3] = 0.1;  
  // Area constraint that will not be used.
  */

        
  mid.pointlist = (REAL *) NULL;
  mid.pointattributelist = (REAL *) NULL;
  mid.pointmarkerlist = (int *) NULL;
  mid.trianglelist = (int *) NULL;
  mid.triangleattributelist = (REAL *) NULL;
  mid.neighborlist = (int *) NULL;
  mid.segmentlist = (int *) NULL;
  mid.segmentmarkerlist = (int *) NULL;
  mid.edgelist = (int *) NULL;
  mid.edgemarkerlist = (int *) NULL;

  vorout.pointlist = (REAL *) NULL;
  vorout.pointattributelist = (REAL *) NULL;
  vorout.edgelist = (int *) NULL;
  vorout.normlist = (REAL *) NULL;

  tri::triangulate("pczevn", &in, &mid, &vorout);
  
  CellArray *newcells = new CellArray(mid.trianglelist, 
                                      mid.numberoftriangles, 
                                      mid.numberofcorners);
  newgrid->setKCells(newcells, 2);

  GridField *Out = new GridField(gf);
  Out->setGrid(newgrid);

  return Out;
}

GridField *ConnectOp::ConnectDelaunay(GridField *gf) {
  
  // check some assertions
  assert(gf->isAttribute("x"));
  assert(gf->isAttribute("y"));
  
  AbstractCellArray *nodes = gf->grid->getKCells(0);
  nodes->ref();
  
  Grid *newgrid = new Grid(gf->grid->name, 2);
  newgrid->setKCells(nodes, 0);
  
  vertexSet vset;

  for (int i=0; i<nodes->getsize(); i++) {
    float x = gf->getFloatAttributeVal("x", i);    
    float y = gf->getFloatAttributeVal("y", i);    
    vset.insert(vertex(x,y,i));
  }

  triangleSet tset;

  Delaunay d;

  d.Triangulate(vset, tset);
  
  int *trinodes = new int[tset.size()*3];
  
  tIterator ti;
  int j = 0;
  for (ti=tset.begin(); ti!=tset.end(); ti++) {
    trinodes[3*j + 0] = ti->GetVertex(0)->GetId(); 
    trinodes[3*j + 1] = ti->GetVertex(1)->GetId(); 
    trinodes[3*j + 2] = ti->GetVertex(2)->GetId(); 
    j++;
  }

  CellArray *newcells = new CellArray(trinodes, tset.size(), 3);
  newgrid->setKCells(newcells, 2);

  GridField *Out = new GridField(gf);
  Out->setGrid(newgrid);

  return Out;
}
