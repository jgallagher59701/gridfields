#include "gridfield.h"
#include "onegrid.h"
#include "arrayreader.h"
#include "refrestrict.h"
#include "cross.h"
#include "join.h"
#include "vtkGridField.h"
#include "vtkDataSetMapper.h"
#include "vtkRenderWindow.h"
#include "vtkPolyDataMapper.h"
#include "vtkExtractEdges.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkLabeledDataMapper.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

GridField *makeH();
Grid *makeHGrid();
GridField *makeV(int size);
Grid *makeVGrid(int size);

float xs[8] = { 386380, 386460, 386688, 386460, 386678, 386180, 386409, 386187 };
float ys[8] = { 286208, 285995, 286214, 286368, 286483, 286406, 286564, 286680 };
float bs[8] = { 50, 44, 55, 54, 55, 50, 53, 51  };
float hs[8] = { 5.122, 9.167, 1.0, 2.209, 1.614, 4.627, 2.629, 4.195  };
float zs[62] = { 2627, 3627, 4127, 4327, 4427, 4447, 4467, 4487, 4507, 4527, 4547, 4567, 4587, 4607, 4627, 4647, 4667, 4687, 4707, 4727, 4747, 4767, 4777, 4786.4, 4791.4, 4796.4, 4799.4, 4801.4, 4803.4, 4804.9, 4805.9, 4806.7, 4807.5, 4808.3, 4809.1, 4809.9, 4810.7, 4811.5, 4812.2, 4812.9, 4813.6, 4814.3, 4815, 4815.7, 4816.4, 4817.1, 4817.8, 4818.5, 4819.2, 4819.9, 4820.6, 4821.3, 4822, 4822.7, 4823.4, 4824.2, 4825, 4825.8, 4826.6, 4827.6, 4829.6, 4866 };
  
int main( int argc, char *argv[] ) {

  GridField *H = makeH();
  GridField *V = makeV( 62 );

  float *b; 
  H->getAttribute("b")->getData(b);
  float *pos = new float[H->card()];
  pos[0] = 0;
  for (int i=1; i<H->card(); i++) {
    pos[i] = pos[i-1] + V->card() - b[i-1]; 
  }
  Array *addr = new Array("addr", FLOAT);
  addr->shareData(pos, H->card());
  H->Bind(addr);
  
  Array *zpos = new Array("zpos", FLOAT);
  float *ids = new float[V->card()];
  for (int i=0; i<V->card(); i++) {
    ids[i] = i/1.0; 
  }
  zpos->shareData(ids, V->card());
  
  V->Bind(zpos);
 
// small test grid....
  Condition *xmin = new Condition("x", 386180.0f, ">");
  Condition *ymin = new Condition("y", 285900.0f, ">");
  Condition *xmax = new Condition("x", 386700.0f, "<");
  Condition *ymax = new Condition("y", 286600.0f, "<");
  Condition *zmax = new Condition("z", 483000.0f, "<");
  Condition *zmin = new Condition("z", 440000.0f, ">");
/*
  GridField *G = CrossOp::Cross(H,V);
  GridField *cut = RestrictOp::Restrict(ymin,
                   RestrictOp::Restrict(xmin, 
                   RestrictOp::Restrict(zmax, G)));
*/

  GridField *cutH = RefRestrictOp::Restrict(ymin,
                    RefRestrictOp::Restrict(ymax,
                    RefRestrictOp::Restrict(xmax,
                    RefRestrictOp::Restrict(xmin,H))));


  GridField *cutV = RefRestrictOp::Restrict(zmax,
                    RefRestrictOp::Restrict(zmin, V));

//  GridField *cutH = H;
//  GridField *cutV = V;

  Condition *bottom = new Condition("zpos", "b", ">");
//  Condition *bottom = new Condition("b", "zpos", "<");
/*
  //method 1
  GridField *wetgrid = JoinOp::Join(bottom, cutH, cutV);  
*/
  //method 2
//  GridField *cut = CrossOp::Cross(cutH, cutV);
  GridField *cut = CrossOp::Cross(cutV, cutH);
  GridField *wetgrid = RefRestrictOp::Restrict(bottom, cut);
  
/* 
  ArrayReader *ar1 = new ArrayReader(argv[1], 1135132 + 8);
  Array *surf = new Array("surf", INT);
  ar1->Read(H, 0, surf);

  surf->print();
*/

  ArrayReader *ar = new ArrayReader("firsteight", 0);
  Array *salt = new Array("salt", FLOAT);
  ar->Read(wetgrid, 0, salt);
 
  wetgrid->Bind(salt);
  float s,bv,xv,yv,zv; 
  for (int i=0; i< wetgrid->card(); i++) {
      bv = *(float *) wetgrid->getAttributeVal("b", i);
      xv = *(float *) wetgrid->getAttributeVal("x", i);
      yv = *(float *) wetgrid->getAttributeVal("y", i);
      zv = *(float *) wetgrid->getAttributeVal("z", i);
      s = *(float *) wetgrid->getAttributeVal("salt", i);
//      cout << i << ", " << xv << ", " << yv << ", " << 
//        zv << ", " << bv << ", " << s << endl;
  }
  
  float *nids = new float[wetgrid->card()];
  for (int i=0; i<wetgrid->card(); i++) {
    nids[i] = i;
  }
  Array *nodeids = new Array("nodeids", FLOAT);
  nodeids->shareData(nids, wetgrid->card());
  wetgrid->Bind(nodeids);
 
  wetgrid->grid->normalize();
  
  vtkGridField *vtkgrid = vtkGridField::New();
  vtkgrid->UseNamedPerspective();
  vtkgrid->SetGridField(wetgrid);
  
/*
  vtkUnstructuredGrid *tst = vtkgrid->MakeGrid();
  tst->PrintSelf(cout,3);
*/
  /*
  vtkDataSetMapper *gridMapper = vtkDataSetMapper::New();
  gridMapper->SetInput( vtkgrid->GetOutput() );
*/

  vtkExtractEdges *edges = vtkExtractEdges::New();
  edges->SetInput( vtkgrid->GetOutput() );
 
  vtkRenderer *ren1= vtkRenderer::New();
  
  vtkSelectVisiblePoints *visPts = vtkSelectVisiblePoints::New();
  visPts->SetInput( (vtkDataSet *)edges->GetOutput() );
  visPts->SetRenderer(ren1);
  
  vtkLabeledDataMapper *lab = vtkLabeledDataMapper::New();
  lab->SetInput( (vtkDataSet *) visPts->GetOutput() );
  lab->SetLabelFormat("%g");
  lab->SetLabelModeToLabelScalars();
  vtkActor2D *pointLabels = vtkActor2D::New();
  pointLabels->SetMapper(lab);
  
  vtkPolyDataMapper *polyMapper = vtkPolyDataMapper::New();
  polyMapper->SetInput( edges->GetOutput() );
  polyMapper->SetScalarRange(0,25);
  
  vtkActor *gridActor = vtkActor::New();
  gridActor->SetMapper( polyMapper );

  ren1->AddActor( gridActor );
  ren1->AddActor2D( pointLabels );
  ren1->SetBackground( 0.1, 0.2, 0.4 );

  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer( ren1 );
  renWin->SetSize( 500, 500 );

  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);
  
  iren->Initialize();
  renWin->Render();
  iren->Start();
  
  vtkgrid->Delete();
  //gridMapper->Delete();
  polyMapper->Delete();
  gridActor->Delete();
  ren1->Delete();
  renWin->Delete();

  return 0;
}

Array *makeArray(char *name, int size, float (*genval)(int)) {
  Array *arr;
  float data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = genval(i);
  }
  arr = new Array(name, FLOAT);
  arr->copyData(data, size);
  return arr;  
}

float genx(int i) { return xs[i]; }
float geny(int i) { return ys[i]; }
float genz(int i) { return zs[i]*100; }
float genh(int i) { return hs[i]; }
float genb(int i) { return bs[i]; }

Grid *makeVGrid(int size) {
  return new OneGrid("V",size);
}

GridField *makeV(int size) {
  Array *z = makeArray("z", size, genz);
  Grid *G = makeVGrid(size);
  return new GridField(G, 0, z);
}

Grid *makeHGrid() {
  Grid *grid;
  CellArray *twocells;
  //CellArray *onecells;

  Node nodes[3];
  //Node segment[2];

  //int i;
  twocells = new CellArray();
  
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 2;
  twocells->addCellNodes(nodes, 3);
  nodes[0] = 2;
  nodes[1] = 3;
  nodes[2] = 0;
  twocells->addCellNodes(nodes, 3);
  nodes[0] = 2;
  nodes[1] = 4;
  nodes[2] = 3;
  twocells->addCellNodes(nodes, 3);
  nodes[0] = 0;
  nodes[1] = 3;
  nodes[2] = 5;
  twocells->addCellNodes(nodes, 3);
  nodes[0] = 3;
  nodes[1] = 6;
  nodes[2] = 5;
  twocells->addCellNodes(nodes, 3);
  nodes[0] = 6;
  nodes[1] = 3;
  nodes[2] = 4;
  twocells->addCellNodes(nodes, 3);
  nodes[0] = 7;
  nodes[1] = 5;
  nodes[2] = 6;
  twocells->addCellNodes(nodes, 3);
  
  //  twocells->print();

  /* 
  onecells = new CellArray();
  for (i=0; i<scale-1; i++) {
    segment[0] = i;
    segment[1] = i+1;
    onecells->addCellNodes(segment, 2);
  }
  */
  
  //onecells->print();
  grid = new Grid("H", 2);
  grid->setImplicit0Cells(8);
  //grid->setKCells(onecells, 1);
  grid->setKCells(twocells, 2);
  //grid->print(0);
  
  return grid; 
  
}
  
GridField *makeH() {

  Grid *G;
  GridField *GF;
  Array *x, *y, *h, *b;
  
  G = makeHGrid();
  CellArray *cells = G->getKCells(0);
  x = makeArray("x", cells->getsize(), genx);
  y = makeArray("y", cells->getsize(), geny);
  h = makeArray("h", cells->getsize(), genh);
  b = makeArray("b", cells->getsize(), genb);

  GF = new GridField(G, 0);
  GF->Bind(x);
  GF->Bind(y);
  GF->Bind(h);
  GF->Bind(b);
  
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}
