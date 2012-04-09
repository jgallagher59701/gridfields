#include "include.h"
#include "isolines.h"
#include "sift.h"
#include "subapply.h"
#include "changerank.h"

//defaults

string filename("1_salt.63");
string transfilename("../tests/reference/vslice_mchann.bpclean");

float x=0;
float y=0;
float z=0;

int depth=0;
int timestep=0;

string expr;

template <class T>
bool from_string(T &t, const std::string &s) {
  std::istringstream iss(s);
  return !(iss >> std::dec >> t).fail();
};

void Extract_Station(GridField *H, GridField *V) ;
void checkopts( int argc, char **argv );
void showhelp();

int main( int argc, char *argv[] ) {
  
  checkopts( argc, argv );
  
  GridField *H = readHGrid(filename.c_str());
  GridField *V = readVGrid(filename.c_str());
  Extract_Station(H, V);
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
        from_string<float>(x, cmdline.GetSafeArgument("-x", 0, "320000"));
        from_string<float>(y, cmdline.GetSafeArgument("-y", 0, "280000"));
        from_string<float>(z, cmdline.GetSafeArgument("-z", 0, "4820"));
}        


void View(GridFieldOperator *op, string attr) {
 
  GridField *gf = op->getResult();
  cout << gf->grid->getdim() << endl;
  gf->print();
  cout << gf->rank() << endl;
  cout << gf->card() << endl;
  
  vtkGridField *vtkgrid = toVTK(gf, attr.c_str());
  
  vtkRenderWindow *renWin = vtkRenderWindow::New();
//  Visualize(vtkgrid->GetOutput(), "", renWin);
  DirectVis(vtkgrid->GetOutput(), renWin);  
}

void Extract_Station(GridField *H, GridField *V) {

  //Construct the unit grid representing the user point.
  GridField *P = new GridField(new UnitGrid(), 0);
  Array *xarr = new Array("x", FLOAT);
  Array *yarr = new Array("y", FLOAT);
  Array *zarr = new Array("z", FLOAT);
  xarr->copyData(&x, 1);
  yarr->copyData(&y, 1);
  zarr->copyData(&z, 1);
  P->Bind(xarr);
  P->Bind(yarr);
  P->Bind(zarr);

  
  //Vertical Grid:
   
  // group node data onto 1-cells
  GridField *V1 = new GridField(V->grid, 1);
  Aggregate::mkvector pair("hilo");
  Assign::Nodes ends;
  GridFieldOperator *grpV1 = new AggregateOp::AggregateOp(V1, &ends, &pair, V);
  
  //find z in V
  Aggregate::count cnt;
  Assign::pointInRange pr("hilo", "z", "z");
  GridFieldOperator *zinV1 = new AggregateOp::AggregateOp(grpV1, &pr, &cnt, P);

  // restrict to the matching polygons
  GridFieldOperator *newV1 = new RestrictOp::RestrictOp("count>0", zinV1);

  
  GridFieldOperator *siftV1 = new SiftOp::SiftOp(newV1);
  
  // record ordinals
  GridFieldOperator *vpos 
    = new AccumulateOp::AccumulateOp(V, "vpos", "vpos+1", "0");
  
  // restrict to the matching polygons
  GridFieldOperator *newV = new MergeOp::MergeOp(vpos, siftV1);
  
  // record Ordinals
  GridFieldOperator *positions 
    = new AccumulateOp::AccumulateOp(H, "hpos", "hpos+1", "0");
  
  // get addresses
  stringstream ss;
  ss << "addr+" << V->card() << "-b";
  GridFieldOperator *columns 
      = new AccumulateOp::AccumulateOp(H, "addr", ss.str(), "0");
  
  // derive a rank 2 gridfield from H  
  GridField *H2 = new GridField(H->grid, 2);
    
  // group the node data and bind to the 2-cells
  Assign::Nodes nds;
  Aggregate::mkvector clump("poly");
  GridFieldOperator *mkclump = new AggregateOp::AggregateOp(H2, &nds, &clump, positions);
    
  // find P in H
  Assign::pointpoly3 pp;
  GridFieldOperator *probe = new AggregateOp::AggregateOp(mkclump, &pp, &cnt, P);
  
  // cut away the polygons where P was not found, likely leaving just 1 cell  
  GridFieldOperator *losepolys = new RestrictOp::RestrictOp("count>0", probe);
  
  if (losepolys->getResult()->card() == 0) {
    cerr << "The point (" << x << ", " << y << ") does not seem to be in the horizontal grid." << endl;
    exit(1);
  }
  if (newV1->getResult()->card() == 0) {
    cerr << "The point " << z << " does not seem to be in the vertical grid." << endl;
    exit(1);
  }
  
  // cut all the remaining nodes not incident to the found cell
  GridFieldOperator *sift = new SiftOp::SiftOp(losepolys);

  // we want the grid we have, but the data from H
  GridFieldOperator *intersect = new MergeOp::MergeOp(columns, sift);

  //View(intersect, "b");

  //3D grid
  //GridFieldOperator *nestVH = new SubApplyOp::SubApplyOp(newV, "H", columns);
  //nestVH->parameterize("a", bind, (ParameterAssigner::ParamFunc) &ApplyOp::setExpression);
  
  // build the 3d grid
  GridFieldOperator *cross = new CrossOp::CrossOp(intersect, newV);


  // cut away the portion underground
  GridFieldOperator *bottom = new RestrictOp::RestrictOp("vpos>b", cross);
  
  if (bottom->getResult()->card() == 0) {
    cerr << "Depth " << z << " seems to be below ground at point (" << x << ", " << y << ")." << endl;
    exit(1);
  }
    
  // compute the real addresses
  GridFieldOperator *varaddr 
      = new ApplyOp::ApplyOp(bottom, "varaddr", "addr+vpos");

  //read in the variable data
  ElcircFile ef = ElcircFile(filename);
  ArrayReader *varrdr = ef.getVariableReader(timestep, "varaddr");

  BindOp *bind = new BindOp::BindOp("var", FLOAT, varrdr, varaddr);

  //clump all the points on the 3d prism
 // GridFieldOperator *clump3D = new ChangeRankOp::ChangeRankOp(bind, 3);

  Aggregate::interpolate3D interp("var");
  GridFieldOperator *unify = new AggregateOp::AggregateOp(P, &nds, &interp, bind);

  GridField *T = ef.readTGrid();

  stringstream t;
  t << ef.getHeaderSize() <<"+((tstep/10)-1)*"<< ef.getTimestepSize();
  ApplyOp *appT = new ApplyOp::ApplyOp(T, "taddr", t.str());
  Type inttype = INT;
  appT->setResultType((UnTypedPtr *) &inttype);

  GridField *newT = appT->getResult();

  // just hack everything to stdout
  int taddr;
  int offset;
  GridField *varval;

  for (int i=0; i<newT->card(); i++) {
    taddr = *(int *)newT->getAttributeVal("taddr", i);
    offset = ef.getOffset("var", i);
    bind->setOffsetInt((UnTypedPtr) &offset);
    bind->Execute();
    unify->Execute();
    varval = unify->getResult();
    if (varval->card() > 0) {
      cout << *(float *)newT->getAttributeVal("tstamp", i) << ", ";
      //varval->print();
      cout << *(float *) varval->getAttributeVal("var", 0) << endl;
    }
  }
 /* 
  string innerattr("container");  
  SubApplyOp *nestTG = new SubApplyOp::SubApplyOp(appT, innerattr, unify);
  nestTG->parameterize("taddr", bind, (ParameterAssigner::ParamFunc) &BindOp::setOffset);
*/
  //View(nestTG, "taddr");
  /*
  GridField *gf = bind->getResult();
  vtkGridField *vtkgrid = toVTK(gf, "var");
    
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  //  Visualize(vtkgrid->GetOutput(), "", renWin);
  DirectVis(vtkgrid->GetOutput(), renWin);  
*/
} 

void showhelp() {
  cout 
    << "Usage: "
    << endl << "extstation -f <filename> "
    << endl << "-x <float> "
    << endl << "-y <float> "
    << endl << "-z <float> ";
}
