#include <iostream>
#include "expr.h"
#include "tuple.h"
#include <string>
#include "constarray.h"

using namespace std;

#define DEBUG 0

ConstArray::ConstArray(string nm, int sz, Type t, UnTypedPtr constval) 
  : Array(nm.c_str(), t) 
{
  _size = sz;
  setConst(constval);
}

void ConstArray::setConst(UnTypedPtr constval) {
  switch (type) {
    case INT:
      int_constant = *(int*) constval; 
      float_constant = 0;
      obj_constant = 0;
      break;
    case FLOAT:
      int_constant = 0;
      float_constant = *(float*) constval; 
      obj_constant = 0;
      break;
    case OBJ:
      int_constant = 0;
      float_constant = 0;
      obj_constant = constval; 
      break;
    default:
      cout << "Unkown type" << endl;
  }
}

ConstArray::ConstArray(string nm, int sz, float constval) 
  : float_constant(constval), int_constant(0), obj_constant(0),
    Array(nm.c_str(), FLOAT) 
{
  _size = sz;
}

ConstArray::ConstArray(string nm, int sz, int constval) 
  : float_constant(0.0), int_constant(constval), obj_constant(0),
    Array(nm.c_str(), INT) 
{ 
  _size = sz;
}

ConstArray::ConstArray(string nm, int sz, UnTypedPtr constval) 
  : float_constant(0.0), int_constant(0), obj_constant(constval),
    Array(nm.c_str(), OBJ) 
{
  _size = sz;
}

void ConstArray::getData(void **&out) {
  cout << "can't get data from a ConstArray." << endl;
}
void ConstArray::getData(float *&out) {
  cout << "can't get data from a ConstArray." << endl;
}
void ConstArray::getData(int *&out) {
  cout << "can't get data from a ConstArray." << endl;
}

void ConstArray::copyData(int *data, int s) {
  cout << "can't copy data into a ConstArray." << endl;
}

void ConstArray::shareData(int *data, int s) {
  cout << "can't share data with a ConstArray." << endl;
}

void ConstArray::copyData(float *data, int s) {
  cout << "can't copy data into a ConstArray." << endl;
}

void ConstArray::shareData(float *data, int s) {
  cout << "can't share data with a ConstArray." << endl;
}

void ConstArray::copyData(UnTypedPtr *data, int s) {
  cout << "can't copy data into a ConstArray." << endl;
}

void ConstArray::shareData(void **data, int size) {
  cout << "can't share data with a ConstArray." << endl;
}

ConstArray *ConstArray::copy() { return resize(_size); }

void ConstArray::setVals(UnTypedPtr vals, int size) {
  cout << "can't set Vals of a ConstArray." << endl;
}

ConstArray *ConstArray::copyAndFilter(bool *filter) {
  ConstArray *arr = this;
  ConstArray *newarr;
  int i;
  int newsize=0;

  if (filter == NULL) {
    newarr = arr->copy();
  } else {

    for (i=0; i<arr->_size; i++) {
      if (filter[i]) { newsize++; }
    }

    newarr = resize(newsize);
  }
  return newarr;

};

ConstArray *ConstArray::resize(int newsize) {
  switch (type) {
    case INT:
      return new ConstArray(this->getName(), newsize, int_constant); 
    case FLOAT:
      return new ConstArray(this->getName(), newsize, float_constant);
    case OBJ:
      return new ConstArray(this->getName(), newsize, obj_constant);
    default:
      cout << "Unkown type" << endl;
  }
};

long ConstArray::getConst() {
  switch (type) {
    case INT:
      return int_constant; 
      break;
    case FLOAT:
      return long(float_constant);
      break;
    case OBJ:
      return long(obj_constant);
      break;
    default:
      cout << "Unkown type" << endl;
  }
};

ConstArray *ConstArray::repeat(int n)  { return resize(n*_size); }

ConstArray *ConstArray::expand(int n)  { return resize(n*_size); }


void ConstArray::cast(Type t) {

  if (t != INT && t!= FLOAT) {
    Warning("Can only cast numeric types");
    return;
  }
  Array *arr = this;
  
  switch (arr->type) {
  case INT:
    if (t==INT) return;
    int_constant = int(float_constant);
    float_constant = 0;
    break;
  case FLOAT:
    if (t==FLOAT) return;
    float_constant =  float(int_constant);
    int_constant = 0;
  default:
    Warning("Can only cast ints and floats");
  }
  arr->type = t;
}

void ConstArray::clear() {
  _size=0; 
  deleteName(); 
  int_constant = 0;
  float_constant = 0;
  obj_constant = 0;
}

UnTypedPtr ConstArray::getValPtr(int i) {
  if (i<_size && i>= 0) {
    switch (type) {
      case INT:
        return (UnTypedPtr) &int_constant;
      case FLOAT:
        return (UnTypedPtr) &float_constant;
      case OBJ:
        return (UnTypedPtr) &obj_constant;
      default:
        cout << "unkown type" << endl;
        return NULL;
    }
  } else{
    cout << "index out of range " << endl;
    return NULL;  
  }
}
/*
UnTypedPtr operator[](int i) {
  return this->getValPtr(i); 
}
*/
UnTypedPtr ConstArray::getVals() {
  UnTypedPtr vals;
  switch (this->type) {
    case INT:
      vals = (int *)new int[this->_size];
      for (int i=0; i<_size; i++) {((int *)vals)[i] = int_constant;}
      break;
    case FLOAT:
      vals = (float *)new float[this->_size];
      for (int i=0; i<_size; i++) {((float *)vals)[i] = float_constant;}
      break;
    case OBJ:
      vals = new UnTypedPtr[this->_size];
      for (int i=0; i<_size; i++) {((float *)vals)[i] = *(float *)obj_constant;}
      break;
  } 
}


