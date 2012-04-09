#ifndef _SUBAPPLY_H
#define _SUBAPPLY_H

#include "gridfieldoperator.h"
#define CALL_MEMBER_FUNC(object, ptrToMember) ((object).*(ptrToMember))

class ParameterAssigner {
  public:
    typedef void (GridFieldOperator::*ParamFunc)(UnTypedPtr value);
    ParameterAssigner(GridFieldOperator *op, ParamFunc method);
    void Assign(UnTypedPtr value);

  private:
    GridFieldOperator *parameterOwner;
    ParamFunc parameterAssignmentMethod;
};

class SubApplyOp : public UnaryGridFieldOperator {
  /* SubApply applies a recipes to an inner gridfield parameterized by values
   * in the outer gridfield.
   * An ParameterList associates attribute names in the outer gridfield (strings) 
   * with pointer-to-member-functions of the operators in the subrecipe. 
   * (through the ParameterAssigner class)
   * These functions are responsible for taking a value and setting the appropriate
   * member data for a particular operator in the recipe.
   * This way, conditions for restrict or even gridfields for aggregate can be
   * parameters for a recipe.
   */
  public:
    typedef vector<pair<string, ParameterAssigner *> > ParameterList;

    SubApplyOp(GridFieldOperator *toapply, 
        ParameterList *p, 
        string innerAttribute,
        GridFieldOperator *previous); 

    SubApplyOp(GridFieldOperator *toapply, 
        string innerAttribute,
        GridFieldOperator *previous); 

    void parameterize(string attr, 
        GridFieldOperator *op, 
        ParameterAssigner::ParamFunc method);

    void Execute();

    GridField *getNext();

  
  static GridField *SubApply(GridFieldOperator *toapply, 
                             ParameterList *p, 
                             string innerAttribute,
                             GridField *nestedGF);
protected:
  GridFieldOperator *toapply;
  ParameterList *plist;
  string innerAttribute;
  int next;

private:
};

#endif
