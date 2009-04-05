#include "gridfield.h"
#include "array.h"
#include "subapply.h"
#include "restrict.h"
#include "expr.h"
#include "timing.h"
ParameterAssigner::ParameterAssigner(GridFieldOperator *op, ParamFunc f) {
  this->parameterOwner = op;
  this->parameterAssignmentMethod = f;
}

void ParameterAssigner::Assign(UnTypedPtr valptr) {
  CALL_MEMBER_FUNC(*this->parameterOwner, this->parameterAssignmentMethod)(valptr);
}

SubApplyOp::SubApplyOp(GridFieldOperator *toapply, Dim_t k,
                       string innerAttribute, 
                       GridFieldOperator *previous) 
 : UnaryGridFieldOperator(previous), _k(k),
   plist(new ParameterList()), 
   innerAttribute(NULL), 
   toapply(toapply)
{
}

SubApplyOp::SubApplyOp(GridFieldOperator *toapply, 
                       ParameterList *p, Dim_t k, 
                       string innerAttribute,
                       GridFieldOperator *previous)
 : UnaryGridFieldOperator(previous), plist(p), _k(k), innerAttribute(innerAttribute), toapply(toapply)
{ 
}

void SubApplyOp::parameterize(string attr, 
                              GridFieldOperator *op, 
                              ParameterAssigner::ParamFunc f) {
  this->plist->push_back(
       pair<string, ParameterAssigner *>(attr, new ParameterAssigner(op, f))
  );
}

void SubApplyOp::Execute() {
  this->PrepareForExecution();
  this->Result = SubApplyOp::SubApply(this->toapply, 
                                      this->plist, 
                                      this->_k,
                                      this->innerAttribute, 
                                      this->GF);
  cout << Result << endl;
}

GridField *SubApplyOp::getNext() {

  if (this->next >= this->GF->card())
    return NULL;
  
  string attr;
  UnTypedPtr val;
  ParameterList::const_iterator pi;
  ParameterAssigner *pa;
  
  // Assign all the parameters to the subrecipe
  for (pi = plist->begin(); pi!=plist->end(); pi++) {
    attr = (*pi).first;
    pa = (*pi).second;
    val = this->GF->getAttributeVal(attr.c_str(), this->next++);
    pa->Assign(val);
  }
  
  return toapply->getResult();
}

GridField *SubApplyOp::SubApply(GridFieldOperator *toapply, 
                                ParameterList *plist, Dim_t k,
                                string innerAttribute,
                                GridField *nestedGF) {

  if (plist == NULL) {
    Fatal("No parameter list passed to SubApply");
  }

  ParameterList::const_iterator pi;
  
  ParameterAssigner *pa;
  string attr;
  UnTypedPtr val;

  Array *innerGFs = new Array(innerAttribute.c_str(), GRIDFIELD);
  UnTypedPtr *gfptrs = new UnTypedPtr[nestedGF->card()];
  
  for (int i=0; i<nestedGF->card(); i++) {
    
    // Assign all the parameters to the subrecipe
    for (pi = plist->begin(); pi!=plist->end(); pi++) {
      attr = (*pi).first;
      pa = (*pi).second;
      val = nestedGF->getAttributeVal(attr.c_str(), i);
      pa->Assign(val);
    }

    //save the new gridfield in the new attribute
    gfptrs[i] = (UnTypedPtr) toapply->getResult();
  }
    
  //prepare the result (mostly a shallow copy of the input)
  GridField *out = new GridField(nestedGF->grid, nestedGF->rank());
  Scheme *sch = nestedGF->getScheme();
  for (int i=0; i<sch->size(); i++) {
    attr = sch->getAttribute(i);
    if (attr != innerAttribute) {
      out->Bind(nestedGF->getAttribute(attr.c_str()));
    }
  }

  //Bind the new Array of inner gridfields.
  innerGFs->shareObjData(gfptrs, nestedGF->card());
  out->Bind(innerGFs);

  return out;
}
