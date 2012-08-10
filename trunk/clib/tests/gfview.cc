#include "visualize.h"
#include "vtkGridField.h"
#include "expr.h"
#include "corierecipes.h"
#include "CmdLine.h"
#include "scaninternal.h"
#include <sstream>

//defaults
string filename;
string attribute;

using namespace GF;

void View(string fn);
void showhelp();
void checkopts( int argc, char **argv );

template <class T>
bool from_string(T &t, const std::string &s) {
  std::istringstream iss(s);
  return !(iss >> std::dec >> t).fail();
};

int main( int argc, char *argv[] ) {
  checkopts(argc, argv);
  View(filename);
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
        
        from_string<string>(attribute, cmdline.GetSafeArgument("-a", 0, "var"));
}


void View(string fn) {

  //GridFieldOperator *input = new ScanInternal::ScanInternal(fn+".gf", 0);
  ScanInternal *scan = new ScanInternal(fn, 0);
  GridField *gf = scan->getResult();

  // input->Execute();
  //GridField *gf = input->getResult();
  vtkGridField *vtkgrid = toVTK(gf, attribute);
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  //Visualize(vtkgrid->GetOutput(), bath->GetOutput(), "", renWin);
  DirectVis(vtkgrid->GetOutput(), renWin);
}


void showhelp() {
  cout 
    << "Usage: "
    << endl << "view -f <gridfield filename> "
    << endl << endl 
    << endl;
}
