#ifndef _ACCUMULATE_H
#define _ACCUMULATE_H

#include "gridfieldoperator.h"
#include <string>
#include <vector>
#include <ext/functional>
#include "cell.h"

class FunctionParser;
class GridField;

class AccumulateOp : public UnaryGridFieldOperator {
 public:
  
  AccumulateOp(GridFieldOperator *op, Dim_t k,
               std::string acc,
               std::string ex,
               std::string sdex);

  void Execute();
  static GridField *Accumulate(GridField *Gg, Dim_t k,
                               std::string resultname,
                               std::string expr,
                               std::string seedexpr,
                               int offset);

 void SetOffset(int off);
 int position_offset;
 private:
  Dim_t _k;
  std::string unparsedExpr;
  std::string _accumulator;
  std::string seedExpr;

  static std::vector<std::string> getVars(std::string expr);
  static std::string getVarStr(std::vector<std::string> varlist);
  static double bindVar(GridField *Gg, Dim_t k, std::string var, int i);
};

#endif /* ACCUMULATE_H */
