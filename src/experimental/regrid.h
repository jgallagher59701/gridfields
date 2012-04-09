#ifndef _REGRID_H
#define _REGRID_H

#include "expr.h"
#include "gridfieldoperator.h"

class ReGridOp : public BinaryGridFieldOperator {
  //  maps a grid onto another grid using a geometric interpretation 
  //  of the attributes of Geometry.  Not sure how to handle different
  //  interpolation mechanisms (linear, quadratic) or different assignment methods
  //  (cells that overlap target cells, cells that contain or are 
  //  contained by target cells)
  //
  //  Combinations we might support:
  //  Requirements: 
  //    rank(GeoX) == 0
  //    dim(GeoX) >= arity(GeoX)
  //    1st dim(GeoX) attributes on GeoX interpreted as point geometry --
  //  SourceRank, SourceGeo, TargetRank, TargetGeo: method for floats
  //  0, 0, 0, 0: find target point in source cell, linear interpolation
  //              (what if point lies on boundary?)
  //  n, 0, 0, 0: find source cells that contain target point, average cells values
  //  0, 0, n, 0: find source points contained by target cell, average them
  //  n, 0, n, 0: find source cells overlapping target cell, weighted average by extent
  //  
  //  -- 
  //  Integer values cast as floats? dropped? dropped for now.
  //  

public:
  ReGridOp(GridFieldOperator *Target, GridField *TargetGeometry, 
           GridFieldOperator *Source, GridField *SourceGeometry);
  ReGridOp(Condition *p, GridFieldOperator *op);
  
  void Execute();
  
  static GridField *ReGrid(Condition *p, GridField *GF);
  static GridField *ReGrid(string expr, GridField *GF);

  // Operator can be parameterized via these methods 
  void setFloatValue(UnTypedPtr val);
  void setIntValue(UnTypedPtr val);
protected:
  GridField *SourceGeometry;
  GridField *TargetGeometry;
    
private:
 
};

#endif
