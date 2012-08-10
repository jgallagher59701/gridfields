#include "transect.h"
#include "include.h"
#include "sift.h"
#include "vtkUnstructuredGridWriter.h"

using namespace GF;

//defaults

string filename("1_salt.63");
string stationfilename("vslice_mchann.bp");

int timestep=0;

string expr;

template <class T>
bool from_string(T &t, const std::string &s) {
  std::istringstream iss(s);
  return !(iss >> std::dec >> t).fail();
};

void checkopts( int argc, char **argv );
void showhelp();

int main( int argc, char *argv[] ) {
  
  checkopts( argc, argv );
  
  VerticalSlice();
}


void checkopts( int argc, char **argv )
{
        string emptymsg("supply at least an elcirc file ( -f <filename> )");
        
        CCmdLine cmdline;
        if (cmdline.SplitLine(argc, argv) < 1) {
          showhelp();
          exit(-1);
        }
        
        try {
          filename = cmdline.GetArgument("-f", 0);
        } catch (...) {
          showhelp();
          exit(-1);
        }
        from_string<string>(stationfilename, cmdline.GetSafeArgument("-s", 0, "vslice_mchann.bp"));
        from_string<int>(timestep, cmdline.GetSafeArgument("-t", 0, "0"));
}


GridField *getUserGrid(string fn) {

  ifstream f;
  f.open(fn.c_str(), ios::in);
  if (f.fail()) {
    cerr << "Unable to open " << fn << endl;
    exit(1);
  }

  int s; 
  char name[256];
  
  f.getline(name,256);
  
  f >> s;

  float *x = new float[s];
  float *y = new float[s];
  float z;
  int id;
  
  for (int i=0; i<s; i++) {
    f >> id >> x[i] >> y[i] >> z;
  }
  f.close(); 
  
  Grid *P = new OneGrid("stations", s);
  GridField *Pn = new GridField(P, 0);
 
  Array *arrx = new Array("x", FLOAT);
  Array *arry = new Array("y", FLOAT);
  
  arrx->shareData(x, s);
  arry->shareData(y, s);
  
  Pn->Bind(arrx);
  Pn->Bind(arry);
  return Pn;
}

void View(GridFieldOperator *op, string attr, GridFieldOperator *bottom=NULL) {
 
 
  GridField *gf = op->getResult();
  //cout << gf->grid->getdim() << endl;
  //gf->print();
  //cout << gf->rank() << endl;
  //cout << gf->card() << endl;

  vtkGridField *vtkgrid = toVTK(gf, attr.c_str());

  vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
  writer->SetInput(vtkgrid->GetOutput());
  writer->SetFileName("output.vtk");
  writer->Write();
  
  vtkRenderWindow *renWin = vtkRenderWindow::New();
//  Visualize(vtkgrid->GetOutput(), "", renWin);
  if (bottom == NULL) {
    DirectVis(vtkgrid->GetOutput(), renWin);   
  } else {
    vtkGridField *vtkbottom = toVTK(bottom->getResult(), "h");
  vtkUnstructuredGridWriter *b_writer = vtkUnstructuredGridWriter::New();
  b_writer->SetInput(vtkbottom->GetOutput());
  b_writer->SetFileName("bottom.vtk");
  b_writer->Write();
  
    DirectVis(vtkgrid->GetOutput(), vtkbottom->GetOutput(),renWin);   
  }
  
}

void VerticalSlice() {
  ElcircFile ef(filename);
  
  GridField *H = ef.readHGrid();
  GridField *V = ef.readVGrid();
  
  GridField *Pn = getUserGrid(stationfilename);

  GridFieldOperator *scalez = new ApplyOp::ApplyOp(V, "z","z*60"); 
  GridFieldOperator *vpos = 
        new AccumulateOp::AccumulateOp(scalez, "vpos", "vpos+1", "0");

  Grid *gV0 = new Grid("V0");
  gV0->setImplicit0Cells(V->card()-1);
  
  GridField *V0 = new GridField(gV0, 0);
  GridFieldOperator *vmerge = new MergeOp::MergeOp(V0, vpos);
  
  GridFieldOperator *hpos = new AccumulateOp::AccumulateOp(H, "hpos", "hpos+1", "0");

  
  stringstream ss;
  ss << "addr+" << V->card() << "-b";
  AccumulateOp *haddr = new AccumulateOp::AccumulateOp(hpos, "addr", ss.str(), "0");
  haddr->SetOffset(-1);

  FileArrayReader *surfrdr = ef.getSurfReader(timestep, "");
  GridFieldOperator *bindsurf = new BindOp::BindOp("surf", INT, surfrdr, haddr);
  GridFieldOperator *mkconst = new ApplyOp::ApplyOp(bindsurf, "const=0; foo=x+y*4");
  
  
  GridField *H2 = new GridField(H->grid, 2);
  
  Assign::Nodes nds;
  Aggregate::mkvector clump("poly");
  GridFieldOperator *poly = new AggregateOp::AggregateOp(H2, &nds, &clump, bindsurf);

  Assign::pointpoly3 pp3;
  Aggregate::_count cnt;
  GridFieldOperator *map = new AggregateOp::AggregateOp(poly, &pp3, &cnt, Pn);

  GridFieldOperator *losepolys = new RestrictOp::RestrictOp("count>0", map);  

  GridFieldOperator *sift = new SiftOp::SiftOp(losepolys);

  GridFieldOperator *merge = new MergeOp::MergeOp(mkconst, sift);

  
  GridFieldOperator *cross = new CrossOp::CrossOp(merge, vmerge);
  
  GridFieldOperator *cutbottom = new RestrictOp::RestrictOp("b<vpos", cross);  

  GridFieldOperator *addr3d = new ApplyOp::ApplyOp(cutbottom, "addr2","addr+vpos-b"); 

  FileArrayReader *varrdr = ef.getVariableReader(timestep, "addr2");
  GridFieldOperator *bindvar = new BindOp::BindOp("var", FLOAT, varrdr, addr3d);

  //GridField *g = bindvar->getResult();

  //GridFieldOperator *mappolys = new AggregateOp::AggregateOp();
  
  Grid *gP1 = new OneGrid("P1", Pn->card());
  GridField *P1 = new GridField(gP1, 0);
  P1->Bind(Pn->getAttribute("x"));
  P1->Bind(Pn->getAttribute("y"));

  /*
  Assign::pointpoly2 pp2;
  vector<string> attrs;
  attrs.push_back("hpos");
  attrs.push_back("b");
  attrs.push_back("surf");
  Aggregate::project proj(attrs);
  Aggregate::_max<int> maxb("b");
  Aggregate::_min<int> minsurf("surf");
 
  Assign::neighbors neigh;
  AggregateOp *Hslice2 = new AggregateOp(Hslice2, neigh, maxb, Hslice0);
  Hslice2  = MergeOp::Merge(Hslice2, 
      AggregateOp::Aggregate(Hslice2, neigh, minsurf, Hslice0));
  Hslice2->getScheme()->print();
  Pn = new AggregateOp(Pn, pp2, proj, Hslice2));
  */
  
  GridFieldOperator *crossPV = new CrossOp::CrossOp(P1, vpos);

 
  GridFieldOperator *conster = new ApplyOp::ApplyOp(crossPV, "const", "0");

  Assign::pointpoly pp;
  Assign::match m("vpos");
  Assign::Both both(m, pp);
  Aggregate::interpolate3D inter("var");
  bindvar->getResult()->grid->normalize();
  
  AggregateOp *mapback = new AggregateOp(crossPV, &both, &inter, bindvar);
  mapback->getResult()->getScheme()->print();
  mapback->getResult()->getAttribute("var")->print();
  RestrictOp *filter = new RestrictOp("var>0", mapback);
  
  View(filter, "var", NULL);

//  GridFieldOperator *cutbottom2 = new RestrictOp::RestrictOp("b<vpos", crossPV);  
  
  stringstream zoomexpr;
  float xmin = 320000;
  float xmax = 355000;
  float ymin = 280000;
  float ymax = 310000;
    
  zoomexpr << "(" << xmin << "<x)&(x<" << xmax << ")";
  zoomexpr << "&";
  zoomexpr << "(" << ymin << "<y)&(y<" << ymax << ")";
  GridFieldOperator *rzoom = 
        new RestrictOp::RestrictOp(zoomexpr.str(), H);

  GridFieldOperator *inverth = new ApplyOp::ApplyOp(rzoom, "z", "60*(4825.1-h)");
  
  GridFieldOperator *scalezdown = new ApplyOp::ApplyOp(conster, "z","z/60"); 

}

void showhelp() {
  cout 
    << "Usage: "
    << endl << "transect -f <data filename> "
    << endl << "         -s <transect filename> "
    << endl << "        [-t <int>  (timestep=0)] "
    << endl;
}
