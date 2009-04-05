/*
 * CORIE Reader implementation
 *
 */

#include "vtkCORIEReader.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPointData.h"
#include "timing.h"
#include <string>

#define STARTTIME 0
#define STOPTIME 1
#define TYPE61 "61"
#define TYPE62 "62"
#define TYPE63 "63"
#define TYPE64 "64"

#define CELL_VERTEX_COUNT 3
#define VGRID_RECORD_SIZE 1


#define MAX(x, y) ((x)>(y)?(x):(y))
#define MIN(x, y) ((x)<(y)?(x):(y))
#define SORT(x, y, t) if (x>y) { t = x; x = y; y = t; }

using namespace std;

vtkCORIEReader* vtkCORIEReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCORIEReader");
     if(ret)
       {
       return (vtkCORIEReader*)ret;
       }
  // If the factory was unable to create the object, then create it here.
     return new vtkCORIEReader;    
}

vtkCORIEReader::vtkCORIEReader()
{
    this->FileName = NULL;

    this->FormatDescription = new char[48];
    this->ELCIRCVersion = new char[48];
    this->StartTimeString = new char[48];
    this->VariableName = new char[48];
    this->VariableDescription = new char[48];

    this->OutputType = GRID_3D;    

    this->NTimeSteps = -1;
    this->OutputTimeStep = -1;
    this->Skip = -1;
    this->Shape = 1;
    this->Dimension = 3;
    this->MeanSeaLevel = -1;

    this->ReadTimeStep = 0;
    this->ReadTimeStepCount = 1;

    this->CellType = VTK_WEDGE;
}

vtkCORIEReader::~vtkCORIEReader()
{
  if ( this->FileName )
    {
    delete [] this->FileName;
    }
  if ( this->FormatDescription )
    {
    delete [] this->FormatDescription;
    }
  if ( this->ELCIRCVersion )
    {
    delete [] this->ELCIRCVersion;
    }
  if ( this->StartTimeString )
    {
    delete [] this->StartTimeString;
    }
  if ( this->VariableName )
    {
    delete [] this->VariableName;
    }
  if ( this->VariableDescription )
    {
    delete [] this->VariableDescription;
    }
}

void vtkCORIEReader::Update()
{
  
  if ( this->GetOutput() )
    {
    this->GetOutput()->Update();
    }
}

void vtkCORIEReader::Execute()
{
   FILE *fp;
   vtkUnstructuredGrid *output = this->GetOutput();

   fp = SafeFileOpen(this->FileName, "CORIE output file v2");
   
   output->Initialize();
  
   if ( this->ReadHeader(fp) ) return;
   if ( this->ReadVGrid(fp) ) return;
   if ( this->ReadHGrid(fp) ) return;   

//   this->Print(cout);

   if ( this->Dimension == 3 ) 
     switch ( this->OutputType )
       {
       case SLICED_3D:
         {
         if ( this->Make3DSliced(output) ) return;
         break;
         }
       case GRID_3D:
	 {
	 if ( this->Make3DGrid(output) ) return;
	 break;
	 }
       default:
	 {
	 vtkErrorMacro("Unkown CORIE Output Type");
	 }
       }
     else
       if ( this->Make2DGrid(output) ) return;
       
    //bind the data to the grid.
    if ( ReadData(fp, output) ) return;

    //cout << "data loaded: cells: " << output->GetNumberOfCells() << "\n";
    float *bounds = output->GetBounds();
    //cout << "bounds: ";
//    for (int i=0; i<6; i++) cout << bounds[i] << "  ";
    //cout << "\n";
           
    if (this->Shape > 1) 
      {
      vtkDataArray *vecs = output->GetPointData()->GetVectors();
  //    cout << "tuples: " << vecs->GetNumberOfTuples() << "\n";
    //  cout << "components: " << vecs->GetNumberOfComponents() << "\n";
      float *xrange = new float[2];
      xrange = vecs->GetRange(0);
      float *yrange = new float[2];
      yrange = vecs->GetRange(1);
      //cout << "range: (" << xrange[0] << ", " << yrange[0] << "), (" << xrange[1] << ", " << yrange[1]  << ")\n";
      }
    else
      {
      vtkDataArray *scalars = output->GetPointData()->GetScalars();
//      cout << "tuples: " << scalars->GetNumberOfTuples() << "\n";
//      cout << "components: " << scalars->GetNumberOfComponents() << "\n";
      float *range = new float[2];
      range = scalars->GetRange();
//      cout << "range: (" << range[0] << ", " << range[1]  << ")\n";
      }
     
    return;
}

int vtkCORIEReader::ReadHeader(FILE *dataFp)
{
    fread(this->FormatDescription, sizeof(char), 48, dataFp);
    fread(this->ELCIRCVersion, sizeof(char), 48, dataFp);
    fread(this->StartTimeString, sizeof(char), 48, dataFp);
    fread(this->VariableName, sizeof(char), 48, dataFp);
    fread(this->VariableDescription, sizeof(char), 48, dataFp);

    //fread(headstring, sizeof(char), 205, dataFp);
    //for (int i=0;i<205;i++) 
    //  cout << headstring[i];
    //printf("\n");
    fread(&(this->NTimeSteps), sizeof(int), 1, dataFp);
    fread(&(this->OutputTimeStep), sizeof(float), 1, dataFp);
    fread(&(this->Skip), sizeof(int), 1, dataFp);
    fread(&(this->Shape), sizeof(int), 1, dataFp);
    fread(&(this->Dimension), sizeof(int), 1, dataFp);
    fread(&(this->VerticalStructure), sizeof(float), 1, dataFp);

    return 0;
}

int vtkCORIEReader::ReadVGrid(FILE *vgridFp)
{
    float vGridRecord[VGRID_RECORD_SIZE];
    int nVerticalLayers;

    this->VerticalLayers = nVerticalLayers;
    //parse fort.19 header
    vtkFloatArray *vgrid = vtkFloatArray::New();

    fread( &(this->MeanSeaLevel), sizeof(float), 1, vgridFp);
    fread( &(nVerticalLayers), sizeof(float), 1,  vgridFp); 
    int total = nVerticalLayers;
    //nVerticalLayers = 4;
    vgrid->SetNumberOfComponents( VGRID_RECORD_SIZE );
    vgrid->SetNumberOfTuples( nVerticalLayers );
    
    for (int i=0; i<nVerticalLayers; i++ )
      {
      fread( vGridRecord, sizeof(float), VGRID_RECORD_SIZE, vgridFp );
      vgrid->SetTuple( i, vGridRecord );
//      cout << i << ": " << vGridRecord[0] << "\n";
      }
//    cout << "\n";
//    cout << "layers: " << nVerticalLayers << "\n" << "msl: " << this->MeanSeaLevel << "\n";
    //cout << "vertical bounds: " << vgrid->GetRange(0)[0] << ", " << vgrid->GetRange(0)[1];
    this->VerticalGrid = vgrid;

    fseek(vgridFp, sizeof(float)*(total-nVerticalLayers), SEEK_CUR);
    return 0;
}


int vtkCORIEReader::ReadHGrid(FILE *hgridFp)
{
    int i, j, bottomIndex;
    int nNodes, nElements;
    int node[CELL_VERTEX_COUNT]; // 3; 2-d simplices
    float x, y, eta;

    vtkUnstructuredGrid *hgrid = vtkUnstructuredGrid::New();

    //parse header
    fread( &nNodes, sizeof(int), 1, hgridFp);
    fread( &nElements, sizeof(int), 1, hgridFp);
    int seekNodes = nNodes;
    int seekElements = nElements;
    //nNodes = 8;
    //nElements = 7;
    //Read points (nodes)
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints( nNodes );

    //the bathymetry is also in this section
    vtkFloatArray *bath = vtkFloatArray::New();
    bath->SetNumberOfComponents( 4 );
    bath->SetNumberOfTuples( nNodes );       
    bath->SetName("bot");

    for ( i=0; i<nNodes; i++ ) 
      {
      fread( &x, sizeof(float), 1, hgridFp);
      fread( &y, sizeof(float), 1, hgridFp);
      fread( &eta, sizeof(float), 1, hgridFp);
      fread( &bottomIndex, sizeof(int), 1, hgridFp);
      points->SetPoint(i, x, y, 0);      // 2d grid; no z
      //cout << "xy: " << x << ", " << y << "\n";
      //cout << "eta: " << eta << ", " << bottomIndex << "\n";
      bath->SetTuple4(i, eta, float(bottomIndex-1), x, y);
      }
    
    fseek(hgridFp, (sizeof(float)*3 + sizeof(int))*(seekNodes-nNodes), SEEK_CUR);

    //Read cells (elements)
    vtkCellArray *cells = vtkCellArray::New();
    for (i=0;i<nElements;i++)
      {

      fread(node, sizeof(int), CELL_VERTEX_COUNT, hgridFp);
      cells->InsertNextCell(CELL_VERTEX_COUNT);
      
      //cout << "cell: ";
      //our node ids are zero based
      for (j=0; j<CELL_VERTEX_COUNT; j++)
	{
	  //cout << node[j]-1 << " ";
	cells->InsertCellPoint(node[j]-1);
	}
      //cout << "\n";
      }

    fseek(hgridFp, (sizeof(int)*CELL_VERTEX_COUNT)*(seekElements-nElements), SEEK_CUR);
    //build an array of cell types
    int *cellTypes = new int[nElements];
    for (i=0; i<nElements; i++)
      {
      cellTypes[i] = VTK_TRIANGLE;
      }
   
    hgrid->SetPoints(points);
    hgrid->SetCells(cellTypes, cells);
    hgrid->GetPointData()->SetScalars(bath);

//    cout << "elements: " << hgrid->GetNumberOfCells() << "\n" << "nodes: " << hgrid->GetNumberOfPoints() << "\n";
    this->HorizontalGrid = hgrid;
   
    return 0;
}

int vtkCORIEReader::Make2DGrid( vtkUnstructuredGrid *output )
{
    output = this->HorizontalGrid;
    return 0;
}

int vtkCORIEReader::Make3DSliced( vtkUnstructuredGrid *output )
{
  //Not necessary yet, so not implemented.
    return 0;
}

int vtkCORIEReader::Make3DGrid( vtkUnstructuredGrid *output )
{
    // notation:
    // vId is an index into the vertical grid
    // hId is an index into the horizontal grid
    // gId is an index into the 3D grid

    int i,j,k;  // i for nodes, j for cells, k for depths
    float x,y,z;
    float coords[3];

    int horizontalPointCount = (int) this->HorizontalGrid->GetNumberOfPoints();
    int horizontalCellCount = (int) this->HorizontalGrid->GetNumberOfCells();
    int verticalPointCount = (int) this->VerticalGrid->GetNumberOfTuples();

//    int gIdTopIndex[horizontalPointCount];
    //int vIdBottomIndex[horizontalPointCount];
    //int cellvIdBottomIndex[horizontalPointCount];

    int next = -1;

    //compute the cross product of the xy values and z values

    //we got bathymetry information from the horizontal grid
    vtkFloatArray *bathymetry = (vtkFloatArray *) this->HorizontalGrid->GetPointData()->GetScalars();

    vtkPoints *points = vtkPoints::New();
    vtkIntArray *bottomdata = vtkIntArray::New();
    bottomdata->SetNumberOfComponents(2);
    bottomdata->SetNumberOfTuples(horizontalPointCount*verticalPointCount);
    bottomdata->SetName("bot");
    
//    cout << "building points...";
    for ( i=0;  i < horizontalPointCount; i++ ) 
      {
      this->HorizontalGrid->GetPoint(i, coords);
      x = coords[0];
      y = coords[1];
      //vIdBottomIndex[i] = (int) bathymetry->GetComponent(i, 1);

      //then (NVerticalLayers - bottomIndex + 1) full cells in the water
      for ( k=0; k<verticalPointCount; k++ )
        {
        this->VerticalGrid->GetTuple(k, &z);
	//cout << "xyz" << i << ": " << x << ", " << y << ", " << z << " ";
        
	next = points->InsertNextPoint( x, y, z );
	//cout << next;
	//cout << "\n";
	//getchar();
        //cout << bathymetry->GetComponent(i,1) << ", " << k << endl;
        bottomdata->SetComponent(i*verticalPointCount + k, 0, bathymetry->GetComponent(i,1));
        bottomdata->SetComponent(i*verticalPointCount + k, 1, k);
	}

      //gIdTopIndex[i] = next;  
      //cout << "column bottom index " << i << ": " << gIdTopIndex[i] << "\n";
      } 
//    cout << "done: " << points->GetNumberOfPoints()  << "\n";
      

    /*    
    // now compute the cross product of triangles and lines: wedges
    //                     /\~~~~~~\
    //  (current depth)   /__\______\   (next depth)
    */  
    vtkCellArray *cells = vtkCellArray::New();
    vtkIdType pts = (vtkIdType) CELL_VERTEX_COUNT;
    vtkIdType *vertices = new vtkIdType[CELL_VERTEX_COUNT]; 
    int cellType = this->GetCellType();
     
//    cout << "building cells...\n";
    for ( j=0; j < horizontalCellCount; j++ )
      {
      HorizontalGrid->GetCellPoints( j, pts, vertices );
      
      //cout << "min vertex id for cell " << j << "=" << cellvIdBottomIndex[j] << "\n";
      for (k=0; k<verticalPointCount-1; k++)
	{
	  
	cells->InsertNextCell(CELL_VERTEX_COUNT * 2);

	cells->InsertCellPoint( verticalPointCount*vertices[0] + k );
	cells->InsertCellPoint( verticalPointCount*vertices[1] + k );
	cells->InsertCellPoint( verticalPointCount*vertices[2] + k );

	cells->InsertCellPoint( verticalPointCount*vertices[0] + (k+1) );
	cells->InsertCellPoint( verticalPointCount*vertices[1] + (k+1) );
	cells->InsertCellPoint( verticalPointCount*vertices[2] + (k+1) );
	}
      
      }
//    cout << "done: " << cells->GetNumberOfCells() << "\n";

    //build an array of cell types
    vtkIdType *cellTypes = new vtkIdType[cells->GetNumberOfCells()];
    for (i=0; i < cells->GetNumberOfCells(); i++)
      {
      cellTypes[i] = cellType;
      }
      
    //now assign the points and cells to the unstructured grid output
   
    output->SetCells(cellTypes, cells);
    output->SetPoints(points);
    output->GetPointData()->AddArray(bottomdata);
    
    return 0;    
}

int vtkCORIEReader::ReadData(FILE *dataFp, vtkUnstructuredGrid *output)
{
  float timestamp;
    int timestep;
    int i, j, k;
    int next;
    float x,y;//,z; 
    int tsize;
    int validNodeCount = 0; 

    int shape = this->Shape;
    int bottomIndex;
    int nodeCount = output->GetNumberOfPoints();
    int horizontalNodeCount = this->HorizontalGrid->GetNumberOfPoints();
    int verticalNodeCount = this->VerticalGrid->GetNumberOfTuples();
//    float column[verticalNodeCount*shape];
    float val[shape];
    //float tuple[3];

    int *surfaceIndex = new int[horizontalNodeCount];

    vtkFloatArray *data = vtkFloatArray::New();
    data->SetNumberOfComponents(shape);  //only vectors with shape 3 allowed by vtk...
    vtkFloatArray *bath = (vtkFloatArray *) this->HorizontalGrid->GetPointData()->GetScalars();            
    //vtkFloatArray *zs = this->VerticalGrid;            

    tsize = 0;
    tsize = tsize + sizeof(float);  //timestamp
    tsize = tsize + sizeof(int);    //timestep
    tsize = tsize + (sizeof(float) * horizontalNodeCount); //surface indices

    for (i=0;i<horizontalNodeCount;i++) 
      {
      validNodeCount += verticalNodeCount - (int) bath->GetComponent(i,1) + 1; 
      }
    
    tsize = tsize + (sizeof(float) * validNodeCount * shape); //data

//    cout << "reading " << this->ReadTimeStepCount << " timesteps, " << nodeCount << " nodes.\n";
    //for each timestep...
    for (i=this->ReadTimeStep; 
            i<(this->ReadTimeStep + this->ReadTimeStepCount); 
            i++) 
      {

//      cout << "jumping to timestep " << i << "\n";
      //position the file at appropriate timestep 
      fseek(dataFp, i*tsize, SEEK_CUR);

      // there are some descriptive data in the file...
      // two values are included at the head of each
      // timestep

//      cout << "reading timestep header...";
      fread(&timestamp, sizeof(float), 1, dataFp); 
      fread(&timestep, sizeof(int), 1, dataFp); 
//      cout << "timestamp: " << timestamp << ", timestep:" << timestep << "\n";

//      cout << "reading surface indices...\n";
      //read in the surface node index
      for (int p = 0; p<horizontalNodeCount; p++) 
        {
        fread(&(surfaceIndex[p]), sizeof(int), 1, dataFp);
        //   for (i=0;i<horizontalNodeCount;i++) 
	    //cout << surfaceIndex[p] << " ";
        //if (i % 20 == 0) getchar();
        }
    
//      cout << "done.\n";
 
//      cout << "reading data...";
      // for each node...
      for ( j=0; j<horizontalNodeCount; j++ ) 
        {
	    //cout << ".";
	    //cout << "reading bottom index...";
        //get bathymetry information for this node
    	bottomIndex = (int) bath->GetComponent(j,1); 
    	x = (float) bath->GetComponent(j,2); 
    	y = (float) bath->GetComponent(j,3); 
//	cout << bottomIndex << ", " << surfaceIndex[j] << "\n";
//        getchar();

   	    // for each vertical layer...
        // read a (water) column of vectors of size shape and type float

        for (k=0; k<verticalNodeCount; k++) {
          if (k>=bottomIndex) {
//          fread(column, sizeof(float), (verticalNodeCount-bottomIndex+1)*shape, dataFp);
            fread(val, sizeof(float), shape, dataFp);
            /*
            if ( x >= 320000 && x <= 350000 && y<= 310000 && y>= 280000 ) {
              z = zs->GetComponent(k,0);
              cout << x << ", " << y << ", " << z << ", " << bottomIndex << ", " << k << ", " << val[0] << endl;
              getchar();
            }
            */
          } else {
            for (int x=0; x<shape; x++) {
              val[x] = 99999;
            }
          }
          next = data->InsertNextTuple(val);
        }
   
        /*
        for (int v=bottomIndex; v<=verticalNodeCount; v++)
            cout << v-bottomIndex << column[v-bottomIndex] << " ";
        cout << "\n\n";
        */
//        for ( k=MAX(surfaceIndex[j],bottomIndex)-bottomIndex;k<=verticalNodeCount; k++ ) 
//          {
          //vtk wants only vectors with 3 components...
//          tuple[0] = column[k*shape];
//          tuple[1] = column[k*shape+1];
//          tuple[2] = 0;
//	      next = data->InsertNextTuple(tuple);
      	  //cout << "\n" << next  << ": (" << data->GetTuple(next)[0] << ", " << data->GetTuple(next)[1];
	    //cout << "(" << column[k*shape] << ", " << column[k*shape+1] << ")";
//          }
	//getchar();
	      
        } 
       
      }
   
//    cout << "done: " << data->GetNumberOfTuples() << "\n";
    //vtkDataSetAttributes *attributeData;
    //attributeData = (vtkDataSetAttributes *)output->GetPointData()
    //cout << "dim: " << dim << "\n";
   
    data->SetName("salt"); 
    if ( shape > 1 )
      output->GetPointData()->SetVectors(data);
    else
      output->GetPointData()->SetScalars(data);
    
    return 0;
}

void vtkCORIEReader::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkUnstructuredGridSource::PrintSelf(os,indent);

    os << indent << "File Name: " <<
       (this->FileName ? this->FileName : "(none)") << "\n";
    
    os << indent << "Format Description: " << 
      (this->FormatDescription ? this->FormatDescription : "(none)") << "\n";

    os << indent << "ELCIRC Version: " << 
      (this->ELCIRCVersion ? this->ELCIRCVersion : "(none)") << "\n";

    os << indent << "Format Description: " << 
      (this->StartTimeString ? this->StartTimeString : "(none)") << "\n";
    
    os << indent << "Variable Name: " << 
      (this->VariableName ? this->VariableName : "(none)") << "\n";

    os << indent << "Variable Dimenison: " << 
      (this->VariableDescription ? this->VariableDescription : "(none)") << "\n";

    os << indent << "Number of Time Steps: " << 
      (this->NTimeSteps) << "\n";

    os << indent << "OutputTimeSteps: " << 
      (this->OutputTimeStep) << "\n";

    os << indent << "Skip: " << 
      (this->Skip) << "\n";

    os << indent << "Shape: " << 
      (this->Shape) << "\n";

    os << indent << "Dimension: " << 
      (this->Dimension) << "\n";

    os << indent << "VerticalStructure: " << 
      (this->VerticalStructure) << "\n";    
}

int vtkCORIEReader::BuildCell(int vNode, int *hNodes, int cellType, vtkCellArray *cells)
{

  //cout << "building cell...";
  switch (cellType)
    {
    case VTK_WEDGE:
      
      cells->InsertNextCell(CELL_VERTEX_COUNT * 2);

      //3 node ids form the triangle at the current depth
      //3 node other ids form the triangle at the next depth
      cells->InsertCellPoint( hNodes[0] - vNode );
      cells->InsertCellPoint( hNodes[1] - vNode );
      cells->InsertCellPoint( hNodes[2] - vNode );
   
      cells->InsertCellPoint( hNodes[0] - (vNode+1) );
      cells->InsertCellPoint( hNodes[1] - (vNode+1) );
      cells->InsertCellPoint( hNodes[2] - (vNode+1) );

      break;
      
    case VTK_TETRA:
      {
      int i, j, tmp, base[3];
      
      //sort the base nodes
      for (i=0; i<3; i++)
	{
	base[i] = hNodes[i] - vNode;
	}

      for (i=0; i<3; i++)
	{
	for (j=0; j<3; j++) SORT(base[i], base[j], tmp);
	}

      cells->InsertNextCell( 4 );
      cells->InsertCellPoint( base[0] );
      cells->InsertCellPoint( base[0] - 1 );
      cells->InsertCellPoint( base[1] - 1 );   
      cells->InsertCellPoint( base[2] - 1 );
      
      cells->InsertNextCell( 4 );
      cells->InsertCellPoint( base[0] );
      cells->InsertCellPoint( base[1] );
      cells->InsertCellPoint( base[1] - 1 );   
      cells->InsertCellPoint( base[2] - 1 );

      cells->InsertNextCell( 4 );
      cells->InsertCellPoint( base[0] );
      cells->InsertCellPoint( base[1] );
      cells->InsertCellPoint( base[2] );   
      cells->InsertCellPoint( base[2] - 1 );
      
      break;
      }
    default:
      cout << "Bad Cell Type\n";
    }
    /*
    cout << " " << hNodes[0] - vNode;
    cout << " " << hNodes[1] - vNode;
    cout << " " << hNodes[2] - vNode;
    cout << " " << hNodes[0] - (vNode+1);
    cout << " " << hNodes[1] - (vNode+1);
    cout << " " << hNodes[2] - (vNode+1);
    cout << "\n";
    */

    return 0;
}

FILE *vtkCORIEReader::SafeFileOpen(char *fileName, char *defaultFileName)
{
    FILE *fp;
    
    if ( fileName == NULL )
      {
      vtkErrorMacro(<< "Must specify " << defaultFileName <<" file");
      return NULL;
      }
    
    if ( (fp = fopen(fileName, "r")) == NULL)
      {
      vtkErrorMacro(<< "File: " << fileName << " not found");
      return fp;
      }

    fclose(fp);
    fp = fopen(fileName, "rb");

    return fp;
}
