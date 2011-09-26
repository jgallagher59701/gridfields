#ifndef _REF_H
#define _REF_H

template<class T>
class TArray : public Array {

 public:
  typedef auto_ptr<T> reftype;
  
  using 
  char *name;
  TArray(string nm, Type t);
  TArray(string nm, Type t, int size);
  TArray(Array *a, string attr);
  TArray(const char *nm, Type t);
  TArray(string nm, Scheme *sch);
  TArray(const char *nm, Scheme *sch);
  void unref();
  void ref();
  ~Array();

  virtual Array *copyAndFilter(bool *filter);
  virtual Array *copy();

  virtual void copyIntData(int *data, int s);
  virtual void shareIntData(int *data, int s);

  virtual void copyFloatData(float *data, int s);
  virtual void shareFloatData(float *data, int s);

  virtual void copyObjData(void **data, int s);
  virtual void shareObjData(void **data, int s);

  virtual void getData(int *&out);
  virtual void getData(float *&out);
  virtual void getData(void **&out);

  int size() { return _size; }
  
  virtual void setVals(UnTypedPtr vals, int s);
  virtual UnTypedPtr getVals();
  virtual UnTypedPtr getValPtr(int i);
  virtual inline void next(UnTypedPtr *p) { 
    //++(((int *)&) (*p)); 
    *p = (UnTypedPtr) (((int) (*p)) + sizeof(int));
  };

  virtual Array *expand(int n);
  virtual Array *repeat(int n);
  virtual void cast(Type t);

  virtual void print();

  virtual void clear();

  Type type;

  virtual Scheme *getScheme();
 protected:
  int _size;
 private:
  int share;
  int full;
  int *ints;
  float *floats;
  void **objs;

  void setType(Type type);
  void init(const char *nm, Type t);

  Scheme *_sch;
};

/*



class ArrayOfVector : public Array, private vector<Tuple> {
 public:

   ArrayOfVector(Scheme *sch, int sz);

 private:
 
}
*/
#endif /* _ARRAY_H */
