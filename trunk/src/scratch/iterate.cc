#include "timing.h"
#include "state.h"
#include "iterate.h"
#include "array.h"

void IterateOp::Execute() {

  GridField *Aa = this->LeftOp->getResult();
  int nA = Aa->Size(this->_i);
  
  GridField *Bb=NULL;
  GridField *newcopy=NULL;
  
  for (int i=0; i<nA; i++) {
//    cout << "------------------->" << i << endl;
    float loop = gettime();
    Bb = this->RightOp->getResult();
    //Bb->print();
    // cout << "Loop: " << gettime() - loop << endl;
    //    if (i%10==0) cout << i << endl;
    Bb->GetAttribute(this->_j, "sumsalt")->print();
    //getchar();
    if (newcopy!=NULL) delete newcopy;
    newcopy = new GridField(Bb);
    state->SetState(newcopy);
    //state->getResult()->print();
    //getchar();

  }

  Result = state->getResult();
}

