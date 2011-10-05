
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
#include "vtkPointData.h"
#include "vtkWedge.h"
#include "timing.h"
#include <string>

#define SALTLOW 0 
#define SALTHIGH 34

int main( int argc, char *argv[] )
{
  
  if (argc < 2) {
    cout << "supply a 63 file...\n";
    exit(1);
  }
  
  float start = gettime();
  float tpost = gettime(); 
  vtkCORIEReader *corie = vtkCORIEReader::New();
    corie->SetFileName(argv[1]);
    corie->SetReadTimeStep(0);
    corie->SetOutputType(GRID_3D);
    //corie->DebugOn();
    corie->Update();
  cout << gettime() - tpost << '\t' << "( Cross, Restrict, Bind )" << endl;


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
  calch->SetFunction("20*(4825.1-h)");
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
  std::string region("estuary");
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
    stretchz->Scale(1,1,20);
     
  vtkTransformFilter *stretchFilter = vtkTransformFilter::New();
  stretchFilter->SetTransform(stretchz);  
  stretchFilter->SetInput( (vtkPointSet *)subgrid->GetOutput());
  stretchFilter->Update();
  cout << gettime() - tpost << '\t' << "( stretch Z  )" << endl;
  
  cout << gettime() - start << '\t' << "( Total  )" << endl;
  
  vtkExtractGeometry *cutH = vtkExtractGeometry::New();
  cutH->SetInput( (vtkDataSet *) aa2->GetOutput());
  cutH->SetImplicitFunction(planesH);
  cutH->ExtractInsideOn();
  cutH->Update();
  
  // call the data product visualization function
  std::string dataprod;
  if (argc >=4) {
    dataprod = argv[3];
  } else {
    dataprod = "";
  } 
  Visualize(stretchFilter->GetOutput(), cutH->GetOutput(), dataprod);
}
