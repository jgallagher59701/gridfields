#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkGridField.h"
#include "vtkCellType.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"

#include <sstream>
#include "timing.h"
#include "array.h"
#include "constarray.h"
#include "expr.h"

vtkGridField *vtkGridField::New() {
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkGridField");
  if(ret) {
   vtkGridField* recast = (vtkGridField*) ret;
   recast->visdim=-1;
   return recast;
  }
  vtkGridField* recast = new vtkGridField();
  recast->visdim=-1;
  return recast;
};


void vtkGridField::Update() {  
  if ( this->GetOutput() ) {
    this->GetOutput()->Update();
  } else {
    this->Execute();
  }

};

void vtkGridField::Execute() { 
  //  this->SetOutput(Convert(gf)); 
  Convert(this->GetOutput());
};

void vtkGridField::ValidateForConversion() {

  //assert(gfpts->GetGrid() == gfdata->GetGrid());
  //cout << gf->rank() << endl;

  //assert(gfdata->arity() + gfpts->arity() - G->dim() < MAX_VTK_ATTRS);
  // assert(gf->getarity() - gf->GetGrid()->getdim() < MAX_VTK_ATTRS);
  //assert(gf->isAttribute(this->activeAttribute.c_str()));
  
  Scheme sch = gf->GetScheme(0);

  if (NamedPerspective()) {
    int geometry_attrs[3] = {'x', 'y', 'z'};
    int count = 0;
    
    // make sure we have at least dim geometry attributes
    for (int i=0; i<3; i++) {
      string a(1, geometry_attrs[i]);
      if ((gf->IsAttribute(0, a)) && sch.getType(a.c_str()) == FLOAT) count ++;
    }
    
    int dim = gf->GetGrid()->getdim();
    if (count < dim) {
      Fatal("Need at least %i float attributes in the set {%c, %c, %c}", dim, 'x', 'y', 'z');
    }
/*
    switch (gf->GetGrid()->getdim()) {
    case 0:
      break;
    case 1:
      assert(gf->isAttribute("x"));
      assert(sch->getType("x") == FLOAT);
      break;
    case 2:
//      gf->getScheme()->print();
      assert(gf->isAttribute("x"));
      assert(sch->getType("x") == FLOAT);
      assert(gf->isAttribute("y"));
      assert(sch->getType("y") == FLOAT);
      break;
    case 3:
      assert(gf->isAttribute("x"));    
      assert(sch->getType("x") == FLOAT);
      assert(gf->isAttribute("y"));
      assert(sch->getType("y") == FLOAT);
      assert(gf->isAttribute("z"));
      assert(sch->getType("z") == FLOAT);
      break;
    default:
      assert(false);
    }
  */  
  } else {
    
    assert(gf->Arity(0) >= gf->GetGrid()->getdim());
    for (int i=0; i<gf->GetGrid()->getdim(); i++) {
      assert(sch.getType(sch.getAttribute(i)) == FLOAT);
    }
  }

}

int vtkGridField::GuessVisualizationDimension(GridField *gfpts)
  {
  Grid *G = gfpts->GetGrid();
  int Dim=G->getdim();   
  {if(gfpts->Arity(Dim)>0){visdim=Dim;}
   else {visdim=0;}
  }


  return visdim;
  }

void vtkGridField::AttachAttributes(GridField *gfpts, Dim_t k, vtkDataSetAttributes *arrays)
  {vtkDataArray *dat;
   
    float v;
  for (int i=0; i<gfpts->Arity(k); i++) {
  Scheme sch = gfpts->GetScheme(k);

  string attr = sch.getAttribute(i);
  vtkDataArray *  dat = MakeDataArray(gfpts,k, attr);
    arrays->AddArray(dat);
    dat->Delete();
  }

  arrays->SetActiveScalars(this->activeAttribute.c_str());

  if (this->vectorAttribute1 != "" && this->vectorAttribute2 != "") {

      vtkDataArray *vectors 
              = this->MakeVectorArray(gfpts, k,
			             this->vectorAttribute1, 
			             this->vectorAttribute2,
				     this->vectorAttribute3);
    
    
    arrays->AddArray(vectors);
    arrays->SetActiveVectors(vectors->GetName());
  }

}

vtkUnstructuredGrid *vtkGridField::MakeGrid() {
  Execute();
  return this->GetOutput();
}

/*
 * Converts a GridField to a vtkUnstructuredGrid. 
 * Assumes that the ValidForConversion method has been called.
 */

void vtkGridField::SetVisualizationDimension(Dim_t k)
{
 this->visdim = k;
}

void vtkGridField::Convert(vtkUnstructuredGrid *vtkgrid) {
  GridField *gfpts = gf;
  ValidateForConversion();
  Grid *G = gfpts->GetGrid();
  Scheme sch = gfpts->GetScheme(0);
  int card = gfpts->Size(0);
  
  //************* points
  vtkPoints *vtkpts = vtkPoints::New();
  AbstractCellArray *zerocells = G->getKCells(0);
  vtkpts->SetNumberOfPoints(card);

//  if (!gfpts->IsAttribute(0, this->activeAttribute)) {
//    stringstream ss;
//    ss << "vtkGridField::Convert : '" << this->activeAttribute;
//    ss << "' is not an attribute of this gridfield. \n";
//    gfpts->GetScheme(0).PrintTo(ss, 0);
//    Fatal(ss.str().c_str()); 
//  }
  
  string x_attr, y_attr, z_attr;
  if (NamedPerspective()) {
    x_attr = "x";
    y_attr = "y";
    z_attr = "z";
  } else {
    x_attr = sch.getAttribute(0);
    y_attr = sch.getAttribute(1);;
    z_attr = sch.getAttribute(2);;
  }

  ConstArray zeros("zero", card, 0.0f);
  
  Array *x, *y, *z;
  // set x values
  if (gfpts->IsAttribute(0, x_attr)) {
    x = gfpts->GetAttribute(0, x_attr);
  } else {
    x = &zeros;
  }

  // set y values
  if (gfpts->IsAttribute(0, y_attr)) {
    y = gfpts->GetAttribute(0, y_attr);
  } else {
    y = &zeros;
  }

  // set z values
  if (gfpts->IsAttribute(0, z_attr)) {
    z = gfpts->GetAttribute(0, z_attr);
  } else {
    z = &zeros;
  }
  
  // make the vtk points
  for (int i=0; i<zerocells->getsize(); i++) {    
    //cout << i << ", " << x[i] << ", " << y[i] << ", " << z[i] << endl;
    vtkpts->SetPoint(i, *(float *) x->getValPtr(i), 
                        *(float *) y->getValPtr(i),
                        *(float *) z->getValPtr(i));
  }

  vtkgrid->SetPoints(vtkpts);
  vtkpts->Delete();
  // ************* cells
  AbstractCellArray *cells;

  vtkCellArray *vtkcells = vtkCellArray::New();

  Cell *c;

  vtkIntArray *celltypes = vtkIntArray::New();
  
  int type=-1;
  for (int d=1; d<=G->getdim(); d++) {
    cells = G->getKCells(d);
    for (int i=0; i<cells->getsize(); i++) {
      c = cells->getCell(i);
      type = guessType(c, d);
      repairCell(c, type);
      //sortby(*c, gfpts, "x");
      int nodecount = c->getsize();
      Node *nodes = c->getnodes();
      vtkcells->InsertNextCell(nodecount);
      for (int j=0; j<nodecount; j++) {
        vtkcells->InsertCellPoint(nodes[j]);
      }
      celltypes->InsertNextValue(type);
    }
  }
      int nodes[10];
      vtkCell *cc;
      int sz;
  vtkgrid->SetCells(celltypes->GetPointer(0), vtkcells);
  //cout << vtkgrid->GetNumberOfCells() << endl;
      //for (int p=0; p<vtkgrid->GetNumberOfCells(); p++) {
       // cc = vtkgrid->GetCell(p);
       // sz = cc->GetNumberOfPoints();
        //cout << "points: " << sz << endl;
        //cout << sz << ", " << cc->GetCellType() << ": ";

        //for (int q=0; q<sz; q++) {
        //  cout << cc->GetPointId(q) << ", ";
        //}
        
        //cout << endl;
     // }
  celltypes->Delete();
  vtkcells->Delete();
  // ********data
  //assert(gfpts->k == 0); //for now

  string attr;
  float v;

  vtkDataSetAttributes* arrays;

  if (visdim==-1)
  {visdim=GuessVisualizationDimension(gfpts);}
  if (visdim == 0) {cout<<"ahoy"<<endl;
    arrays = vtkgrid->GetPointData();
    AttachAttributes(gfpts, 0, arrays);
    } else if (visdim == G->getdim()) {
    arrays =  (vtkDataSetAttributes *) vtkgrid->GetCellData();
     AttachAttributes(gfpts, G->getdim(), arrays);
    } else {
    Fatal("VTK can only display gridfields with rank 0 or dim(G).");
  }

 


  //vtkgrid->PrintSelf(cout, 0);
}

int vtkGridField::guessType(Cell *c, int dim) {
  int defaulttypes[4] = {VTK_VERTEX, VTK_LINE, VTK_TRIANGLE, VTK_WEDGE};
 
  if (dim > 3) {
    cerr << "dimension is too high: " << dim << endl;
    exit(2);
  }

  switch (dim) {
    case 0:
      return VTK_VERTEX;
    case 1: 
      switch (c->getsize()) {
        case 2:
          return VTK_LINE;
        default:
          return VTK_POLY_LINE;
      }
      
    case 2:
      switch (c->getsize()) {
        case 3:
          return VTK_TRIANGLE;
        case 4:
          return VTK_QUAD;
        case 6:
          return VTK_POLYGON;
      }

    case 3:
      switch (c->getsize()) {
        case 4:
          return VTK_TETRA;
        case 5:
          return VTK_PYRAMID;
        case 6: 
          return VTK_WEDGE;
        //case 8: 
        //  return VTK_VOXEL;
        case 8: 
          return VTK_HEXAHEDRON;
        case 10: 
          return VTK_PENTAGONAL_PRISM;
        case 12: 
          return VTK_HEXAGONAL_PRISM;
      }
  }
  
  return defaulttypes[dim];
}

void vtkGridField::repairCell(Cell *c, int type) {
  /*
  if (type == VTK_HEXAHEDRON) {
    vector<int> order;
    order.reserve(8);
    order[0] = 0;
    order[1] = 1;
    order[2] = 2;
    order[3] = 3;
    order[4] = 4;
    order[5] = 5;
    order[6] = 6;
    order[7] = 7;
    reorderNodes(*c, order);
  }
  */
 /* 
  if (type == VTK_WEDGE) {
    vector<int> order;
    order.reserve(4);
    order[0] = 0;
    order[1] = 1;
    order[2] = 2;
    order[3] = 3;
    order[4] = 4;
    order[5] = 5;
    reorderNodes(*c, order);
  }
  */
  /*
  if (type == VTK_QUAD) {
    vector<int> order;
    order.reserve(4);
    order[0] = 0;
    order[1] = 1;
    order[2] = 3;
    order[3] = 2;
    reorderNodes(*c, order);
  }
  */
}

void vtkGridField::reorderNodes(Cell &c, vector<int> &order) {
  Node temp[c.getsize()];
  int i;
  
  //cout << "cell..." << endl; 
  //c.print();
  for (i=0; i<c.getsize(); i++) {
    temp[i] = c.getnodes()[order[i]];
  }
  for (i=0; i<c.getsize(); i++) {
    c.getnodes()[i] = temp[i];
  }
  //c.print();
}

void vtkGridField::sortby(Cell &c, GridField *gf, string attr) {
  multimap<float, int> sorted;
  
  multimap<float, int>::iterator p;
  int i=0;
  for (i=0; i<c.getsize(); i++) {
    sorted.insert(
        pair<float, int>(
          *(float *)gf->GetAttributeValue(0, attr, c.getnodes()[i]), 
          i
          ));
  }
//  cout << "cell..." << endl; 
//  c.print();
  vector<int> order;
  order.reserve(c.getsize());
  
  for (p=sorted.begin(),i=0; p!=sorted.end(); p++,i++) {
//    cout << (*p).first << ", " << (*p).second << endl;
    order[i] = (*p).second;
  }
  reorderNodes(c, order);
}

void vtkGridField::UpdateScalars(vtkUnstructuredGrid *ug, 
                                        GridField *gf, Dim_t k, char *attr) {
  Array *a = gf->GetAttribute(k, attr);
  a->print();
  UnTypedPtr utp = a->getVals();  cout << attr << ", ";
  vtkDataArray *sclrs = ug->GetPointData()->GetScalars();
  cout <<  "hi" << attr << ", ";
  sclrs->PrintSelf(cout, vtkIndent(5));
  if (sclrs) {
    sclrs->SetVoidArray(utp, k, 1);
    sclrs->SetName(attr);
  }
}

vtkDataArray *vtkGridField::MakeVectorArray(GridField *gfpts, Dim_t k,
		                            string u, 
					    string v, 
					    string w) {

    // Third component can be an empty string to indicate a zero vector  
    // This is a bit ugly.  Recommend providing a list of attributes that
    // become vector components rather than just 2 or 3 where 2 is a hacked
    // special case of 3.

    int sz = gfpts->Size(k);
    vtkDataArray *dat;
    Type tu = gf->GetScheme(k).getType(u);
    Type tv = gf->GetScheme(k).getType(v);
    Type tw;
    if (w.empty()) {
      tw = FLOAT;
    } else {
      tw = gf->GetScheme(k).getType(w);
    }

    if (tu != tv) {
      Fatal("Both attributes must be the same type to form a vector.");
    }

    if (tu == FLOAT) {
      dat = (vtkDataArray *) vtkFloatArray::New();
    } else {
      dat = (vtkDataArray *) vtkIntArray::New();
    }
    
    dat->SetNumberOfComponents(3);
    dat->SetNumberOfTuples(sz);
    dat->SetName((char *)(u + v + w).c_str());


    for (int i=0; i<sz; i++) {
      float uval = *(float *) gfpts->GetAttributeValue(k, u.c_str(), i); 
      float vval = *(float *) gfpts->GetAttributeValue(k, v.c_str(), i);
      
      float wval = 0;
      if (w.empty()) {
        wval = 0;
      } else {
        wval = *(float *) gfpts->GetAttributeValue(k, w.c_str(), i);
      }
      dat->SetTuple3(i, uval, vval, wval); 
    }
    
    return dat;
}

vtkDataArray *vtkGridField::MakeDataArray(GridField *gfpts, Dim_t k, string attr) {
    
    vtkDataArray *dat;
    if (gf->GetScheme(k).getType(attr) == FLOAT) {
      dat = (vtkDataArray *) vtkFloatArray::New();
    } else {
      dat = (vtkDataArray *) vtkIntArray::New();
    }

    dat->SetName((char *)attr.c_str());
    dat->SetVoidArray(gfpts->GetAttribute(k, attr)->getVals(), gfpts->Size(k), 1);
    //gfpts->getAttribute(attr.c_str())->print();
    return dat;
}

vtkDataArray *vtkGridField::CopyDataArray(GridField *gfpts, Dim_t k, string attr) {
    
    if (gf->GetScheme(k).getType(attr) == FLOAT) {
      float vf;
      vtkFloatArray *datf = vtkFloatArray::New();
    
      datf->SetNumberOfTuples(gfpts->Size(k));
      datf->SetName((char *)attr.c_str());

      for (int j=0; j<gfpts->Size(k); j++) {
        vf = *(float *) gfpts->GetAttributeValue(0, attr, j);
        datf->SetValue(j,vf);
      }
      return (vtkDataArray *) datf;
    } else {
      int vi;
      vtkIntArray *dati = vtkIntArray::New();
      
      int n = gfpts->Size(k);
      dati->SetNumberOfTuples(n);
      dati->SetName((char *)attr.c_str());
       
      for (int j=0; j<n; j++) {
        vi = *(int *) gfpts->GetAttributeValue(k, attr, j);
        dati->SetValue(j,vi);
      }
      return (vtkDataArray *) dati;
    }
}
vtkGridField::~vtkGridField() {
  this->GetOutput()->GetCells()->Delete();
  this->GetOutput()->GetPoints()->Delete();
  this->GetOutput()->Delete();
}
