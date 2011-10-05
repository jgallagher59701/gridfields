
#ifndef DATAPROD_H
#define DATAPROD_H
#include <string>
#include "visualize.h"

class GridField;
class vtkGridField;
class vtkDataSetMapper;
class vtkRenderer;
class vtkActor;
class vtkRenderWindowInteractor;

void HorizontalSlice(GridField *H, GridField *V);
#endif
