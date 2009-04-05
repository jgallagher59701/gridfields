
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

int SurfaceAnimation( GridField *H, GridField *V, const char *filename, 
              int addr, string region);

int scalar3D( GridField *H, GridField *V, const char *filename, 
              int addr, string region, string dataprod);

void HorizontalSlice(GridField *H, GridField *V, const char *filename, 
                    int addr, string region, int depth);

void VerticalSlice( GridField *H, GridField *V, const char *filename, 
                    int addr, string region);

void InterpolateVertSlice(GridField *H, GridField *V, const char *filename,
                     int offset, string region); 
void BadInterpolateVertSlice(GridField *H, GridField *V, const char *filename,
                     int offset, string region); 

class SurfaceAnimator : public vtkMyCallback {

  public:
    
  SurfaceAnimator(GridFieldOperator *_bindsurf, 
                       GridFieldOperator *_bindvar,
                       GridFieldOperator *_getmag,
                       ElcircFile *_ef,
                       ElcircFile *_surfef
                       ) {
    this->bindsurf = _bindsurf;
    this->bindvar = _bindvar;
    this->getmag = _getmag;
    this->ef = _ef;
    this->surfef = _surfef;
  }
//  void Initialize(vtkGridField *vtkgrid, vtkGridField *bath);
  void Initialize(vtkGridField *vtkgrid);
  void Animate();
   private:
    GridFieldOperator *bindsurf;
    GridFieldOperator *bindvar;
    GridFieldOperator *getmag;
    ElcircFile *ef;
    ElcircFile *surfef;
    vtkDataSetMapper *dsmapper;
    vtkRenderer *ren;
    vtkActor *actor;
    vtkRenderWindow *renWin;
    vtkRenderWindowInteractor *iren;
};
#endif
