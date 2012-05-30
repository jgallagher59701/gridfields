#ifndef _RemeshOP_H
#define _RemeshOP_H

#include "expr.h"
#include "gridfieldoperator.h"
#include "assignments.h"
#include "RTree.h"

class RemeshOp : public UnaryGridFieldOperator {
public:
  RemeshOp(string attrx,string attry, GridField * Gnew,GridFieldOperator *op);
  GridField* Gnew;
  string attry;
  string attrx;  
  void Execute();

  class Cgridpoint{
         public:
        double px;
        double py;
        double pz;
         //Cgridpoint(){px=new double;py=new double;pz=new double;}
          void setgridpoint(double x,double y)
          {//px=new double;py=new double;pz=new double;
          px=x;
          py=y;
          pz=0.0;
          }

          double getx(int k)
          {double pval;
           if(k==1)
           {pval=px;}
           else if(k==2)
           {pval=py;}
           else{pval=666.0;}
           return pval;}
       } gpoint;

  class CTriangle{
         public:
          Cgridpoint p1;
          Cgridpoint p2;
          Cgridpoint p3;
         //CTriangle(){p1=new Cgridpoint;p2=new Cgridpoint;p3=new Cgridpoint;}

          void setpointtriangle(Cgridpoint p11,Cgridpoint p12,Cgridpoint p13)
          {//p1=new Cgridpoint;p2=new Cgridpoint;p3=new Cgridpoint;
          p1=p11;
          p2=p12;
          p3=p13;
          }

          Cgridpoint getpoint(int k)
          {Cgridpoint pval;
           if(k==1)
           {pval=p1;}
           else if(k==2)
           {pval=p2;}
           else if(k==3)
           {pval=p3;}
           else if(k==4)
           {pval=p1;}
           else{cout<<"Devil in the details "<<endl;pval=p3;}
           return pval;}

       } gtriangle;




  class CPolygon{
         public:
          int Numpts;
          CPolygon(int n,vector<Cgridpoint> p13)
          {
           for(int i=1;i<=n;i++)
           {points.reserve(points.size()+1);points.push_back(p13.at(i-1));}
            Numpts=n;
          }
          vector<Cgridpoint> points;

          CPolygon(){Numpts=0;points.reserve(0);}
          void setpointpolygon(int n,vector<Cgridpoint> p13)
          {
           for(int i=1;i<=n;i++)
           {points.reserve(points.size()+1);points.push_back(p13.at(i-1));}
            Numpts=n;
          }

          Cgridpoint getpoint(int k)
          {Cgridpoint pval;
           if(k<=Numpts)
           {pval=points.at(k-1);}
           else{cout<<"Bad"<<endl;pval=points.at(Numpts+10);}
           return pval;}

           void addpoint(Cgridpoint p13)
           {Numpts=Numpts+1;Cgridpoint a;a.setgridpoint(p13.getx(1),p13.getx(2));
            points.reserve(points.size()+1);points.push_back(a);}



       } gpolygon;
RTree<CellId, double, 2> constructtree(Array* x1,Array* y1,vector< vector<int> > nodes2);
 /* class CPolygon{
         public:
         int polysize;
         vector<Cgridpoint*>* Points=new vector<Cgridpoint*>;
         
          void setpointPolygon(int n,vector<Cgridpoint>)
          {
           polysize=n;
           for(int i=1
          }

       } gtriangle;*/
/*  void setEnvironment(GridField *t, Dim_t i, GridField *s, Dim_t j) {
      this->T=t;
      this->S=s;
      this->_i = i;
      this->_j = j;
      left.setEnvironment(t,i,s,j);
      right.setEnvironment(t,i,s,j);
    };*/
  bool point_inside_polygon(double x,double y,CPolygon poly);

  bool   inside(vector<double>& p,vector<double>&);

  void clip(vector<double>& c,vector<double>& p1,vector<double>& p2,vector<double>& plane);

  void polyclip(CPolygon* pout,CPolygon* pin,double x1[2],double x2[2]);
  int intersect(CPolygon* pout,CPolygon* pin,CPolygon* pin1);
  GridField* Remesh(const string &attrx,const string &attry,GridField *GF,GridField* Gnew);
  //vector< vector<double> > Remeshvec(const string &attrx,const string &attry,GridField *GF, GridField * &Gnew);

  set<int> constructintersection(RTree<CellId, double, 2>& tree,CPolygon& Tri);
protected:

private:
 
};

#endif
