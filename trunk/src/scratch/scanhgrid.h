#ifndef _SCANHGRIDOP_H
#define _SCANHGRIDOP_H

#include "elio.h"
#include "scan.h"

class ScanHGridOp : public ScanOp {
public:
  ScanHGridOp(string filename);
  void Execute();
  static GridField *Scan(string filename);
private:
  static GridField *gfH(ElcircHeader &h);

};

#endif
