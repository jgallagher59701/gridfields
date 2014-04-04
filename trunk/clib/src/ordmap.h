#ifndef _ORDMAP_H
#define _ORDMAP_H

namespace GF {

class Cell;

class OrdMap {

 public:
  //virtual ~OrdMap()=0;
	virtual ~OrdMap() {} // The class needs a dtor since it has virtual methods. jhrg 4/4/14
  virtual int getBaseSize(int d) = 0;
  virtual int getBaseOrd(Cell *c, int d) = 0;
 private:
};

} // namespace GF

#endif /* _ORDMAP_H */
