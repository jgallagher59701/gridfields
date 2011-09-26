#ifndef _SCANVGRIDOP_H
#define _SCANVGRIDOP_H

#include "elio.h"
#include "scan.h"

class ScanVGridOp : public ScanOp {
public:
  ScanVGridOp(string filename);
 
  void Execute();
  static GridField *Scan(string filename);
private:
  static GridField *gfV(ElcircHeader &h);

};

#endif
