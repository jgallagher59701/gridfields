
#include "timing.h"
#include "iostream"
#include "gridfieldoperator.h"
#include "gridfield.h"
#include "expr.h"
using namespace std;
ZeroaryGridFieldOperator::ZeroaryGridFieldOperator() {};

UnaryGridFieldOperator::UnaryGridFieldOperator(GridFieldOperator *prev) 
        : PreviousOp(prev), GF(NULL) {};

BinaryGridFieldOperator::BinaryGridFieldOperator(GridFieldOperator *L, 
                                                 GridFieldOperator *R)
        : LeftOp(L), RightOp(R), A(NULL), B(NULL) {};

GridFieldOperator::GridFieldOperator() : 
         Result(NULL), updated(true) { //cleanup(true), { 
}

GridField *GridFieldOperator::getResult() { 
  if (this->Updated(this->modtime)) {
    //if (this->Result) { this->Result->getScheme()->print();}
    this->clearResult();
    this->modtime = gettime();
    this->updated = false;
    this->Execute();
  }
  return &this->Result; 
};

void ZeroaryGridFieldOperator::PrepareForExecution() {} 

void UnaryGridFieldOperator::PrepareForExecution() {
  float start = gettime();
  if (this->PreviousOp == NULL) {
     if (this->GF == NULL) {
       Fatal("No gridfield or previous operator provided as input to Operator.");
     }
  } else {
    if (this->GF == NULL) {
      //Warning("Overwriting GridField pointer with previous op's output.");
    }
    this->GF = this->PreviousOp->getResult();
  }
  
  if (this->GF == NULL) {
    Fatal("No gridfield provided as input to Operator.");
  }
  //std::cout << "UnOp: " << gettime() - start << std::endl; 
}

void BinaryGridFieldOperator::PrepareForExecution() {
  float start = gettime();
  char arg = '0';
  if (this->LeftOp == NULL && this->A == NULL) {
    arg = 'A';
  }
  if (this->RightOp == NULL && this->B == NULL) {
    arg = 'B';
  }
  if (arg != '0') {
    Fatal("No %c argument or previous operator provided as input to Operator.", arg);
  }
  
  //cout << this->LeftOp << ", " << this->A << endl;
  //cout << this->LeftOp->Updated() << endl;
  if (this->LeftOp != NULL && this->A != NULL) {
    arg = 'A';
  }
  if (this->RightOp != NULL && this->B != NULL) {
    arg = 'B';
  }
  if (arg != '0') {
  //  Warning("Overwriting pointer to argument %c with previous op's output.", arg);
  }
  
  this->A = this->LeftOp->getResult();
  this->B = this->RightOp->getResult();
  
  if (this->A == NULL || this->B == NULL) {
    Fatal("No gridfield available as input to binary operator.");
  }
  //std::cout << "BinOp: " << gettime() - start << std::endl; 
}

GridFieldOperator::~GridFieldOperator() {
  // if we're a trivial gridfield operator, do nothing
  // otherwise dispose of our result
//  cout << this << ", " << this->Result << ", " << (this->Result == this) << endl;
//  if (this->cleanup) {
    
    //cout << "~gridfieldop: " << this << ", " << this->Result << ", " << this->Result->grid->name << " -> " << this->Result->getScheme()->asString() << endl;
    
//    cout << "deleting result gridfield" << endl;
//    this->Result->unref();
    clearResult();
//  }
}

inline bool ZeroaryGridFieldOperator::Updated(float sincetime) {
  return this->updated || this->modtime > sincetime;
}

bool UnaryGridFieldOperator::Updated(float sincetime) {
  bool result = this->updated || this->modtime > sincetime || this->PreviousOp->Updated(sincetime);
  return result;
}

bool BinaryGridFieldOperator::Updated(float sincetime) {
  bool result =  this->updated || this->modtime > sincetime 
	      || this->LeftOp->Updated(sincetime) || this->RightOp->Updated(sincetime);
  return result;
}

void GridFieldOperator::Update() {
  this->updated = true;
}

void GridFieldOperator::clearResult() {
  this->Result.Clear();
}
