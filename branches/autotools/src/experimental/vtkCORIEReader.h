/*
 *
 * CORIE Reader object for translating CORIE grids and data into VTK
 * 
 * requires fort.14 (xy points and simplicial connections), 
 * fort.19 (depth values), and eventually fort.21 (bathymetry) 
 * and xxx.63/64 (scalar/vector data files)
 *
 */


#ifndef __vtkCORIEReader_h
#define __vtkCORIEReader_h

#define GRID_2D 0
#define SLICED_3D 1
#define GRID_3D 2

#include <stdio.h>
#include "vtkUnstructuredGridSource.h"
#include "vtkUnstructuredGrid.h"
#include "vtkIntArray.h"
#include "vtkCellArray.h"

class VTK_IO_EXPORT vtkCORIEReader : public vtkUnstructuredGridSource
{
    public:
        static vtkCORIEReader *New();
        vtkTypeMacro(vtkCORIEReader, vtkUnstructuredGridSource);
        void PrintSelf(ostream& os, vtkIndent indent);
            
        // Description:
        // Set/Get the CORIE data FileName (fort.21).
        vtkSetStringMacro(FileName);
        vtkGetStringMacro(FileName);

        vtkGetMacro(HorizontalGrid, vtkUnstructuredGrid*);
        
        //offset into timeseries; only one timestep is supported
        vtkGetMacro(TimeOffset, int);
        vtkSetMacro(TimeOffset, int);

        vtkSetMacro(OutputType, int);
        vtkGetMacro(OutputType, int);

        vtkSetMacro(ReadTimeStep, int);
        vtkGetMacro(ReadTimeStep, int);

        vtkSetMacro(ReadTimeStepCount, int);
        vtkGetMacro(ReadTimeStepCount, int);

        vtkSetMacro(CellType, int);
        vtkGetMacro(CellType, int);

        vtkGetMacro(MeanSeaLevel, float);
        
        void Update();
	vtkFloatArray *VerticalGrid;
     
    protected:
        vtkCORIEReader();
        ~vtkCORIEReader();
        
        void Execute();
        
        char *FileName;

        int OutputType;
       
        int ReadHeader(FILE *dataFp);  
        
        int ReadHGrid(FILE *hgridFp);

        int ReadVGrid(FILE *vgridFp);
       
        int ReadData(FILE *dataFp, 
                     vtkUnstructuredGrid *output);
        
        int Make2DGrid(vtkUnstructuredGrid *output);
        
        int Make3DSliced(vtkUnstructuredGrid *output);
        
        int Make3DGrid(vtkUnstructuredGrid *output);
        
	float MeanSeaLevel;
	vtkUnstructuredGrid *HorizontalGrid;
       
        int TimeOffset;
       
        //see ELCIRC user manual for meaning
        char *FormatDescription;
        char *ELCIRCVersion;
        char *StartTimeString;
        char *VariableName;
        char *VariableDescription;
        int NTimeSteps;
        float OutputTimeStep;
        int Skip;
        int Shape;
        int Dimension;
        int VerticalLayers;
        float VerticalStructure;

	int ReadTimeStep;	
	int ReadTimeStepCount;

	int CellType;   //vtk 3d cell type; wedge or tetrahedron
    private:
       
       //figure out the type and prepare to read the file
       FILE *SafeFileOpen(char *fileName, char *defaultFileName);

       vtkCORIEReader(const vtkCORIEReader&);
       void operator=(const vtkCORIEReader&);

       int BuildCell(int vNode, int *hNodes, int cellType, vtkCellArray *cells);
};

#endif
