#include <map>
#include "visualize.h"
#include "vtkDataSetMapper.h"
#include "vtkRenderWindow.h"
#include "vtkXOpenGLRenderWindow.h"
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
#include "vtkRendererCollection.h"
//#include "vtkInteractorStyleRubberBandZoom.h"
#include <sstream>

vtkGridField *toVTK(GridField *G, string active) {
  // create a vtk object from the gridfield
  
  // normalize the node ids to prepare for vtk
  //float secs = gettime(); 
  G->GetGrid()->normalize();
//  cout << gettime() - secs << tab << "( normalize )" << endl; 
 
  //secs = gettime();
  vtkGridField *vtkgrid = vtkGridField::New(); 
  vtkgrid->UseNamedPerspective();
  vtkgrid->SetScalarAttribute(active.c_str());
  vtkgrid->SetGridField(G);
  vtkgrid->Update(); 
//  vtkgrid->GetOutput()->GetPointData()->SetActiveScalars(active.c_str());
 // cout << gettime() - secs << tab << "( to VTK )" << endl; 
  return vtkgrid;
}

int ShowEdges(vtkDataSet *vtkgrid, vtkRenderWindow *renWin) {
  double rng[2]; 
  vtkgrid->GetPointData()->GetScalars()->GetRange(rng);

  vtkExtractEdges *edges = vtkExtractEdges::New();
  edges->SetInput( vtkgrid );
  //edges->Update();
  
  vtkPolyDataMapper *polyMapper = vtkPolyDataMapper::New();
  polyMapper->SetScalarRange( rng[0], rng[1] );
//  polyMapper->SetScalarRange( 3, 34 );
  polyMapper->SetInput( edges->GetOutput() );
  
  std::vector<vtkMapper *> mappers;
  mappers.push_back(polyMapper);

  //ShowBathymetry(bottom, mappers);
  vtkMyCallback *callback = vtkMyCallback::New();
  
  RenderMappers(mappers, renWin, callback); 
}

int Visualize(vtkDataSet *vtkgrid, vtkDataSet *bottom, std::string dataprod, vtkRenderWindow *renWin) {

  if (dataprod == "edges") {
    ShowEdges(vtkgrid, renWin);
  } else {
  std::vector<vtkMapper *> mappers;
  
  double rng[2]; 
  vtkgrid->GetPointData()->GetScalars()->GetRange(rng);
//  rng[0] = 0; rng[1] = 34;
  //cout << "range: " << rng[0] << ", " << rng[1] << endl;
  
  vtkContourFilter *iso = vtkContourFilter::New();
  iso->SetInput( vtkgrid );
  iso->UseScalarTreeOn();
//  iso->SetValue(0,17);
//  iso->SetValue(1,7);
//  iso->SetValue(2,11);
//  iso->SetValue(3,31.9);
//  iso->SetValue(4,29);
//  iso->SetValue(5,3);
  float off = (rng[1] - rng[0])/8;
//  iso->GenerateValues(8, rng[0] + off, rng[1] - off);
  iso->GenerateValues(5, 7, 30);
  iso->Update();

  vtkPolyDataMapper *polyMapper = vtkPolyDataMapper::New();
  polyMapper->SetScalarRange( rng[0], rng[1] );
  polyMapper->SetScalarRange( 3, 34 );
  polyMapper->SetInput( iso->GetOutput() );
  
  mappers.push_back(polyMapper);

  ShowBathymetry(bottom, mappers);

  vtkMyCallback *callback = vtkMyCallback::New();
  RenderMappers(mappers, renWin, callback);
  //RenderMappersToFile(mappers, "scalar3d.png");
  }
  
  return 0;
}

void ShowBathymetry(vtkDataSet *bottom, std::vector<vtkMapper *> &mappers) {
  //Render the bathymetry as a surface
  
  vtkWarpScalar *warp = vtkWarpScalar::New();
  warp->SetInput((vtkPointSet *) bottom);
  warp->UseNormalOn();
  warp->SetNormal(0.0, 0.0, 1.0);
  warp->SetScaleFactor(1);
  warp->Update();
  
  vtkLookupTable *lut = vtkLookupTable::New();
  lut->SetNumberOfColors(10);
  lut->SetHueRange(0.1, 0.1);
  lut->SetValueRange(0.75, 0.3);
  lut->SetSaturationRange(0.2, 0.7);
  lut->Build();
  
  vtkDataSetMapper *bathMapper = vtkDataSetMapper::New();
  bathMapper->SetInput(warp->GetOutput());
  bathMapper->SetLookupTable(lut);
  bathMapper->SetScalarRange(bottom->GetScalarRange());
  mappers.push_back(bathMapper);

  // An outline to make the view more clear
  vtkOutlineFilter *outline = vtkOutlineFilter::New();
  outline->SetInput((vtkDataSet *) warp->GetOutput());
  vtkPolyDataMapper *outlineMapper = vtkPolyDataMapper::New();
  outlineMapper->SetInput((vtkPolyData *)outline->GetOutput());
  mappers.push_back(outlineMapper);
  
}

vtkRenderer *makeRenderer(std::vector<vtkMapper *> &mappers) {
  
  vtkActor *actor;
  vtkRenderer *ren= vtkRenderer::New();

  for (int i=0; i<mappers.size(); i++) {
    actor = vtkActor::New();
    actor->SetMapper(mappers[i]);
    ren->AddActor(actor);      
  }
  return ren;
}

vtkRenderWindow *makeRenderWindow( vtkRenderer *ren) {
 // static vtkRenderWindow *renWin = NULL;
 // if (renWin == NULL) renWin 
  
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer( ren );
  renWin->SetSize( 400, 400 );
  return renWin;
  
}

void RenderMappersToFile(std::vector<vtkMapper *> &mappers, 
                         std::string filename, vtkRenderWindow *renWin) {
  vtkRenderer *ren = makeRenderer( mappers );
  ren->SetBackground(1.0, 1.0, 1.0);
//  vtkXOpenGLRenderWindow *renWin = vtkXOpenGLRenderWindow::New();
  renWin->SetOffScreenRendering(1);
  renWin->AddRenderer( ren );
  renWin->SetSize( 400, 400 );
  //renWin->Render();
  CaptureImage(renWin, filename);
}

void RenderMappers(std::vector<vtkMapper *> &mappers, 
                   vtkRenderWindow *renWin,
                   vtkMyCallback *callback) {
  
  vtkRenderer *ren = makeRenderer( mappers );
  ren->SetBackground(0.0, 0.0, 0.0);
  renWin->AddRenderer( ren );
  renWin->SetSize( 400, 400 );
  
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);
  
  iren->AddObserver("KeyPressEvent", (vtkCommand *) callback);
  
  iren->Initialize();
  renWin->Render();
  iren->Start();
}

void CaptureImage(vtkRenderWindow *renWin, std::string filename) {
  vtkWindowToImageFilter *w2i = vtkWindowToImageFilter::New();
  vtkPNGWriter *writer = vtkPNGWriter::New();
  w2i->SetInput(renWin);
  w2i->Update();
  writer->SetInput(w2i->GetOutput());
  writer->SetFileName(filename.c_str());
  renWin->Render();
  writer->Write();
}

void ExportVRML(vtkRenderWindow *renWin) {
  vtkVRMLExporter *vrml = vtkVRMLExporter::New();
  vrml->SetRenderWindow(renWin);
  vrml->SetFileName("output.wrl");
  vrml->Write();
}

int ViewTwo(vtkDataSet *vtkgrid, vtkDataSet *vtkgrid2, 
              vtkRenderWindow *renWin) {
  std::vector<vtkMapper *> mappers;

  double rng[2]; 
  vtkgrid->GetPointData()->GetScalars()->GetRange(rng);
  
  vtkDataSetMapper *dsmapper = vtkDataSetMapper::New();
  dsmapper->SetInput(vtkgrid);
  dsmapper->SetScalarRange( rng[0], rng[1] );
  // dsmapper->SetScalarRange( 0, 32 );
  
  //vtkgrid->GetPointData()->GetScalars()->GetRange(rng);
  vtkDataSetMapper *dsmapper2 = vtkDataSetMapper::New();
  dsmapper2->SetInput(vtkgrid2);
  dsmapper2->SetScalarRange( rng[0], rng[1] );
  // dsmapper->SetScalarRange( 0, 32 );
  
  mappers.push_back(dsmapper);
  mappers.push_back(dsmapper2);

  std::stringstream ss;
  ss << vtkgrid->GetPointData()->GetScalars()->GetName() << ".png";
  //RenderMappersToFile(mappers, ss.str(), renWin, callback);
  vtkMyCallback *callback = vtkMyCallback::New();
   RenderMappers(mappers, renWin, callback);

}

int DirectVis(vtkDataSet *vtkgrid, 
              vtkRenderWindow *renWin,
              vtkMyCallback *callback) {
  std::vector<vtkMapper *> mappers;

  double rng[2]; 
  //vtkgrid->GetPointData()->GetScalars()->GetRange(rng);
  vtkgrid->GetScalarRange(rng);
  //cout << rng[0] << ", " << rng[1] << endl;
  
  vtkDataSetMapper *dsmapper = vtkDataSetMapper::New();
  dsmapper->SetInput(vtkgrid);
  dsmapper->SetScalarRange( rng[0], rng[1] );
  // dsmapper->SetScalarRange( 0, 32 );
  
  mappers.push_back(dsmapper);

  std::stringstream ss;
  //ss << vtkgrid->GetPointData()->GetScalars()->GetName() << ".png";
  //RenderMappersToFile(mappers, ss.str(), renWin, callback);
  RenderMappers(mappers, renWin, callback);

}

int DirectVis(vtkDataSet *vtkgrid, 
              vtkDataSet *bottom, 
              vtkRenderWindow *renWin) {
  vtkMyCallback *callback = vtkMyCallback::New();
  DirectVis(vtkgrid, bottom, renWin, callback);

}

int DirectVis(vtkDataSet *vtkgrid, 
              vtkRenderWindow *renWin) {
  vtkMyCallback *callback = vtkMyCallback::New();
  DirectVis(vtkgrid, renWin, callback);

}

int DirectVis(vtkDataSet *vtkgrid, 
              vtkDataSet *bottom, 
              vtkRenderWindow *renWin,
              vtkMyCallback *callback) {
  std::vector<vtkMapper *> mappers;

  double rng[2]; 
  vtkgrid->GetPointData()->GetScalars()->GetRange(rng);
  
  vtkDataSetMapper *dsmapper = vtkDataSetMapper::New();
  dsmapper->SetInput(vtkgrid);
  dsmapper->SetScalarRange( rng[0], rng[1] );
  
  mappers.push_back(dsmapper);

  //cout << rng[0] << ", " << rng[1] << " <-- Range" << endl;
  ShowBathymetry(bottom, mappers);

  RenderMappers(mappers, renWin, callback);
  //RenderMappersToFile(mappers, "directvis.png", renWin);
}


float *ZoomBox(std::string region) {

  float *bounds = new float[6];
  std::map<std::string, int> regions;

  regions["full"] = 0;
  regions["myzoom"] = 1;
  regions["far"] = 2;
  regions["plume"] = 3;
  regions["estuary"] = 4;
  regions["testgrid"] = 5;
  regions["debug"] = 6;
   
  int regionId = regions[region];
  switch (regionId) {
  case 0:
    bounds[0] = -84077;
    bounds[1] = 600373;
    bounds[2] = -966970;
    bounds[3] = 735114;
    bounds[4] = 3667;
    bounds[5] = 4829.2;
    break;
  
  case 1:
    
    bounds[0] = 320000;
    bounds[1] = 350000;
    bounds[2] = 280000;
    bounds[3] = 310000;
    bounds[4] = 1560;
    bounds[5] = 4890;
    //bounds[4] = -6000;
    //bounds[5] = 6000;

    
    /*
    bounds[0] = 330000;
    bounds[1] = 340000;
    bounds[2] = 288000;
    bounds[3] = 295000;
    bounds[4] = 1560;
    bounds[5] = 4890;
    */
    break;

  case 2:
    bounds[0] = -313981;
    bounds[1] = 2162419;
    bounds[2] = -995300;
    bounds[3] = 750432;
    bounds[4] = 4700;
    bounds[5] = 4866;
    break;

  case 3:
    bounds[0] = 63963;
    bounds[1] = 551997;
    bounds[2] = 110438;
    bounds[3] = 454337;
    bounds[4] = 4230;
    bounds[5] = 4866;
    break;

  case 4:
    bounds[0] = 309491;
    bounds[1] = 406391;
    bounds[2] = 249063;
    bounds[3] = 315714;
    bounds[4] = 3667;
    bounds[5] = 4835;
    break;

  case 5:
    bounds[0] = 330000;
    bounds[1] = 340000;
    bounds[2] = 290000;
    bounds[3] = 300000;
    bounds[4] = 4660;
    bounds[5] = 4800;
   /* 
    bounds[0] = 347000;
    bounds[1] = 355000;
    bounds[2] = 510000;
    bounds[3] = 520000;
    bounds[4] = 4560;
    bounds[5] = 4700;
    */
   
    bounds[0] = 335000;
    bounds[1] = 360000;
    bounds[2] = 500000;
    bounds[3] = 560000;
    bounds[4] = 4560;
    bounds[5] = 4860;
    
    break;
  
  case 6:
    bounds[0] = 386180;
    bounds[1] = 386700;
    bounds[2] = 285900;
    bounds[3] = 286600;
    bounds[4] = 4820;
    bounds[5] = 4828;
    break;
  }  
  
  return bounds;
}

void ShowCamera(vtkRenderWindow *renWin) {
  cout << "Showing Camera" << endl;
  vtkRendererCollection *coll = renWin->GetRenderers();
  vtkRenderer *ren = coll->GetNextItem();
  if (ren == NULL) cout << "renderer null" << endl;
  vtkCamera *cam = ren->GetActiveCamera();
  std::stringstream ss;
  cam->PrintSelf(cout, 0);
  /*
  vtkTextMapper *txt = vtkTextMapper::New();
  txt->SetInput(ss.c_str());
  vtkActor *tact = vtkActor::New();
  tact->SetMapper(txt);
  renWin->GetRenderer()->GetNextItem()->AddActor(txt);
  renWin->Render();

  */
}

