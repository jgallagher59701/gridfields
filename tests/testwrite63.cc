#include <iostream>
#include "visualize.h"
#include "vtkGridField.h"
#include <fstream>
#include <sstream>
#include "array.h"
#include "grid.h"
#include "arrayreader.h"
#include "accumulate.h"
#include "elcircfile.h"
#include "bind.h"
#include "apply.h"
#include "cross.h"
#include "restrict.h"
#include "gridfield.h"
#include "write63.h"
#include <sstream>

using namespace std;

int main(int argc, char **argv) {

  ElcircFile ef("/home/workspace/ccalmr/hindcasts/1999-01-14/run/1_salt.63");
 
  GridField *H = ef.readHGrid();
  GridFieldOperator *Hp = new AccumulateOp(H, 0, "hpos", "hpos+1", "0");
  GridField *V = ef.readVGrid(); 
  GridFieldOperator *Vp = new AccumulateOp(V, 0, "vpos", "vpos+1", "0");
 
  stringstream ss;
  ss << "column+" << V->getResult()->Card(0) << "-b"; 
  GridFieldOperator *Acc = new AccumulateOp(Hp, 0, "column", ss.str(), "0");

  RestrictOp *zoom = new RestrictOp("(x<99350000)&(x>-99330000)&(y>-99280000)&(y<99310000)", 0, H);
  //RestrictOp *zoom = new RestrictOp("(x<350000)&(x>330000)&(y>280000)&(y<310000)", 0, H);
  
  ArrayReader *ar = ef.getSurfReader(0, "column");
  BindOp *surf = new BindOp("surf", FLOAT, ar, 0, Acc); 

  GridFieldOperator *HxV = new CrossOp(surf, Vp);

  GridFieldOperator *wetgrid = new RestrictOp("(vpos>b)", 0, HxV);
  GridFieldOperator *addr = new ApplyOp("addr=column - b + vpos", 0, wetgrid);
 
  addr->getResult()->GetAttribute(0, "addr")->print();

  ArrayReader *var = ef.getVariableReader("salt", 0, "addr");
  BindOp *salt = new BindOp("salt", FLOAT, ar, 0, Acc); 

  //vtkGridField *vtkS = toVTK(salt->getResult(), "salt");
  //vtkRenderWindow *renWin = vtkRenderWindow::New();
  //DirectVis(vtkS->GetOutput(), renWin);

  salt->getResult()->GetAttribute(0, "salt")->print();

/*
  #B = BindSome(context, var+ ["z"], addr)
  if no_z:
    bindvars = var
  else:
    bindvars = var+["z"]
  B = BindSome(context, bindvars, addr)
  #X = gf.Apply("foo=u*2", 0, B)
  #Z = gf.Apply("z=(vpos>16)*z*(h-surf)*60 + (vpos<17)*z*60", 0, B)
  return B, surfH, rV, addr
#  T = gf.Scan(context, "T")

#  dpr = vis.Visualize(X, "foo")
#  for i in range(T.getResult().Card(0)):
  cout << "H:" << H->card() << ", " << "V:" << V->card() << endl;

  ofstream *f = prepfile("out.63", 0);


  copyHeader("1_salt.63", *f);
  writeVGrid_bin(V, *f);
  writeHGrid_bin(H, *f);

*/
}
