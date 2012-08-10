
#include "config.h"

#include "apply.h"
#include "timing.h"
#include "project.h"
#include "expr.h"
#include <math.h>

namespace GF {

ProjectOp::ProjectOp(GridFieldOperator *op, Dim_t k, string attribute_list) 
         :  UnaryGridFieldOperator(op),_k(k) 
{
  split(attribute_list, ";, -:", keepers);
}

ProjectOp::ProjectOp(GridFieldOperator *op, Dim_t k, vector<string> &ks) 
  : UnaryGridFieldOperator(op), _k(k), keepers(ks)
{
}

void ProjectOp::Execute() {
  this->PrepareForExecution();
  this->Result =  Project(this->GF, this->_k,
                          this->keepers);
}

GridField *ProjectOp::Project(GridField *Gg, Dim_t k, string keeper) {
  return ProjectOp::Project(Gg, k, vector<string>(1, keeper));
}

GridField *ProjectOp::Project(GridField *Gg, Dim_t k,
                              vector<string> keepers) {

  GridField *Out = new GridField(Gg->GetGrid());

  
  vector<string>::iterator p;
  for (p=keepers.begin(); p!=keepers.end(); ++p) {
    if (Gg->IsAttribute(k, *p)) {
      Out->Bind(k, Gg->GetAttribute(k, *p));
    }
  }
  return Out;
}

} // namespace GF

