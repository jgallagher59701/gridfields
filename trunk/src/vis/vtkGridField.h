#ifndef _VTKGRIDFIELD_H
#define _VTKGRIDFIELD_H


#include "gridfield.h"
#include "vtkUnstructuredGridSource.h"
#include "vtkUnstructuredGrid.h"
#include "vtkObjectFactory.h"

#define MAX_VTK_ATTRS 5

typedef enum _Perspective {NAMED, UNNAMED} Perspective;

class vtkGridField : public vtkUnstructuredGridSource {
 public:
  static vtkGridField *New();
  ~vtkGridField();
  
  static void UpdateScalars(vtkUnstructuredGrid *ug, GridField *gf, Dim_t k, char *attr);

  vtkTypeMacro(vtkGridField, vtkUnstructuredGridSource);
  //void PrintSelf(ostream& os, vtkIndent indent);

  void Update();

  /************************/
  //  static vtkUnstructuredGrid *Convert(GridField *);
  void Convert(vtkUnstructuredGrid *vtkgrid);
  void SetGridField(GridField *GF) { gf = GF; };
  /************************/

  vtkUnstructuredGrid *MakeGrid();

  bool NamedPerspective() { return (perspective == NAMED); }
  void UseNamedPerspective() { perspective = NAMED; }
  void UseUnnamedPerspective() { perspective = UNNAMED; }

  void SetScalarAttribute(string attr) { activeAttribute = attr; cout << activeAttribute;};
  void SetScalarAttribute(char *attr) { activeAttribute = string(attr); };

  void SetVectorAttributes(string u, string v) {
      vectorAttribute1 = u;
      vectorAttribute2 = v; 
      vectorAttribute3 = ""; 
  };

  void SetVectorAttributes(const char *u, const char *v) {
      vectorAttribute1 = string(u);
      vectorAttribute2 = string(v);
      vectorAttribute3 = "";
  };


  void SetVectorAttributes(string u, string v, string w) {
      vectorAttribute1 = u;
      vectorAttribute2 = v; 
      vectorAttribute3 = w; 
  };
  
  void SetVectorAttributes(const char *u, const char *v, const char *w) {
      vectorAttribute1 = string(u);
      vectorAttribute2 = string(v);
      vectorAttribute3 = string(w);
  }
  
  int guessType(Cell *c, int dim);
  void repairCell(Cell *c, int type);

 protected:
  
  void Execute();


 private:
  string activeAttribute;
  string vectorAttribute1;
  string vectorAttribute2;
  string vectorAttribute3;

  Perspective perspective;
  
  GridField *gf;
  void sortby(Cell &c, GridField *gf, string attr);
  void reorderNodes(Cell &c, vector<int> &order);
  void ValidateForConversion();
  vtkDataArray *MakeDataArray(GridField *gf, Dim_t k, string attr);
  vtkDataArray *MakeVectorArray(GridField *gfpts, Dim_t k,
                                              string u, 
                                              string v,
					      string w);

  vtkDataArray *CopyDataArray(GridField *gf, Dim_t k, string attr);
};

#endif
