#include "accumulate.h"
#include "fparser.hh"
#include "gridfield.h"
#include "array.h"
#include "timing.h"
#include "expr.h"
#include <string>
#include <vector>
#include <math.h>

AccumulateOp::AccumulateOp(GridFieldOperator *op, Dim_t k,
                           string acc,
                           string ex,
                           string sdex) 
             : UnaryGridFieldOperator(op), position_offset(0), _k(k)
{
  this->PreviousOp = op;
  this->_accumulator = acc;
  this->unparsedExpr = ex;
  this->seedExpr = sdex;
  this->GF = NULL;
 // this->cleanup = false;
} 

void AccumulateOp::SetOffset(int off) {
  this->position_offset = off;
}

void AccumulateOp::Execute() {
  this->PrepareForExecution();
  this->Result =  Accumulate(this->GF, this->_k,
	                     this->_accumulator,
                             this->unparsedExpr,
                             this->seedExpr,
                             this->position_offset);
}


GridField *AccumulateOp::Accumulate(GridField *Gg, Dim_t k,
			  string accumulator,
			  string expr,
                          string seedexpr,
                          int offset) {

  FunctionParser fp;
  FunctionParser seedfp;
  expr = remove_whitespace(expr);
  
  vector<string> vars = getVars(expr);
  vector<string> seedvars = getVars(seedexpr);
  int card = Gg->Size(k);
  
  // check existence of the variables in the expression
  for (unsigned int j=0; j<vars.size(); j++) {
    if ( !Gg->IsAttribute(k, vars[j]) && 
         !(vars[j] == accumulator)) {
      Warning("Accumulate(%s): %s is not an attribute of this Gridfield.", 
               expr.c_str(), vars[j].c_str());
      Gg->GetScheme(k).print();
      return Gg;
    }
  }

  // check the seed expression
  for (unsigned int j=0; j<seedvars.size(); j++) {
    if ( !Gg->IsAttribute(k, seedvars[j])) {
      Warning("Accumulate(%s): %s is not an attribute of this Gridfield.", 
	      expr.c_str(), seedvars[j].c_str());
      Gg->GetScheme(k).print();
      return Gg;
    }
  }

  // parse the expression
  string varstr = getVarStr(vars);
  if (fp.Parse(expr, varstr) != -1) {
    Warning("Parse error: %s: '%s' \% (%s) ", 
            fp.ErrorMsg(),
	    expr.c_str(), 
            varstr.c_str());
    return Gg;
  }

  //parse the seed expression
  string seedvarstr = getVarStr(seedvars);
  if (seedfp.Parse(seedexpr, seedvarstr) != -1) {
    Warning("Parse error: %s: %s", 
	    seedexpr.c_str(), seedfp.ErrorMsg());
    return Gg;
  }
  
  // Evaluate the expression on the GF's attributes
  double *varVals = new double[vars.size()];
  double *seedVarVals = new double[seedvars.size()];
  float *accumulated = new float[card];

  //set the initial value of the new attribute
  for (unsigned int j=0; j<seedvars.size(); j++) {
    seedVarVals[j] = AccumulateOp::bindVar(Gg, k, vars[j], 0);
  }

  int start = MIN(card, MAX(-offset, 1));
  int end = MIN(-offset+card, card);
  
  //set the first -<offset> values to the seed (if offset negative)
  float seed = (float) seedfp.Eval(seedVarVals);
  for (int i=0; i<start; i++) {
    accumulated[i] = seed;
  }

  //set the last |offset| values to the seed (if offset positive)
  for (int i=end; i<card; i++) {
    accumulated[i] = seed;
  }
  
  Scheme s = Gg->GetScheme(k);
  //Tuple t(&s);
  //set the rest of the values
  for (int i=start; i<end; i++) {
    //Gg->FastBindTuple(i, t);
    
    for (unsigned int j=0; j<vars.size(); j++) {
      
      if (vars[j] == accumulator) {
        //previous accumulated value
        varVals[j] = accumulated[i-1];
      } else {
        //regular value from the gridfield
        varVals[j] = AccumulateOp::bindVar(Gg, k, vars[j], i+offset);
      }
      
    }
    accumulated[i] = (float) fp.Eval(varVals);
  }
  
  //append the new attribute to the output gridfield
  Array *arrResult = new Array(
		 (const char *) accumulator.c_str(), FLOAT);
  arrResult->copyFloatData(accumulated, card);
  delete [] accumulated;
  Gg->Bind(k, arrResult);
  arrResult->unref();
  Gg->ref();
  return Gg;
  
}

double AccumulateOp::bindVar(GridField *Gg, Dim_t k, string var, int i) {
  //bind a variable to the ith value of the gridfield
  //attribute with the same name
  Scheme sch = Gg->GetScheme(k);
  double val = 0;
  if (sch.getType(var) == INT) {
    val =  double(*(int *)
                Gg->GetAttributeValue(k, var, i));
  } else {
    val =  double(*(float *)
                Gg->GetAttributeValue(k, var, i));

  }
  return val;
}
string AccumulateOp::getVarStr(vector<string> varlist) {
  if (varlist.size() == 0) return "";
  string retval(varlist[0]);
  for (size_t i=1; i<varlist.size(); i++) {
    retval = retval + "," + varlist[i];
  }
  return retval;
}

string getVar(string &expr, size_t &i) {
  string var = "";
  while (isalpha(expr[i]) || expr[i] == '_' || isdigit(expr[i])) {
    var = var + expr[i++];
  }
  return var;
}

vector<string> AccumulateOp::getVars(string expr) {
  set<string> vars;
  size_t i=0;

  while (i<expr.length()) {
    if (isalpha(expr[i])) {
      vars.insert(getVar(expr, i));
    } else if (expr[i] == '_') {
          
    }
    i++;
  }
  
  vector<string> retval(vars.size());
  set<string>::iterator p;
  
  for (p=vars.begin(), i=0; p!=vars.end(); i++,p++) {
    retval[i] = *p;
  }

  return retval;
}

