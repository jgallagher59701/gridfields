%module gfvis

%{
#include <string>
#include "cellarray.h"
#include "grid.h"
#include "array.h"
#include "gridfieldoperator.h"
#include "gridfield.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointSet.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridSource.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkWriter.h"
#include "vtkPoints.h"
#include "vtkDataWriter.h"
#include "vtkGridField.h"
#include "vtkPythonUtil.h"
#include "vtkObjectBase.h"
#include "vtkObject.h"
%}

/* These are necessary to prevent syntax erors by the SWIG preprocessor */
#define VTK_FILTERING_EXPORT
#define VTK_COMMON_EXPORT
#define VTK_IO_EXPORT

PyObject *vtkPythonObject(vtkUnstructuredGrid *ptr);

%{
PyObject *vtkPythonObject(vtkUnstructuredGrid *ptr) {
  return vtkPythonGetObjectFromPointer(ptr);
}
%}

%exception {
  try {
    $function
  } catch(std::string e) {
    PyErr_SetString(PyExc_Exception, e.c_str());
    return NULL;
  }
}

typedef short Dim_t;
%rename(output) print;

%include "vtkSetGet.h"
%include "vtkSystemIncludes.h"
%include "vtkDataArray.h"
%include "vtkDataSet.h"
%include "vtkDataSetAttributes.h"
%include "vtkPointSet.h"
%include "vtkPoints.h"
%include "vtkPointData.h"
%include "vtkUnstructuredGrid.h"
%include "vtkUnstructuredGridSource.h"
%include "vtkWriter.h"
%include "vtkDataWriter.h"
%include "vtkUnstructuredGridWriter.h"
%include "vtkGridField.h"

