#ifndef _CONSTARRAY_H
#define _CONSTARRAY_H

#include "type.h"
#include "object.h"
#include "array.h"

#include <string>

//using namespace std;

namespace GF {

class Scheme;

class ConstArray : public Array {

 public:
  ConstArray(string nm, int size, Type t, UnTypedPtr constant);
  ConstArray(string nm, int size, float constant);
  ConstArray(string nm, int size, int constant);
  ConstArray(string nm, int size, UnTypedPtr constant);
  //virtual ~ConstArray() {};

  ConstArray *copyAndFilter(bool *filter);
  ConstArray *copy();

  void copyData(int *data, int s) ;
  void shareData(int *data, int s);

  void copyData(float *data, int s);
  void shareData(float *data, int s);

  void copyData(void **data, int s);
  void shareData(void **data, int s);

  void getData(int *&out);
  void getData(float *&out);
  void getData(void **&out);

  void setVals(UnTypedPtr vals, int s);
  UnTypedPtr getVals();
  
  UnTypedPtr getValPtr(int i);
  inline void next(UnTypedPtr *) { };

  long getConst();
  void setConst(UnTypedPtr newconst);
  
  ConstArray *expand(int n);
  ConstArray *repeat(int n);

  ConstArray *resize(int newsize);
  
  void cast(Type t);

  void print() { 
    cout << "name: " << this->getName() << endl;
    cout << "ConstArray: " << this->_size << ", " << type << ", " << this->float_constant << ":" << this->int_constant << ":" << this->obj_constant << endl;
  };

  void clear();

  void SetConst(float val) { 
    float_constant = val; 
    type = FLOAT;
  };
  void SetConst(int val) { 
    int_constant = val; 
    type = INT;
  };

 private:
  float float_constant;
  int int_constant;
  UnTypedPtr obj_constant;
  Scheme *_sch;
};

} // namespace GF

#endif /* _ARRAY_H */
