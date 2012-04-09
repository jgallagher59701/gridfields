#include "gridfield.h"
#include "array.h"
#include "regrid.h"
#include "apply.h"
#include "expr.h"
#include "timing.h"


ReGridOp::ReGridOp(GridFieldOperator *Target, GridFieldOperator *TargetGeometry,
                   GridFieldOperator *Source, GridFieldOperator *SourceGeometry) {
  this->SourceGeometry = NULL;
  this->SourceGeometryOp = SourceGeometry;
  this->TargetGeometry = NULL;
  this->TargetGeometryOp = TargetGeometry;
  this->A = NULL;
  this->B = NULL;
  this->LeftOp = Target;
  this->RightOp = Source;
}

void ReGridOp::Execute() {
  this->PrepareForExecution();
  if (this->p == NULL) {
    this->Result = ReGrid(this->expr, this->GF);
  } else {
    this->Result = ReGrid(this->p, this->GF);
  }
}


GridField *ReGridOp::ReGrid(GridField *Target, GridField *TargetGeometry, 
                            GridField *Source, GridField *SourceGeometry) {
  //1) check source and target gridfield for dimension < 3
  //2) check Xgeometry gridfields for dim(X) float attributes on 0-cells
  //3) search for target 0-cells in source dim-cells
  //4) 
  //  0, 0, 0, 0: find target point in source cell, linear interpolation
  //              (what if point lies on boundary?)
  //  n, 0, 0, 0: find source cells that contain target point, average cells values
  //  0, 0, n, 0: find source points contained by target cell, average them
  //  n, 0, n, 0: find source cells overlapping target cell, weighted average by extent
  //  
}


class pointpoly2 : public AssignmentFunction {
  public:
    virtual void pointpoly2::operator()(Cell &c, vector<Cell>&out) {
      typedef vector<Tuple *> Poly;

      Tuple *tup;
      Poly poly;
      int pts;
      
      //the target point
      Tuple *t = T->eval(c); 
      float x = *(float *)t->get("x");
      float y = *(float *)t->get("y");
      
      float *xs = new float[10];
      float *ys = new float[10];
      
      Cell *sc; 
      //for each cell in the source grid
      for (int i=0; i<S->card(); i++) {
        tup = S->getTuple(i);
        poly = *(Poly *) tup->get("poly");
        pts = poly.size();
        if (pts==0) continue;
        
        for (int j=0; j<pts; j++) {
          xs[j] = *(float *)poly[j]->get("x");
          ys[j] = *(float *)poly[j]->get("y");
        }
      
        if (pnpoly(pts, xs, ys, x, y)) {
          sc = S->grid->getKCells(S->k)->getCell(i);  
          out.push_back(*sc);
        }
      }
    }
};

  

class pointpoly3 : public AssignmentFunction {
  //from source points to target polygons
  public:
    
    virtual void pointpoly3::operator()(Cell &c, vector<Cell>&out) {
      assert(T->grid->getdim() == 2);
      typedef vector<Tuple *> Poly;
      Poly *poly;
      
      Tuple *t;
      Tuple *polytup = T->eval(c);
      poly = (Poly *)polytup->get("poly");
      
      int s = poly->size(); 
      float *xs = new float[s];
      float *ys = new float[s];
      
      for (int j=0; j<s; j++) {
        t = (*poly)[j];
        xs[j] = *(float *)t->get("x");
        ys[j] = *(float *)t->get("y");
//        cout << " | " << xs[j] << ", " << ys[j];
      }
      
         // cout << "----" << endl;

      float x;
      float y;
      Cell *e; 
      AbstractCellArray *kcells = S->grid->getKCells(S->rank());
      for (int i=0; i<S->card(); i++) {
        x = *(float *)S->getAttributeVal("x", i);
        y = *(float *)S->getAttributeVal("y", i);
        //cout << x << ", " << y << endl;
        if (pnpoly(s, xs, ys, x, y)) {
          e = kcells->getCell(i);
          //cout << i << endl;
          //this point goes to be aggregated.
//          c.print();
//          e->print();
          out.push_back(*e);
        }
      }
         
    }
};

class pointpoly : public AssignmentFunction {
  
  public:
    
  /* Expects 2-dimensional gridfields with geometry 
   * defined on the nodes as attributes "x" and "y"
   * Returns the nodes incident to the cells in S that 
   * contain the target cell.
   * Requires that the source grid be normalized
   * maximum polygon size is 10 nodes.
   */
    
    virtual void pointpoly::operator()(Cell &c, vector<Cell>&out) {
      assert(S->grid->getdim() == 2);
      float *xs = new float[10];
      float *ys = new float[10];
      int s = 0; 
      Tuple *t = T->eval(c);
      float x = *(float *)t->get("x");
      float y = *(float *)t->get("y");
      AbstractCellArray *twocells = S->grid->getKCells(2);
      AbstractCellArray *nodes = S->grid->getKCells(0);
      Cell *Sc;
      Cell *node;

      int zt, zs, b;
      int *hck;
      int tpos;
      Tuple *a;
      
      //cout << "checking point " << c.nodes[0] << "...." << flush;
//      if (c.nodes[0] >= 94) cout << "("<< x << "," <<y<<")" << flush;
      int size = twocells->getsize();
      for (int i=0; i < size; i++) {
          Sc = twocells->getCell(i);
          s = Sc->size;
//          if (S->card() <10000) 
  //          cout << "cell----" << endl;
          for (int j=0; j<s; j++) {
            xs[j] = *(float *)S->getAttributeVal("x", Sc->nodes[j]);
            ys[j] = *(float *)S->getAttributeVal("y", Sc->nodes[j]);
          }
         // cout << "----" << endl;
          if (pnpoly(s, xs, ys, x, y)) {
            for (int j=0; j<s; j++) {
              node = nodes->getCell(Sc->nodes[j]);
              if (S->isAttribute("zpos") && T->isAttribute("zpos")) {
                zs = *(int *)S->getAttributeVal("zpos", Sc->nodes[j]);
                zt = *(int *)T->getAttributeVal("zpos", T->grid->getKCells(T->rank())->getOrd(c));
                if (zt == zs) {
                  b = *(int *)S->getAttributeVal("b", Sc->nodes[j]);
                  if (zs>=b) {
                    out.push_back(*node);
                  }
                }
              } else {
                  out.push_back(*node);
              }
   

              //WOOOOOP! WOOOOOP!
              //EMERGENCY HACK!  
                
              if (S->isAttribute("hack")) { 
                tpos = S->grid->getKCells(0)->getOrd(*node);        
                hck = (int *) S->getAttribute("hack")->getValPtr(tpos);
                //cout << "changed?" << endl;
                //cout << *hck << endl;
                *hck = 1;              
  //                *hck = twocells->getOrd(*Sc);
                //cout << *hck << endl;
                //hck = (int *) T->getAttribute("hack")->getValPtr(tpos);
                //cout << *hck << endl;
              }
            }
            
          }
      }
    }

};

  
int pnpoly(int npol, float *xp, float *yp, float x, float y) {
  int i, j, c = 0;
  for (i = 0, j = npol-1; i < npol; j = i++) {
    if ((((yp[i] <= y) && (y < yp[j])) ||
       ((yp[j] <= y) && (y < yp[i]))) &&
       (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))
       c = !c;
  }
  return c;
};


