
#ifndef VISUALIZE_H
#define VISUALIZE_H

#include <string>
#include <vector>
#include "vtkRenderWindow.h"
#include "vtkCommand.h"
#include "vtkDataSet.h"
#include "vtkMapper.h"
#include "vtkObject.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkVRMLExporter.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkGridField.h"

void CaptureImage(vtkRenderWindow *, std::string filename);
void ExportVRML(vtkRenderWindow *);
void ShowCamera(vtkRenderWindow *renWin);

vtkGridField *toVTK(GridField *G, string active);

class vtkMyCallback : public vtkCommand {
  public:
  static vtkMyCallback *New() { return new vtkMyCallback; }

  virtual void Execute(vtkObject *caller, unsigned long, void *) {
    vtkRenderWindowInteractor *obj = 
            reinterpret_cast<vtkRenderWindowInteractor *>(caller);
    char *key = obj->GetKeySym();
    cout << key << endl;
    if (std::string(key) == "v") CaptureImage(obj->GetRenderWindow(),
                                              "image.png");
    if (std::string(key) == ">") this->Animate();
//    if (std::string(key) == "q") obj->;

    if(std::string(key) == "d") ExportVRML(obj->GetRenderWindow());
    
  }
  virtual void Animate() {
    // to be re-implemented in subclasses
    cout << "empty animation" << endl;
  }
};

const char ctab = '\t';
int ShowEdges(vtkDataSet *vtkgrid, vtkRenderWindow *renWin);
int Visualize(vtkDataSet *vtkgrid, vtkDataSet *bottom, std::string dataprod, vtkRenderWindow *renWin);
int DirectVis(vtkDataSet *vtkgrid, vtkDataSet *bottom, vtkRenderWindow *renWin, vtkMyCallback *callback);
int DirectVis(vtkDataSet *vtkgrid, vtkDataSet *bottom, vtkRenderWindow *renWin);
int DirectVis(vtkDataSet *vtkgrid, vtkRenderWindow *renWin, vtkMyCallback *callback);
int ViewTwo(vtkDataSet *vtkgrid, vtkDataSet *vtkgrid2, vtkRenderWindow *renWin);
int DirectVis(vtkDataSet *vtkgrid, vtkRenderWindow *renWin);
void ShowBathymetry(vtkDataSet *vtkgrid, 
                    std::vector<vtkMapper *> &mappers);
vtkRenderWindow *makeRenderWindow(std::vector<vtkMapper *> &mappers);
vtkRenderer *makeRenderer(std::vector<vtkMapper *> &mappers);
void RenderMappers(std::vector<vtkMapper *> &mappers, vtkRenderWindow *renWin, vtkMyCallback *callback);
void RenderMappersToFile(std::vector<vtkMapper *> &mappers, 
                         std::string filename, vtkRenderWindow *renWin);
float *ZoomBox(std::string region);

#endif //def VISUALIZE_H
