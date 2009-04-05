%module core

%{
#include "object.h"
#include "unarynodemap.h"
#include "crossnodemap.h"
#include "abstractcellarray.h"
#include "cellarray.h"
#include "implicit0cells.h"
#include "cell.h"
#include "cellarray.h"
#include "grid.h"
#include "array.h"
#include "constarray.h"
#include "dataset.h"
#include "rankeddataset.h"
#include "gridfield.h"
#include "arrayreader.h"
#include "arraywriter.h"
#include "gridfieldoperator.h"
#include "bind.h"
#include "bindconstant.h"
#include "cross.h"
#include "restrict.h"
#include "apply.h"
#include "accumulate.h"
//#include "subapply.h"
#include "state.h"
#include "project.h"
#include "aggregate.h"
#include "aggregations.h"
#include "assignments.h"
#include "merge.h"
#include "output.h"
#include "tonetcdf.h"
#include "netcdfadaptor.h"
#include "stuebe.h"
#include "scan.h"
#include "scaninternal.h"
#include "tag.h"
#include "iterate.h"
#include "tuple.h"
#include "sift.h"
#include "onegrid.h"
#include "datadump.h"
#include "expr.h"
#include "util.h"

#include "elcircfile.h"
extern "C" { 
#include "elio.h"
}
%}

%include typemaps.i
%include std_string.i
%include "std_vector.i"

namespace std {
   %template(vectori) vector<int>;
   %template(vectord) vector<double>;
   %template(vectorS) vector<string>;
};

%exception {
  try {
    $function
  } catch(std::string e) {
    PyErr_SetString(PyExc_Exception, e.c_str());
    return NULL;
  }
}
#ifdef SWIGPYTHON

%typemap(in) int * {
  int i,sz;
                                                                               
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Expected a sequence");
    return NULL;
  }
                                                                               
  sz = PySequence_Size($input);
  $1 = ($1_ltype) malloc(sz*sizeof($*1_ltype));
  for (i = 0; i < sz; i++) {
    PyObject *o = PySequence_GetItem($input, i);
    if (!PyNumber_Check(o)) {
      PyErr_SetString(PyExc_TypeError, "Sequence elements must be numbers");
      return NULL;
    }
    $1[i] = ($*1_ltype) PyInt_AsLong(o);
  }
}

%typemap(in) float * {
  int i,sz;

  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Expected a sequence");
    return NULL;
  }

  sz = PySequence_Size($input);
  $1 = ($1_ltype) malloc(sz*sizeof($*1_ltype));
  for (i = 0; i < sz; i++) {
    PyObject *o = PySequence_GetItem($input, i);
    if (!PyNumber_Check(o)) {
      PyErr_SetString(PyExc_TypeError, "Sequence elements must be numbers");
      return NULL;
    }
    $1[i] = ($*1_ltype) PyFloat_AsDouble(o);
  }
}


%typemap(freearg) float *, int *, double * {
  if ($1) free($1);
}


#endif

%typecheck(SWIG_TYPECHECK_POINTER) float * {
  $1 = PySequence_Check($input) ? 1 : 0;
}

typedef short Dim_t;
%inline %{

float derefFloat(UnTypedPtr ptr) {
  return *(float *) ptr;
}

int derefInt(UnTypedPtr ptr) {
  return *(int *) ptr;
}

PyObject *derefPyObject(UnTypedPtr ptr) {
  PyObject *p =  *(PyObject **) ptr;
  Py_XINCREF(p);
  return p;
}
  
PyStringObject *derefPyString(UnTypedPtr ptr) {
  return *(PyStringObject **) ptr;
}
  
string derefString(UnTypedPtr ptr) {
  return *(std::string *) ptr;
}

PyObject *asPyObject(UnTypedPtr ptr) {
  PyObject *p =  (PyObject *) ptr;
  Py_XINCREF(p);
  return p;
}

PyStringObject *asPyString(UnTypedPtr p) {
  return (PyStringObject *) p;
}

UnTypedPtr asUnTypedPtr(PyObject *p) {
  return (UnTypedPtr) p;
}

%}

%typemap(python, in) UnTypedPtr {
  $1 = (UnTypedPtr) $input;
}

# any python object can be handled as an UnTypedPtr
# but ints and floats and such should have precedence
%typecheck(2000) UnTypedPtr {
  $1 = true;
}

%inline %{
void print_array(Node *array, int size) {
  for (int i=0; i<size; i++) {
    printf("%f\n", array[i]);
  }
}
%}

%inline %{
void testsign(unsigned int size) {
  printf("unsigned: %u\n", size);
}
%}
%include "std_string.i";
%rename(show) print;
%include "object.h";
%include "gridfieldoperator.h";
%include "rankeddataset.h";
%include "dataset.h";
%include "gridfield.h";
%include "array.h";
%include "constarray.h"

%include "cell.h";
%include "cellarray.h";
%include "grid.h";

%include "implicit0cells.h";

%include "arrayreader.h";
%include "arraywriter.h";
%include "elcircfile.h";

%include "cross.h";
%include "restrict.h";
%include "apply.h";
%include "bind.h";
%include "bindconstant.h";
%include "iterate.h";
%include "state.h";
%include "tag.h";
%include "datadump.h";
%include "output.h";
%include "stuebe.h";
%include "tonetcdf.h";
%include "netcdfadaptor.h";
%include "scan.h";
%include "scaninternal.h";
%feature("notabstract") Nodes;
%feature("notabstract") adjacentNodes;
%feature("notabstract") IncidentTo;
%feature("notabstract") pointInRange;
%feature("notabstract") setunion;
%feature("notabstract") Both;
%feature("notabstract") memberof;
%feature("notabstract") contains;
%feature("notabstract") fastcontainedby;
%feature("notabstract") match;
%feature("notabstract") adjacent;
%feature("notabstract") matchn;
%feature("notabstract") nearest;
%feature("notabstract") neighbors;
%feature("notabstract") sortedmatch;
%feature("notabstract") bypointer;
%feature("notabstract") byPointerSet;
%feature("notabstract") pointpoly;
%feature("notabstract") pointpoly2;
%feature("notabstract") cross;
%feature("notabstract") unify;
%feature("notabstract") ident;
%include "accumulate.h";
%include "aggregate.h";
%rename(_count) count;
%include "aggregations.h";
%include "assignments.h";
%template(interpolate1Dint) Aggregate::interpolate1D<int>;
%template(interpolate1Dfloat) Aggregate::interpolate1D<float>;
%template(avgint) Aggregate::_average<int>;
%template(avgfloat) Aggregate::_average<float>;
%template(sumint) Aggregate::_sum<int>;
%template(sumfloat) Aggregate::_sum<float>;
%template(trigradint) Aggregate::triGradient<int>;
%template(trigradfloat) Aggregate::triGradient<float>;
%template(grad3Dint) Aggregate::gradient3D<int>;
%template(grad3Dfloat) Aggregate::gradient3D<float>;
%template(gradint) Aggregate::gradient<int>;
%template(gradfloat) Aggregate::gradient<float>;
%template(minint) Aggregate::_min<int>;
%template(minfloat) Aggregate::_min<float>;
%template(maxint) Aggregate::_max<int>;
%template(maxfloat) Aggregate::_max<float>;
%template(intunion) Aggregate::setunion<int>;
%template(floatunion) Aggregate::setunion<float>;
%template(intmember) Assign::memberof<int>;
%template(floatmember) Assign::memberof<float>;
#%include "subapply.h";
%include "merge.h";
%include "sift.h";
%include "type.h";
%include "tuple.h";
%include "project.h";
%include "onegrid.h";
%include "expr.h";
%include "util.h";
%ignore ElioMsg;
%ignore ElioErr;
%ignore ElioSysErr;
%ignore ElioLineErr;
%include "elio.h";
