
#include "vtkCORIEReader.h"
#include "visualize.h"
#include "timing.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkLODActor.h"
#include "vtkTransformFilter.h"
#include "vtkExtractGeometry.h"
#include "vtkContourFilter.h"
#include "vtkExtractEdges.h"
#include "vtkCutter.h"
#include "vtkFloatArray.h"
#include "vtkPlanes.h"
#include "vtkTransform.h"
#include "vtkOutlineFilter.h"
#include "vtkPoints.h"
#include "vtkCubeAxesActor2D.h"
#include "vtkCellCenters.h"
#include "vtkArrayCalculator.h"
#include "vtkAssignAttribute.h"
#include "vtkDataSetAttributes.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkLabeledDataMapper.h"
#include "vtkActor2D.h"
#include "vtkIdFilter.h"
#include "vtkThreshold.h"
#include "vtkPointDataToCellData.h"
#include "vtkCellDataToPointData.h"
#include "vtkDataSet.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkStructuredGrid.h"
#include "vtkImplicitDataSet.h"
#include "vtkWedge.h"
#include "vtkPlane.h"
#include "timing.h"
#include <string>
#include "corierecipes.h"
#include "gridfield.h"
#include "array.h"
#include "vtkGridField.h"
#include <sstream>

#define SALTLOW 0 
#define SALTHIGH 34

int main( int argc, char *argv[] )
{
  cout << setprecision(3);
  cout.setf(ios::fixed);
  cout << endl;
  if (argc < 2) {
    cout << "supply a 63 file...\n";
    exit(1);
  }

  string stations;
  if (argc < 3) {
    stations = "../reference/vslice_mchann.bpclean";
  } else {
    stations = string(argv[2]);
  }
  
  float vscale = 100;
  float start = gettime();
  float tpost = gettime(); 
  vtkCORIEReader *corie = vtkCORIEReader::New();
    corie->SetFileName(argv[1]);
    corie->SetReadTimeStep(0);
    corie->SetOutputType(GRID_3D);
    //corie->DebugOn();
    corie->Update();
  cout << gettime() - tpost << '\t' << "( Read and Construct 3d Grid )" << endl;

  tpost = gettime(); 
  vtkArrayCalculator *calc = vtkArrayCalculator::New();
  calc->SetInput((vtkDataSet *)corie->GetOutput());
  calc->SetAttributeModeToUsePointData();
  calc->AddScalarVariable("b", "bot", 0);
  calc->AddScalarVariable("zpos", "bot", 1);
  calc->SetFunction("zpos - b");
  calc->SetResultArrayName("bathymetry");
  calc->Update();
  cout << gettime() - tpost << '\t' << "( compute bathymetry )" << endl;

  vtkAssignAttribute *aa = vtkAssignAttribute::New();
  aa->SetInput(calc->GetOutput());
  aa->Assign("bathymetry", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
  aa->Update();
  /* 
  cout << "scalar range:" << endl;
  cout << aa->GetOutput()->GetPointData()->GetScalars()->GetRange()[1] << endl;
  cout << aa->GetOutput()->GetPointData()->GetScalars()->GetRange()[0] << endl;
  */
  tpost = gettime(); 
  vtkThreshold *wetgrid = vtkThreshold::New();
  wetgrid->SetInput(aa->GetOutput());
  wetgrid->SetAttributeModeToUsePointData();
  wetgrid->ThresholdByUpper(0);
  wetgrid->Update();
  cout << gettime() - tpost << '\t' << "( river bottom )" << endl;
  
  tpost = gettime(); 
  vtkArrayCalculator *calch = vtkArrayCalculator::New();
  calch->SetInput((vtkDataSet *)corie->GetHorizontalGrid());
  calch->SetAttributeModeToUsePointData();
  calch->AddScalarVariable("h", "bot", 0);

  stringstream ss;
  ss << vscale << "*(4825.1-h)";

  calch->SetFunction(ss.str().c_str());
  calch->SetResultArrayName("eta");
  calch->Update();
  cout << gettime() - tpost << '\t' << "( compute bottom z )" << endl;
  
  vtkAssignAttribute *aa2 = vtkAssignAttribute::New();
  aa2->SetInput(calch->GetOutput());
  aa2->Assign("eta", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
  aa2->Update();
 
  vtkAssignAttribute *aa1 = vtkAssignAttribute::New();
  aa1->SetInput((vtkDataSet *) wetgrid->GetOutput());
  aa1->Assign("salt", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
  aa1->Update();
 

  // create the conditions by which to zoom
  std::string region("myzoom");
  if (argc >= 3) region = argv[2];

  float *bounds = ZoomBox(region);
  float *boundsH = ZoomBox(region);
  boundsH[4] = -1;
  boundsH[5] = 1;
  
  vtkPlanes *planesH = vtkPlanes::New();
  planesH->SetBounds(boundsH);
  
  vtkPlanes *planes = vtkPlanes::New();
  planes->SetBounds(bounds);
  
  tpost = gettime(); 
  vtkExtractGeometry *subgrid = vtkExtractGeometry::New();
  subgrid->SetInput( (vtkDataSet *) aa1->GetOutput());
  subgrid->SetImplicitFunction(planes);
  subgrid->ExtractInsideOn();
  subgrid->Update();
  cout << gettime() - tpost << '\t' << "( Zoom Data )" << endl;

  tpost = gettime();      
  vtkTransform *stretchz = vtkTransform::New();
    stretchz->Scale(1,1,vscale);
     
  vtkTransformFilter *stretchFilter = vtkTransformFilter::New();
  stretchFilter->SetTransform(stretchz);  
  stretchFilter->SetInput( (vtkPointSet *)subgrid->GetOutput());
  stretchFilter->Update();
  cout << gettime() - tpost << '\t' << "( stretch Z  )" << endl;

  tpost = gettime();      
  GridField *Pn = readCleanStations("../reference/vslice_mchann.bpclean", 97);
  //GridField *Pn = readCleanStations(stations, 97);
  Array *x_arr = Pn->getAttribute("x");
  Array *y_arr = Pn->getAttribute("y");
  float *xs;
  x_arr->getData(xs);
  float *ys; 
  y_arr->getData(ys);
  cout << gettime() - tpost << '\t' << "( Read User Stations )" << endl;
  
  vtkFloatArray *V = corie->VerticalGrid;

  int lowest = 15;
  int i,j,k;
  int dim[3];
  dim[0] = x_arr->size;
  dim[1] = V->GetSize() - lowest;
  
  vtkStructuredGrid *P = vtkStructuredGrid::New();
  P->SetDimensions(dim[0], 1, dim[1]);
  vtkPoints *pts = vtkPoints::New();
  pts->Allocate(dim[1]*dim[0]);

  tpost = gettime();      
  for (k=lowest; k<V->GetSize(); k++) {
    //cout << V->GetValue(k) << endl;
    for (j=0; j<dim[0]; j++) {
      pts->InsertNextPoint(xs[j], ys[j], V->GetValue(k));
    }
  }
  P->SetPoints(pts);
  cout << gettime() - tpost << '\t' << "( Build User Grid )" << endl;

  tpost = gettime();      
  vtkImplicitDataSet *gridfunc = vtkImplicitDataSet::New();
  gridfunc->SetDataSet(stretchFilter->GetOutput());
  gridfunc->SetOutValue(-99);
  
  vtkFloatArray *salt = vtkFloatArray::New();
 
  //cout << "pts: " <<  P->GetNumberOfPoints() << endl;
  
  vtkTransformFilter *stretchP = vtkTransformFilter::New();
  stretchP->SetTransform(stretchz);  
  stretchP->SetInput( (vtkPointSet *)P);
  stretchP->Update();
  
  vtkUnstructuredGrid *uP = (vtkUnstructuredGrid *) stretchP->GetOutput();
  cout << gettime() - tpost << '\t' << "( Adjust User Grid )" << endl;

  tpost = gettime();      
  float x[3];
  float v;
  for (i=0; i<uP->GetNumberOfPoints(); i++) {
    uP->GetPoint(i, x);
    v = gridfunc->EvaluateFunction(x);
    salt->InsertNextValue(v);
  }
  cout << gettime() - tpost << '\t' << "( Find Cells )" << endl;
  
  uP->GetPointData()->SetScalars(salt);
  
  vtkThreshold *wetgrid2 = vtkThreshold::New();
  wetgrid2->SetInput(uP);
  wetgrid2->SetAttributeModeToUsePointData();
  wetgrid2->ThresholdByUpper(0);
  wetgrid2->Update();
 
  cout << gettime() - start << '\t' << "( Total )" << endl;
  
  vtkTransform *noy = vtkTransform::New();
    noy->Scale(1,0,1);
  vtkTransformFilter *projecty = vtkTransformFilter::New();
  projecty->SetTransform(noy);  
  projecty->SetInput( (vtkPointSet *)wetgrid2->GetOutput());
  projecty->Update();
  
  vtkExtractGeometry *cutH = vtkExtractGeometry::New();
  cutH->SetInput( (vtkDataSet *) aa2->GetOutput());
  cutH->SetImplicitFunction(planesH);
  cutH->ExtractInsideOn();
  cutH->Update();

  //  DirectVis((vtkDataSet *) P, cutH->GetOutput());
  DirectVis((vtkDataSet *) wetgrid2->GetOutput());
  //ShowEdges(uP);
//  DirectVis(uP, cutH->GetOutput());
}
