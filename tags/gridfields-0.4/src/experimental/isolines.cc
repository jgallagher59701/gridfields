#include "include.h"
#include "isolines.h"

//defaults

string filename("1_salt.63");

float xmin=0;
float xmax=0;
float ymin=0;
float ymax=0;

int depth=0;
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
  
  GridField *H = readHGrid(filename.c_str());
  GridField *V = readVGrid(filename.c_str());
  HorizontalSlice(H, V);
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
        from_string<float>(xmin, cmdline.GetSafeArgument("-xmin", 0, "320000"));
        from_string<float>(xmax, cmdline.GetSafeArgument("-xmax", 0, "350000"));
        from_string<float>(ymin, cmdline.GetSafeArgument("-ymin", 0, "280000"));
        from_string<float>(ymax, cmdline.GetSafeArgument("-ymax", 0, "310000"));
        
        from_string<int>(depth, cmdline.GetSafeArgument("-d", 0, "59"));
        from_string<int>(timestep, cmdline.GetSafeArgument("-t", 0, "0"));
       /* 
        cout << filename 
          << ", " << xmin 
          << ", " << xmax 
          << ", " << ymin 
          << ", " << ymax
          << ", " << depth 
          << ", " << timestep 
          << endl;
          */
}


void HorizontalSlice(GridField *H, GridField *V) {

  float start = gettime();

  // find V-index from z value here
  int v = depth;
  stringstream vss;
  vss << v;
  string vstr = vss.str();
    
  float scale = 60;
   
  ElcircFile ef = ElcircFile(filename);
  
  stringstream zstr;
  float z = *(float *)V->getAttributeVal("z", v); //+ 4825.1;
  zstr << *(float *) V->getAttributeVal("z", v) << "*" << scale;
  
  H->recordOrdinals("hpos");
  
  //GridFieldOperator *accumaddr = 
  //      new AccumulateOp::AccumulateOp(H, "addr", ss, "b");
  
  //this one is the trick... 
  computeColumnPositions(H,V);
 
  //restrict to the specified region
  stringstream zoomexpr;
  zoomexpr << "(" << xmin << "<x)&(x<" << xmax << ")";
  zoomexpr << "&";
  zoomexpr << "(" << ymin << "<y)&(y<" << ymax << ")";
  GridFieldOperator *rzoom = 
        new RestrictOp::RestrictOp(zoomexpr.str(), H);

  //remove portions below ground  
  GridFieldOperator *rslice = 
        new RestrictOp::RestrictOp("b<"+vstr, rzoom);
  
  ArrayReader *surfrdr = ef.getSurfReader(timestep, "hpos");
  GridFieldOperator *bindsurf = 
        new BindOp::BindOp("surf", INT, surfrdr, rslice);
  
  //remove portions above surface
  GridFieldOperator *rsurf =
        new RestrictOp::RestrictOp("(surf+2)>"+vstr, bindsurf);

  //compute the positions of the variable data
  GridFieldOperator *appsliceaddr = 
        new ApplyOp::ApplyOp(rsurf, "sliceaddr", "addr+"+vstr+"-b", FLOAT);
  
  //bind the variable of interest 
  ArrayReader *varrdr = ef.getVariableReader(timestep, "sliceaddr");
  GridFieldOperator *bindvar = 
        new BindOp::BindOp("var", FLOAT, varrdr, appsliceaddr);
 
  //extend with z
  GridFieldOperator *appz = 
         new ApplyOp::ApplyOp(bindvar, "z", zstr.str(), FLOAT);

  GridFieldOperator *output = new OutputOp::OutputOp(filename+".gf", 0, appz);
  
  
  output->Execute();
  cout << "To visualize results, run " << endl;
  cout << "gfview -f " << filename +".gf" << endl;
//  GridField *gf = appz->getResult();
  
//  vtkGridField *vtkgrid = toVTK(gf, "var");
//  cout << gettime() - start << tab << "( Total )" << endl; 
    
  
  //isolines (unfilled)
  //vtkRenderWindow *renWin = vtkRenderWindow::New();
  //Visualize(vtkgrid->GetOutput(), bath->GetOutput(), "", renWin);
  //DirectVis(vtkgrid->GetOutput(), renWin);
}


void showhelp() {
  cout 
    << "Usage: "
    << endl << "isolines -f <filename> "
    << endl << "[-xmin <float>] "
    << endl << "[-xmax <float>] "
    << endl << "[-ymin <float>] "
    << endl << "[-ymax <float>] "
    << endl << "[-d <int>  (depth level) "
    << endl << "[-t <int>  (timestep) "

    << endl << endl 
    << "Typical defaults are provided for all but the filename."
    << endl;
}
