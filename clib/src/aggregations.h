#include "aggregate.h"
#include "expr.h"

#define TOLERANCE 1e-5
#define NULL_VALUE -999

namespace Aggregate {

using namespace GF;

float euclid(float x1, float y1, float x2, float y2);
float euclid3D(float x1, float y1, float z1, float x2, float y2, float z2);

class dotwo : public AggregationFunction {
  private:
    AggregationFunction &left;
    AggregationFunction &right;
    Scheme *leftsch;
    Scheme *rightsch;
    Scheme outsch;
    
  public:
    dotwo(AggregationFunction &left, AggregationFunction &right) :
         left(left), right(right) {};

    void operator()(vector<Tuple> &tupset, Tuple &out) {
      
      Tuple lefttup(leftsch);
      Tuple righttup(rightsch);

      mergeTuples(out, lefttup, leftsch, "left_");
      mergeTuples(out, righttup, rightsch, "right_");

      left(tupset, lefttup);
      right(tupset, righttup);

    };

    void mergeTuples(Tuple &out, Tuple &lefttup,
                     Scheme *leftsch, string prefix) {
      
      for (unsigned int i=0; i<leftsch->size(); i++) {
        string attr = leftsch->getAttribute(i);
        if (outsch.isAttribute(attr)) 
          lefttup.set(attr, out.get(attr));
        else
          lefttup.set(attr, out.get(prefix + attr));
      }
      
    }
    
    void mergeSchemes(Scheme *sch, Scheme *leftsch, 
                      Scheme *rightsch, string prefix) {
      
      for (unsigned int i=0; i<leftsch->size(); i++) {
        string attr = leftsch->getAttribute(i);
        Type type = leftsch->getType(i);
        
        if (rightsch->isAttribute(attr))
          sch->addAttribute(prefix + attr, type);
        else
          sch->addAttribute(attr, type);
      } 
    };
    
    Scheme *getOutScheme(Scheme *insch) {
      outsch.clear();
      leftsch = left.getOutScheme(insch);
      rightsch = right.getOutScheme(insch);

      mergeSchemes(&outsch, leftsch, rightsch, "left_");
      mergeSchemes(&outsch, rightsch, leftsch, "right_");

      return &outsch;
    };
};
/*
class AggregationFunction {
  public:
    constructor(string, null_value=NULL_VALUE)
    initialize()=0
    step()=0
    finalize()=0
    getscheme() 
}
*/

template<typename NumericType>
class triGradient : public AggregationFunction {
  public:
    triGradient(string as) : null_value(NULL_VALUE) { 
      split(as, " ;,", attrs); 
    }
    triGradient(string as, NumericType nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
    }
    
    void operator()(vector<Tuple> &tupset, Tuple &out) {
      if (tupset.size() != 3) {
        Fatal("triGrad Aggregation function only works on triangles; %i vertices were passed in.", tupset.size());
      }

      float *area = (float *) out.get("area");
      *area = 0;
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (outsch.isAttribute("gradx"+attrs[i])) {
          float *gradx = (float *) out.get("gradx"+attrs[i]);
          float *grady = (float *) out.get("grady"+attrs[i]);
           
          this->trigrad(tupset, attrs[i], area, gradx, grady);
        }
      }
    }
   
    void trigrad(vector<Tuple> &tupset, string &attr, 
                float *area, float *gradx, float *grady) {
     float x[3];
     float y[3];
     NumericType u[3];

     for (unsigned int i=0; i<3; i++) {
       x[i] = *(float*) tupset[i].get("x");
       y[i] = *(float*) tupset[i].get("y");
       u[i] = *(NumericType*) tupset[i].get(attr);
     }
     
     float tmp_x = (y[1]-y[2])*u[0] + (y[2]-y[0])*u[1] + (y[0]-y[1])*u[2]; 
     float tmp_y = (x[2]-x[1])*u[0] + (x[0]-x[2])*u[1] + (x[1]-x[0])*u[2]; 
     
     *area = (x[1]*(y[2] - y[0]) + x[0]*(y[1] - y[2]) + x[2]*(y[0] - y[1])) / 2;
     
     *gradx = tmp_x; // / ( 2 * (*area) * (*area) ); 
     *grady = tmp_y; // / ( 2 * (*area) * (*area) );
   }
    
   Scheme *getOutScheme(Scheme *insch) {
     outsch.clear();
     outsch.addAttribute("area", FLOAT);
     for (unsigned int i=0; i<attrs.size(); i++) {
       if (insch->isAttribute(attrs[i])) {
         outsch.addAttribute("gradx"+attrs[i], FLOAT);
         outsch.addAttribute("grady"+attrs[i], FLOAT);
       }
     }
     return &outsch;
   }
   
   private:
    Scheme outsch;
    NumericType null_value;
    vector<string> attrs;
    vector<NumericType> totals;

};

class Any : public AggregationFunction {
  public:
    Any(string acheck, string aset, int tag) 
              : tag(tag), setattr(aset), checkattr(acheck) {}
    
    Any(string a, int tag=1) 
           : tag(tag), setattr(a), checkattr(a) {}

    void operator()(vector<Tuple> &tupset, Tuple &out) {
      int *valptr;
      valptr = (int *) out.get(setattr);

      for (unsigned int i=0; i<tupset.size(); i++) {
        int val = *(int *) tupset[0].get(checkattr);
        cout << val << endl;
        if (val) {
          *valptr = tag;
          return;
        }
      }
      *valptr = 0;
#if 0
      cout << "none" << endl;
#endif
      return;
    }
    Scheme *getOutScheme(Scheme * /*insch unused jhrg 10/5/11*/) {
      outsch.clear();
      outsch.addAttribute(setattr, INT);
      return &outsch;
    } 
  private:
   Scheme outsch;
   int tag;
   string setattr, checkattr;
};

template<typename NumericType>
class gradient : public AggregationFunction {
  public:
    gradient(string as) : null_value(NULL_VALUE) { 
      split(as, " ;,", attrs); 
    }
    gradient(string as, NumericType nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
    }
    
    void operator()(vector<Tuple> &tupset, Tuple &out) {
      float x = *(float *) out.get("x");
      float y = *(float *) out.get("y");

      for (unsigned int i=0; i<attrs.size(); i++) {
        if (outsch.isAttribute("gradx"+attrs[i])) {
          float *gradx = (float *) out.get("gradx"+attrs[i]);
          float *grady = (float *) out.get("grady"+attrs[i]);
          NumericType u = 
              *(NumericType *) out.get(attrs[i]);
           
          this->gradLeastSquares(x, y, u, 
                                 attrs[i], tupset, 
                                 gradx, grady);
        }
      }
    }
   
   
   void gradLeastSquares(float x0, float y0, NumericType u0,
                 string attr, vector<Tuple> &tupset,
                 float *gradx, float *grady) {
     // based on 
     // "Revisiting the Leaset Squares Procedure
     //   for gradient estimation on unstructured meshes"
     // Dmitri Mavriplis
     // NASA Technical Report, NIA Report No. 2003-06
     
     // weights are euclidean distance (or 1 if fast)

     bool fast = false;
     
     float a=0, b=0, c=0, d=0, e=0;

     if (tupset.empty()) {
       *gradx = 0;
       *grady = 0;
       return;
     }
     
     float w2 = 1; 
     
     for (unsigned int i=0; i<tupset.size(); i++) {
       float xi = *(float *) tupset[i].get("x");
       float yi = *(float *) tupset[i].get("y");
       NumericType ui = *(NumericType *) tupset[i].get(attr);
       NumericType du = ui - u0;
  
       float dx = xi - x0;
       float dy = yi - y0;
       if (dx==0 || dy==0) continue;

       // (inverse distance)^2 = weight^2
       if (!fast)
         w2 = 1 / (pow( dx, 2 ) + pow( dy, 2 ));
         
       a += w2 * pow(dx, 2);
       b += w2 * dx * dy;
       c += w2 * pow(dy, 2);
       d += w2 * du * dx;
       e += w2 * du * dy;
       
     }
     
     // Cramer's rule
     float det = a*c - pow(b,2);
     assert(det != 0);
     
     *gradx = (d*c - b*e) / det;
     *grady = (a*e - d*b) / det;
     
   }
    
   void gradnorm(float x0, float y0, NumericType f0,
                 string attr, vector<Tuple> &tupset,
                 float *gradx, float *grady) {
     
     float maxdfdu = 0;
     
     *gradx = float(null_value);
     *grady = float(null_value);

     for (unsigned int i=0; i<tupset.size(); i++) {
       float xi = *(float *) tupset[i].get("x");
       float yi = *(float *) tupset[i].get("y");
       NumericType fi = *(NumericType *) tupset[i].get(attr);
       
       float norm = sqrt( pow( (xi-x0), 2 ) + pow( (yi-y0), 2 ) );
       
       float dfdu = (fi - f0) / norm;
       
       
       if (i==0) {
         *gradx = dfdu * ( (xi - x0) / norm );
         *grady = dfdu * ( (yi - y0) / norm );
         maxdfdu = dfdu;
       } else {
         if ( (dfdu > maxdfdu) ) {
           *gradx = dfdu * ( (xi - x0) / norm );
           *grady = dfdu * ( (yi - y0) / norm );
           maxdfdu = dfdu;
         }
       }
     }
   }

   Scheme *getOutScheme(Scheme *insch) {
     outsch.clear();
     for (unsigned int i=0; i<attrs.size(); i++) {
       if (insch->isAttribute(attrs[i])) {
         outsch.addAttribute("gradx"+attrs[i], FLOAT);
         outsch.addAttribute("grady"+attrs[i], FLOAT);
       }
     }
     return &outsch;
   }
   
   private:
    Scheme outsch;
    NumericType null_value;
    vector<string> attrs;
    vector<NumericType> totals;
};

template<typename NumericType>
class gradient3D : public AggregationFunction {
  public:
    gradient3D(string as) : null_value(NULL_VALUE) { 
      split(as, " ;,", attrs); 
    }
    gradient3D(string as, NumericType nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
    }
    
    void operator()(vector<Tuple> &tupset, Tuple &out) {
      float x = *(float *) out.get("x");
      float y = *(float *) out.get("y");
      float z = *(float *) out.get("z");
      for (unsigned int i=0; i<attrs.size(); i++) {
        float *gradx = (float *) out.get("gradx"+attrs[i]);
        float *grady = (float *) out.get("grady"+attrs[i]);
        float *gradz = (float *) out.get("gradz"+attrs[i]);
        NumericType u = 
            *(NumericType *) out.get(attrs[i]);
           
        this->gradLeastSquares(x, y, z, u, 
                               v_poss[i], tupset, 
                               gradx, grady, gradz);
      }
    }
   
   
   void gradLeastSquares(float x0, float y0, float z0, NumericType u0,
                 int attr_pos, vector<Tuple> &tupset,
                 float *gradx, float *grady, float *gradz) {
     // based on 
     // "Revisiting the Leaset Squares Procedure
     //   for gradient estimation on unstructured meshes"
     // Dmitri Mavriplis
     // NASA Technical Report, NIA Report No. 2003-06
     
     // weights are euclidean distance (or 1 if fast)

     bool fast = false;
     
     float a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0, i=0;

     if (tupset.empty()) {
       *gradx = 0;
       *grady = 0;
       *gradz = 0;
       return;
     }
     
     float w2 = 1; 
     for (unsigned int k=0; k<tupset.size(); k++) {
       float xk = *(float *) tupset[k].get(x_pos);
       float yk = *(float *) tupset[k].get(y_pos);
       float zk = *(float *) tupset[k].get(z_pos);
       //cout << xk << ", " << yk << ", " << zk << endl;
       NumericType uk = *(NumericType *) tupset[k].get(attr_pos);
       NumericType du = uk - u0;
  
       float dx = xk - x0;
       float dy = yk - y0;
       float dz = zk - z0;

       // inverse distance weights
       if (!fast)
         w2 = 1 / pow( pow(dx, 2) + pow(dy, 2) + pow(dz, 2), 2 );
         
       
        //  | a  b  c | | ux |   | g  |
        //  | b  d  e | | uy | = | h  |
        //  | c  e  f | | uz |   | i  |
         
       a += w2 * pow(dx, 2);
       b += w2 * dx * dy;
       c += w2 * dx * dz;
       
       d += w2 * pow(dy, 2);
       e += w2 * dy * dz;
       
       f += w2 * pow(dz, 2);
       
       g += w2 * du * dx;
       h += w2 * du * dy;
       i += w2 * du * dz;
     }
     
     // Cramer's rule
     float det = a*d*f - a*e*e + b*e*c - b*b*f + c*b*e - c*d*c;
     assert(det != 0);
     
     
      // g b c
      // h d e
      // i e f
      
     *gradx = (g*d*f - g*e*e + b*e*i - b*h*f + c*h*e - c*d*i) / det;
     
     
      // a g c
      // b h e
      // c i f
      
     *grady = (a*h*f - a*e*i + g*e*c - g*b*f + c*b*i - c*h*c) / det;
     
     
      // a b g
      // b d h
      // c e i
      
     *gradz = (a*d*i - a*h*e + b*h*c - b*b*i + g*b*e - g*d*c) / det;
     
   }
    
   Scheme *getOutScheme(Scheme *insch) {
     outsch.clear();
     
     v_poss.clear();
     x_pos = insch->getPosition("x");
     y_pos = insch->getPosition("y");
     z_pos = insch->getPosition("z");

     for (unsigned int i=0; i<attrs.size(); i++) {
       if (insch->isAttribute(attrs[i])) {
         v_poss.push_back(insch->getPosition(attrs[i]));
         outsch.addAttribute("gradx"+attrs[i], FLOAT);
         outsch.addAttribute("grady"+attrs[i], FLOAT);
         outsch.addAttribute("gradz"+attrs[i], FLOAT);
       } else {
         Fatal("%s is not an attribute of the source gridfield's scheme: %s",
                attrs[i].c_str(), insch->asString().c_str());
       }
     }
     return &outsch;
   }
   
   private:
    int x_pos, y_pos, z_pos;
    vector<int> v_poss;
    Scheme outsch;
    NumericType null_value;
    vector<string> attrs;
    vector<NumericType> totals;
};



template<typename NumericType>
class _sum : public AggregationFunction {
  public:
    _sum(string as) : null_value(NULL_VALUE) { 
      split(as, " ;,", attrs); 
    }
    _sum(string as, NumericType nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
    }

    void operator()(vector<Tuple> &tupset, Tuple &out) {
      NumericType *valptr;
      vector<string>::iterator a;
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (outsch.isAttribute("sum"+attrs[i])) {
          valptr = (NumericType *) out.get("sum"+attrs[i]);
          *valptr = sum(attrs[i], tupset);
        }
      }
    }
 
   NumericType sum(string attr, vector<Tuple> &tupset) {
     //cout << tupset.size() << endl;
     if (tupset.size() == 0) return null_value;
     NumericType total=0;
     NumericType val;
     for (unsigned int i=0; i<tupset.size(); i++) {
       val = *(NumericType *) tupset[i].get(attr);
       if (val == null_value) continue; // return null_value;
       total += val;
     }
     return total;
   }
    
   Scheme *getOutScheme(Scheme *insch) {
     outsch.clear();
     for (unsigned int i=0; i<attrs.size(); i++) {
       if (insch->isAttribute(attrs[i])) {
         outsch.addAttribute("sum"+attrs[i], insch->getType(attrs[i]));
       }
     }
     return &outsch;
   }
    
  private:
    Scheme outsch;
    NumericType null_value;
    vector<string> attrs;
    vector<NumericType> totals;
};

template<typename NumericType>
class _average : public AggregationFunction {

  public:
    _average(string as) { 
      split(as, " ;,", attrs); 
    }
    _average(string as, NumericType nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
    }

    void operator()(vector<Tuple> &tupset, Tuple &out){
      float *valptr;
      vector<string>::iterator a;
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (outsch.isAttribute("avg"+attrs[i])) {
          valptr = (float *) out.get("avg"+attrs[i]);
          *valptr = average(attrs[i], tupset);
        }
      }
    }

    float average(string attr, vector<Tuple> &tupset) {
      NumericType total=0;
      NumericType val;
      int count=0;
      for (unsigned int i=0; i<tupset.size(); i++) {
        val = *(NumericType *) tupset[i].get(attr);
        if (val == null_value) {
          continue; //return null_value;
       	}
        count = count + 1;
        total += val;
      }
      if (count > 0) {
        return total/count;
      } else {
        return null_value;
      }
    }

   Scheme *getOutScheme(Scheme *insch) {
     outsch.clear();
     for (unsigned int i=0; i<attrs.size(); i++) {
       if (insch->isAttribute(attrs[i])) {
         outsch.addAttribute("avg" + attrs[i], FLOAT);
       }
     }
     return &outsch;
   }
  
  private:
    Scheme outsch;
    NumericType null_value;
    vector<string> attrs;
    vector<NumericType> totals;
};

template<typename NumericType>
class _min : public AggregationFunction {

  public:
    _min(string as) : null_value(NULL_VALUE) { 
      split(as, " ;,", attrs); 
    }
    _min(string as, NumericType nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
    }

    void operator()(vector<Tuple> &tupset, Tuple &out) {
      NumericType *valptr;
      vector<string>::iterator a;
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (outsch.isAttribute("min"+attrs[i])) {
          valptr = (NumericType *) out.get("min"+attrs[i]);
          *valptr = min(attrs[i], tupset);
        }
      }
    }
  
    NumericType min(string attr, vector<Tuple> &tupset) {
      if (tupset.size() == 0) return null_value;
      NumericType minv = *(NumericType *) tupset[0].get(attr);
      NumericType v;
      for (unsigned int i=0; i<tupset.size(); i++) {
        v = *(NumericType *) tupset[i].get(attr);
	if (v == null_value) continue; //return null_value;
        if (minv > v) minv = v;
      }
      return minv;
    }
  
    Scheme *getOutScheme(Scheme *insch) {
      outsch.clear();
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (insch->isAttribute(attrs[i])) {
          outsch.addAttribute("min" + attrs[i], FLOAT);
        }
      }
      return &outsch;
    }

  private:
    Scheme outsch;
    NumericType null_value;
    vector<string> attrs;
    vector<NumericType> totals;
};


template<typename NumericType>
class _max : public AggregationFunction {

  public:
    _max(string as) : null_value(NULL_VALUE) {
      split(as, " ;,", attrs);
    }

    _max(string as, NumericType nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
    }

    void operator()(vector<Tuple> &tupset, Tuple &out) {
      NumericType *valptr;
      vector<string>::iterator a;
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (outsch.isAttribute("max"+attrs[i])) {
          valptr = (NumericType *) out.get("max"+attrs[i]);
          *valptr = max(attrs[i], tupset);
        }
      }
    }
  
    NumericType max(string attr, vector<Tuple> &tupset) {
      if (tupset.size() == 0) return null_value;
      NumericType maxv = *(NumericType *) tupset[0].get(attr);
      NumericType v;
      for (unsigned int i=0; i<tupset.size(); i++) {
        v = *(NumericType *) tupset[i].get(attr);
	if (v == null_value) continue; //return null_value;
        if (maxv < v) maxv = v;
      }
      return maxv;
    }
    
    Scheme *getOutScheme(Scheme *insch) {
      outsch.clear();
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (insch->isAttribute(attrs[i])) {
          outsch.addAttribute("max"+attrs[i], insch->getType(attrs[i]));
        }
      }
      return &outsch;
    }
  private:
    Scheme outsch;
    NumericType null_value;
    vector<string> attrs;
    vector<NumericType> totals;
};

class Count : public AggregationFunction {

  public:

  void operator()(vector<Tuple> &tupset, Tuple &out) {
    int *valptr;
    valptr = (int *) out.get("count");
    *valptr = tupset.size();
  }
  
  Scheme *getOutScheme(Scheme */*insch*/) {
    outsch.clear();
    outsch.addAttribute("count", INT);
    return &outsch;
  }
  private:
    Scheme outsch;
    vector<string> attrs;
  
};

/*
class makepolygf : public AggregationFunction {
  
  public:
  int dim;
  makegridfield(int d) { dim = d; }

  void operator()(vector<Tuple> &tupset, Tuple &out) {
    int s = tupset.size();

    if (s==0) return;
    Grid *g = new Grid();
    g->setImplicit0Cells(s); 
    
    for (int i=0; i<tupset.size(); i++) {
    }
  }

  Scheme *getOutScheme(Scheme *insch) {
    Scheme *outsch = new Scheme(*insch);
    outsch->addAttribute("gf", FLOAT);
    return outsch;
  }

}
*/

class area : public AggregationFunction {
  public:
    area(string a) { attr = a; }
   
    void operator()(vector<Tuple> &tupset, Tuple &out) {
      float *area = (float *) out.get(attr);
      if (tupset.size() < 3) *area = 0;
      *area = 0;
      unsigned int i, j;
      for (i=0, j=tupset.size()-1; i<tupset.size(); j = i++) {
        float x1 = *(float *) tupset[j].get("x"); 
        float y1 = *(float *) tupset[j].get("y");
        float x2 = *(float *) tupset[i].get("x");
        float y2 = *(float *) tupset[i].get("y");
        *area += x1*y2 - x2*y1;
      }
      *area =*area/2;
    }
     
    Scheme *getOutScheme(Scheme */*insch*/) {
      outsch.clear();
      outsch.addAttribute(attr, FLOAT);
      return &outsch;
    }
    
  private:
    string attr;
    Scheme outsch;
};

class mkvector : public AggregationFunction {
  public:
    string attr;
    mkvector(string a) { attr = a; }
    
    void operator()(vector<Tuple> &tupset, Tuple &out) {
      //if (tupset.size() > 0) {
      //  tupset[0].print();
      //}

      vector<Tuple> **val = (vector<Tuple> **) out.get(attr);
      if (val == NULL) {
        Fatal("Attribute %s not found during aggregation", attr.c_str());
      }
      
      // how can I clean up this vector?
      *val = new vector<Tuple>(tupset);
      //counted_ptr<vector<Tuple> > p(*val);
/*
      if ((**val).size() > 4) getchar();
      Tuple *t = &(**val)[0];
      cout << "tuple: ";
      cout << *val << endl;
      t->print();
      */
      //cout << "tuple: " << *(float *) (((*val)[0]).get("x")) << endl;
    }
  
    Scheme *getOutScheme(Scheme */*insch*/) {
    outsch.clear();
    outsch.addAttribute(attr, OBJ);
    return &outsch;
  }
  private:
    Scheme outsch;
};


class triweights : public AggregationFunction {
  // UNTESTED
  public:
    
  void operator()(vector<Tuple> &tupset, Tuple &out) {
    unsigned int s = tupset.size();
    float dist[s];
    float x, y;
    float px, py, w[s];
    float sum = 0;
    
    Tuple *t = &out;
    px = *(float *) t->get("x");
    py = *(float *) t->get("y");
    
    for (unsigned int i=0; i<s; i++) { 
      x = *(float *)tupset[i].get("x");
      y = *(float *)tupset[i].get("y");
      dist[i] = euclid(x,y,px,py);
      sum += dist[i];
    }
    for (unsigned int i=0; i<s; i++) { 
      w[i] = (dist[i]/sum);
    }
    out.set("w1", *(UnTypedPtr*) &w[0]);
    out.set("w2", *(UnTypedPtr*) &w[1]);
    out.set("w3", *(UnTypedPtr*) &w[2]);
  }
  
  Scheme *getOutScheme(Scheme *insch) {
    Scheme *outsch = new Scheme(*insch);
    outsch->addAttribute("w1", FLOAT);
    outsch->addAttribute("w2", FLOAT);
    outsch->addAttribute("w3", FLOAT);
    return outsch;
  }
};



template<typename ValueType>
class setunion : public AggregationFunction {
  private:
    Scheme *outsch;
    Scheme *sch;
  public:
    string vecattr;
    string valattr;
    Type type;

    setunion(string a1, string a2, Type t) 
         : vecattr(a1), valattr(a2), type(t) { 
      this->outsch = new Scheme();
      outsch->addAttribute(vecattr, OBJ);
      this->sch = new Scheme();
      sch->addAttribute(valattr, type);
    };

    virtual void operator()(vector<Tuple> &tupset, Tuple &out) {
      
      vector<Tuple>::iterator p,q;

      map<ValueType, UnTypedPtr> outset;
      vector<Tuple> *clump;
      UnTypedPtr ptr;
      for (p=tupset.begin(); p!=tupset.end(); p++) {
        clump = (vector<Tuple> *) (*p).get(vecattr);
        for (q=clump->begin(); q!=clump->end(); q++) {
          ptr = (*q).get(valattr);
          outset[*(ValueType *) ptr] = ptr;
        }
      }     

      typename map<ValueType, UnTypedPtr>::iterator j;

      // guaranteed leak - no way to clean up this memory.
      // Aggregate operator and the type system needs a
      // major overhaul
      vector<Tuple> *unioned = new vector<Tuple>;
      Tuple t(sch);
      
      for (j=outset.begin(); j!=outset.end(); j++) {
        t.set(valattr,(*j).second);
        unioned->push_back(t);
      }
      out.set(vecattr, unioned);
    }
  
  Scheme *getOutScheme(Scheme *insch) {
    return outsch;
  }
};


class interpolate3D : public AggregationFunction {
  // simple linear interpolation 
  // assumes attributes include x,y,z,<attr>
  // where <attr> is a scalar to be interpolated
  public:
  vector<string> attrs;
  interpolate3D(string a) { 
    split(a, ";, :/-", attrs); 
  }

  interpolate3D(vector<string> &as) {
    attrs.insert(attrs.begin(), as.begin(), as.end());
  }
    
  void operator()(vector<Tuple> &tupset, Tuple &out);
  
  Scheme *getOutScheme(Scheme */*insch*/) {
    outsch.clear();
    for (unsigned int j=0; j<attrs.size(); j++) {
      outsch.addAttribute(attrs[j], FLOAT);
    }
    return &outsch;
  }

  private:
    Scheme outsch;
};


class interpolate2D : public AggregationFunction {
  // simple linear interpolation 
  // assumes attributes include x,y,<attr>
  // where <attr> is a scalar to be interpolated
  public:
  string attr;
  interpolate2D(string a) { attr = a; }
    
  void operator()(vector<Tuple> &tupset, Tuple &out) {
    unsigned int s = tupset.size();
    float dist[s];
    float x, y, v[s];
    float px, py, *val;
    float sum = 0;
    
    Tuple *t = &out;
    px = *(float *) t->get("x");
    py = *(float *) t->get("y");
    
    val = new float(0);
    int null = -99;
    if (s==0) out.set(attr, &null);
    
    for (unsigned int i=0; i<s; i++) { 
      x = *(float *)tupset[i].get("x");
      y = *(float *)tupset[i].get("y");
      v[i] = *(float *)tupset[i].get(attr);
      dist[i] = euclid(x,y,px,py);
      sum += dist[i];
//      cout << x << ", " << y << ", " << v[i] << ", " << dist[i] << ", " << sum << endl;
    }
    for (unsigned int i=0; i<s; i++) { 
      *val += (dist[i]/sum) * v[i];
    }
    
    
    out.set(attr, val);
    //cout << "val: " << *(float*)out.get(attr) << endl;
  }
  
  Scheme *getOutScheme(Scheme */*insch*/) {
    Scheme *outsch = new Scheme();
    outsch->addAttribute(attr, FLOAT);
    return outsch;
  }
};

template<typename NumericType>
class interpolate1D : public AggregationFunction {
  public:
    interpolate1D(string xT, string xS, string vs)
	     : x_attrT(xT), x_attrS(xS), null_value(NULL_VALUE) {
      split(vs, " ;,", attrs); 
    }
    interpolate1D(string xT, string xS, string vs, NumericType null) 
	     : x_attrT(xT), x_attrS(xS), null_value(null) {
      split(vs, " ;,", attrs); 
    }

    void operator()(vector<Tuple> &tupset, Tuple &out) {
      NumericType *valptr;
      NumericType xT = *(NumericType *)out.get(x_attrT);
      NumericType foo = *(NumericType *)out.get("hpos");
      
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (outsch.isAttribute(attrs[i])) {
          valptr = (NumericType *) out.get(attrs[i]);
          *valptr = interp(xT, attrs[i], tupset, foo);
        }
      }
    }
    
    NumericType interp(NumericType xT, string attr, vector<Tuple> &tupset, NumericType foo) {
	    
      NumericType total=0;
      NumericType answer=0;
      vector<NumericType> weight(tupset.size());
      vector<NumericType> val(tupset.size());
      vector<NumericType> x(tupset.size());
      
      // inverse-distance interpolation
      for (unsigned int i=0; i<tupset.size(); i++) {
        x[i]   = *(NumericType *)tupset[i].get(x_attrS);
	val[i] = *(NumericType *)tupset[i].get(attr);
	
        weight[i] = 1 / abs(xT-x[i]);
	
	if (weight[i] > (1/TOLERANCE)) return val[i];
	
        total += weight[i];
      }
      
      // linear regression
      if (tupset.size() == 2) {
        //cout << "INTERPOLATE: " << xT << ", " << val[1] << ", " << val[0] << ", " << x[1] << ", " << x[0] << endl;
        return (val[1] - val[0])/(x[1] - x[0]) * (xT-x[0]) + val[0];
      }
      
      for (unsigned int i=0; i<tupset.size(); i++) {
        answer += (weight[i]/total) * val[i];
      }   
      
      if (tupset.size() == 0) answer = null_value;

      return answer;
    }
    
    Scheme *getOutScheme(Scheme *insch) {
      outsch.clear();
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (insch->isAttribute(attrs[i])) {
          outsch.addAttribute(attrs[i], insch->getType(attrs[i]));
        }
      }
      return &outsch;
    }
    
  private:
    Scheme outsch;
    string x_attrT, x_attrS;
    NumericType null_value;
    vector<string> attrs;
};


class first : public AggregationFunction {
  public:
    first(string as) : null_value(NULL_VALUE) { 
      split(as, " ;,", attrs); 
      all = false;
    }
    first(string as, float nv) : null_value(nv) { 
      split(as, " ;,", attrs); 
      all = false;
    }
    first(float nv) : null_value(nv) { 
      all = true;
    }
    first() : null_value(0) { 
      all = true;
    }

  void operator()(vector<Tuple> &tupset, Tuple &out) {
    for (unsigned int i=0; i<attrs.size(); i++) {
      float *v = (float *) out.get(attrs[i]);
      if (tupset.size() == 0) {
        *v = null_value;
       } else {
        *v = *(float *) tupset[0].get(attrs[i]);
       }
    }
  }
  
  Scheme *getOutScheme(Scheme *insch) {
      outsch.clear();
      if (all) {
        attrs.clear();
        for (unsigned int i=0; i<insch->size(); i++) {
          attrs.push_back(insch->getAttribute(i));
	}
      }
      for (unsigned int i=0; i<attrs.size(); i++) {
        if (insch->isAttribute(attrs[i])) {
          outsch.addAttribute(attrs[i], insch->getType(attrs[i]));
        }
      }
      return &outsch;
  }
  private:
    bool all;
    Scheme outsch;
    float null_value;
    vector<string> attrs;
};

class statistics : public AggregationFunction {
  //compute max, min, sum, avg for a given atttribute
  //this version only works for ints
  
  string attr;
  
  public:
  statistics(string att) { attr = att; };
  
  void operator()(vector<Tuple> &tupset, Tuple &out) {

    typedef int valtype;
    
    valtype max;
    valtype min;
    valtype sum;
    valtype avg;
    unsigned int cnt = tupset.size();
    valtype v;

    //assert(tupset.size() > 0);
    if (tupset.size() < 1) {
      max = min = sum = 0;
      cnt = 1;
    } else {
      max = min = sum = *(valtype *) tupset[0].get(attr);
    }
    for (unsigned int i=1; i<cnt; i++) {
      v = *(valtype *) tupset[i].get(attr);
      sum += v;
      if (max < v) max = v;
      if (min > v) min = v;
    }
    avg = sum/cnt;
    out.set("max"+attr, new valtype(max));
    out.set("min"+attr, new valtype(min));
    out.set("sum"+attr, new valtype(sum));
    out.set("avg"+attr, new valtype(avg));
    out.set("cnt"+attr, new int(cnt));
  }
  
  Scheme *getOutScheme(Scheme */*insch*/) {
    Scheme *sch = new Scheme();
    sch->addAttribute("max"+attr, FLOAT);
    sch->addAttribute("min"+attr, FLOAT);
    sch->addAttribute("sum"+attr, FLOAT);
    sch->addAttribute("avg"+attr, FLOAT);
    sch->addAttribute("cnt"+attr, INT);
    return sch;
  }
};

class project : public AggregationFunction {
  
  public:
  std::vector<string> attrs; 
  project(std::vector<string> att) { attrs = att; };
  project(string str_attrs) { split(str_attrs, "; ,", attrs); };
  
  void operator()(vector<Tuple> &tupset, Tuple &out) {
    assert(tupset.size() == 1);
    for (unsigned int i=0; i<attrs.size(); i++) {
      out.set(attrs[i], tupset[0].get(attrs[i]));
    }
  }
  
  Scheme *getOutScheme(Scheme *insch) {
    Scheme *sch = new Scheme();
    for (unsigned int i=0; i<attrs.size(); i++) {
      sch->addAttribute(attrs[i], insch->getType(attrs[i]));
    }
    return sch;
  }
  
};


/*
class alter : public AggregationFunction {
 public:
  alter() {
  }
};
*/
}
