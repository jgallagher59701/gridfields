#include "gridfield.h"
#include "read63.h"
#include "arrayreader.h"
#include "refrestrict.h"
#include "cross.h"
#include <sstream>
#include "apply.h"
#include "timing.h"
#include "vtkGridField.h"
#include "vtkDataSetMapper.h"
#include "vtkRenderWindow.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkWarpScalar.h"
#include "vtkLookupTable.h"
#include "vtkOutlineFilter.h"
#include "vtkExtractEdges.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkCommand.h"
#include "vtkObject.h"
#include "vtkContourFilter.h"
#include "vtkPointData.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"

void CaptureImage(vtkRenderWindow *);
void ShowCamera(vtkRenderWindow *renWin);

using namespace GF;

class vtkMyCallback : public vtkCommand {
  public:
  static vtkMyCallback *New() { return new vtkMyCallback; }

  virtual void Execute(vtkObject *caller, unsigned long, void *) {
    vtkRenderWindowInteractor *obj = reinterpret_cast<vtkRenderWindowInteractor *>(caller);
    char *key = obj->GetKeySym();
    cout << key << endl;
    if (string(key) == "v") CaptureImage(obj->GetRenderWindow());
    if (string(key) == ">") ShowCamera(obj->GetRenderWindow());
    
  }
};

GridField *makeGeometry(int size); 
Grid *makeGrid(int scale, string name);
const char tab = '\t';
int main( int argc, char *argv[] ) {

  float start = gettime();
  float secs;
//  GridField *G = makeGeometry(7);
  if (argc < 2) {
    cout << "supply a 63 file...\n";
    exit(1);
  }

  secs = gettime();
  GridField *H = readHGrid(argv[1]);
  cout << gettime() - secs << tab << "( read H )" << endl;

  //H->getAttribute("h")->print();
  //H->getAttribute("b")->print();
  
  secs = gettime();
  GridField *V = readVGrid(argv[1]);
  cout << gettime() - secs << tab << "( read V )" << endl;
  
//  V->getAttribute("z")->print();
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
  Array *addr = new Array("addr", INT);
  addr->shareData(pos, H->card());
  H->Bind(addr);
/*  
  float *bot = new float[H->card()];
  for (int i=0; i<H->card(); i++) {
    bot[i] = V->card() - b[i]; 
  }
  Array *bottom = new Array("bottom", FLOAT);
  bottom->shareData(bot, H->card());
  H->Bind(bottom);
 */
  
  
  V->recordOrdinals("zpos");
  cout << gettime() - secs << tab << "( bind new attributes )" << endl;
/*  
  Condition *zmax = new Condition("z", 4560, ">");
  Condition *zmin = new Condition("z", 4860, "<");
  */
/* 
// small test grid:
  Condition *xmin = new Condition("x", 335000, ">");
  Condition *xmax = new Condition("x", 360000, "<");
  Condition *ymin = new Condition("y", 500000, ">");
  Condition *ymax = new Condition("y", 560000, "<");
*/
 /* 
  // plume:
  Condition *xmin = new Condition("x", 63963, ">");
  Condition *xmax = new Condition("x", 551997, "<");
  Condition *ymin = new Condition("y", 110438, ">");
  Condition *ymax = new Condition("y", 454337, "<");
  */
 /* 
  // estuary:
  Condition *xmin = new Condition("x", 309491, ">");
  Condition *xmax = new Condition("x", 406390, "<");
  Condition *ymin = new Condition("y", 249063, ">");
  Condition *ymax = new Condition("y", 315713, "<");
  */
  
  // far:
  //  float bounds[6] = {-313981,2162419,-995300,750432,4700,4866};
  Condition *xmin = new Condition("x", -313981, ">");
  Condition *xmax = new Condition("x", 2162419, "<");
  Condition *ymin = new Condition("y", -995300, ">");
  Condition *ymax = new Condition("y", 750432, "<");
  Condition *zmin = new Condition("z", 4700, ">");
  Condition *zmax = new Condition("z", 4866, "<");
  
 /* 
  //myzoom:
  Condition *xmin = new Condition("x", 320000, ">");
  Condition *xmax = new Condition("x", 350000, "<");
  Condition *ymin = new Condition("y", 280000, ">");
  Condition *ymax = new Condition("y", 310000, "<");
  */

/*
  GridField *G = CrossOp::Cross(H,V);
  GridField *cut = RestrictOp::Restrict(ymin,
                   RestrictOp::Restrict(xmin, 
                   RestrictOp::Restrict(zmax, G)));
*/
  
  
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

  secs = gettime();
  float vertscale = 20;
  stringstream ss;
  ss << vertscale; 
  ApplyOp::Apply(cutH, "h", ss.str()+"*(4825.1-h)");
  ApplyOp::Apply(cutV, "z", "z*"+ss.str());
  cout << gettime() - secs << tab << "( Apply h,z  )" << endl;

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
  ArrayReader *ar1 = new ArrayReader(argv[1], 1135132 + 8);
  Array *surf = new Array("surf", INT);
  ar1->Read(H->grid, 0, surf);

  surf->print();
*/
  secs = gettime(); 
  cut = ApplyOp::Apply(cut, "wetpos", "addr+(zpos-b)");
  cout << gettime() - secs << tab << "( compute addresses )" << endl; 

  secs = gettime(); 
  ArrayReader *ar = new ArrayReader(argv[1], 
                                    1135132 + 8 + 29602*4, 
                                    "wetpos");
  Array *salt = new Array("salt", FLOAT);
  ar->Read(cut, 0, salt);
 
  cut->Bind(salt);
  cout << gettime() - secs << tab << "( bind salt ) " << endl;
  
 
  secs = gettime(); 
  Condition *saltnulls = new Condition("salt", 1, ">");
  cut = RestrictOp::Restrict(saltnulls, cut);
  cout << gettime() - secs << tab << "( restrict air cells )" << endl; 

  secs = gettime(); 
  cut->grid->normalize();
  cout << gettime() - secs << tab << "( normalize )" << endl; 
 
  //cut = ApplyOp::Apply(cut, "bottom", "z-b");
   
  vtkGridField *vtkgrid = vtkGridField::New();
  vtkgrid->UseNamedPerspective();
  vtkgrid->SetGridField(cut);

  /*
  vtkDataSetMapper *gridMapper = vtkDataSetMapper::New();
  gridMapper->SetInput( vtkgrid->GetOutput() );
  */

  vtkgrid->Update(); 
  vtkgrid->GetOutput()->GetPointData()->SetActiveScalars("salt");
  vtkDataArray *scalars = vtkgrid->GetOutput()->GetPointData()->GetScalars();  
  float *rng = scalars->GetRange();
  

  vtkContourFilter *iso = vtkContourFilter::New();
  iso->SetInput( vtkgrid->GetOutput() );
//  iso->UseScalarTreeOn();
  iso->SetValue(0,17);
//  iso->SetValue(1,7);
//  iso->SetValue(2,11);
//  iso->SetValue(3,32);
//  iso->SetValue(4,29);
//  iso->SetValue(5,3);
//  iso->GenerateValues(2,rng);
  

  secs = gettime(); 
  vtkExtractEdges *edges = vtkExtractEdges::New();
  edges->SetInput( vtkgrid->GetOutput() );
  //edges->Update();
  cout << gettime() - secs << tab << "( Extract Edges )" << endl; 
 
  rng[0] = 0; rng[1] = 34;
  vtkPolyDataMapper *polyMapper = vtkPolyDataMapper::New();
  polyMapper->SetScalarRange( rng[0], rng[1] );
  polyMapper->SetInput( iso->GetOutput() );
  
  vtkActor *gridActor = vtkActor::New();
  gridActor->SetMapper( polyMapper );

  //Render the bathymetry as a surface
  cutH->grid->normalize();
  vtkGridField *bath = vtkGridField::New();
  bath->UseNamedPerspective();
  bath->SetGridField(cutH);
  bath->Update();
  bath->GetOutput()->GetPointData()->SetActiveScalars("h");
  //bath->GetOutput()->PrintSelf(cout, 1);
  
  vtkWarpScalar *warp = vtkWarpScalar::New();
  warp->SetInput(bath->GetOutput());
  warp->UseNormalOn();
  warp->SetNormal(0.0, 0.0, 1.0);
  warp->SetScaleFactor(1);
  warp->Update();
  
  vtkLookupTable *lut = vtkLookupTable::New();
  lut->SetNumberOfColors(10);
  lut->SetHueRange(0.1, 0.1);
  lut->SetValueRange(0.3, 0.75);
  lut->SetSaturationRange(0.7, 0.2);
  lut->Build();
  
  vtkDataSetMapper *bathMapper = vtkDataSetMapper::New();
  bathMapper->SetInput(warp->GetOutput());
  bathMapper->SetLookupTable(lut);
  bathMapper->SetScalarRange(bath->GetOutput()->GetScalarRange());

  vtkActor *bathActor = vtkActor::New();
  bathActor->SetMapper(bathMapper);
   
  //-----

  // An outline to make the view more clear
  vtkOutlineFilter *outline = vtkOutlineFilter::New();
  outline->SetInput(vtkgrid->GetOutput());
  vtkPolyDataMapper *outlineMapper = vtkPolyDataMapper::New();
  outlineMapper->SetInput((vtkPolyData *)outline->GetOutput());
  vtkActor *outlineActor = vtkActor::New();
  outlineActor->SetMapper(outlineMapper); 
  
  vtkRenderer *ren1= vtkRenderer::New();
  ren1->AddActor( gridActor );
  ren1->AddActor( outlineActor );
  ren1->AddActor( bathActor );
  ren1->SetBackground( 0.1, 0.2, 0.4 );

  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer( ren1 );
  renWin->SetSize( 400, 400 );

  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);
  
  vtkMyCallback *callback = vtkMyCallback::New();
  iren->AddObserver("KeyPressEvent", (vtkCommand *) callback);
  
  iren->Initialize();
  renWin->Render();
  cout << gettime() - start << tab << "( Total )" << endl;
  iren->Start();
 
  
  vtkgrid->Delete();
  //gridMapper->Delete();
  polyMapper->Delete();
  gridActor->Delete();
  ren1->Delete();
  renWin->Delete();

  return 0;
}

void ShowCamera(vtkRenderWindow *renWin) {
  vtkCamera *cam = renWin->GetActiveCamera();
  stringstream ss;
  cam->PrintSelf(cout);
  /*
  vtkTextMapper *txt = vtkTextMapper::New();
  txt->SetInput(ss.c_str());
  vtkActor *tact = vtkActor::New();
  tact->SetMapper(txt);
  renWin->GetRenderer()->GetNextItem()->AddActor(txt);
  renWin->Render();

  */
}

void CaptureImage(vtkRenderWindow *renWin) {
  vtkWindowToImageFilter *w2i = vtkWindowToImageFilter::New();
  vtkPNGWriter *writer = vtkPNGWriter::New();
  w2i->SetInput(renWin);
  w2i->Update();
  writer->SetInput(w2i->GetOutput());
  writer->SetFileName("image.png");
  writer->Write();
}

Array *makeY(int size) {
  Array *arr;
  float data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = i%2*i*4-10;
  }
  arr = new Array("y", FLOAT);
  arr->copyData(data, size);
  return arr;  
}

Array *makeX(int size) {
  Array *arr;
  float data[size];
  int i;

  for (i=0; i<size; i++) {
      data[i] = i*5-20;
  }
  arr = new Array("x", FLOAT);
  arr->copyData(data, size);
  return arr;  
}

GridField *makeGeometry(int size) {

  Grid *G;
  GridField *GF;
  Array *x, *y;
  
  G = makeGrid(12, "geo");
  CellArray *cells = G->getKCells(0);
  x = makeX(cells->getsize());
  y = makeY(cells->getsize());

  GF = new GridField(G, 0);
  GF->Bind(x);
  GF->Bind(y);
  
  //printf("Valid? %i\n", !notValid(GF));
  //GF->print();

  return GF;
}

Grid *makeGrid(int scale, string name) {
  Grid *grid;
  CellArray *twocells;
  CellArray *onecells;

  Node triangle[3];
  Node segment[2];

  int i;
  twocells = new CellArray();
  for (i=0; i<scale-2; i++) {
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
