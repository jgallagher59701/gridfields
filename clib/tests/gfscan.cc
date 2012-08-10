#include "gridfield.h"
#include "restrict.h"
#include "scaninternal.h"
#include "output.h"
#include "elcircfile.h"
#include "gfops.h"

string filename("1_salt.63");
string opid;

char gridLetter;

using namespace GF;

int main( int argc, char *argv[] ) {
  
  cout << "gfScan..." << flush;
  checkopts( argc, argv );
  
  ElcircFile *f = new ElcircFile(filename);
  GridField *result;

  switch (gridLetter) {
    case 'H':
      result = f->readHGrid();
      break;
    case 'V':
      result = f->readVGrid();
      break;
    case 'T':
      result = f->readTGrid();
      break;
    default:
      cout << "Invalid Grid Letter: " << gridLetter << endl;
      exit(1);
  }

  GridFieldOperator *output = 
       new OutputOp::OutputOp(filename+opid, 0, result);
  output->Execute();
  cout << "done." << endl;
  
}


void checkopts( int argc, char **argv )
{
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
        
        from_string<string>(opid, cmdline.GetSafeArgument("-i", 0, "_S"));
        from_string<char>(gridLetter, cmdline.GetSafeArgument("-g", 0, "H"));
}


void showhelp() {
  cout 
    << "Usage: "
    << endl << "gfrestrict -f <filename> [-p <predicate expression>]"
    << endl << "    where <predicate expression> is a boolean expression "
    << endl << "          involving attribute names that evaluates to 0 for"
    << endl << "          false and non-zero for true."

    << endl << endl 
    << "Ex: gfrestrict -f \"1.salt.63\" -p \"(x<30)&(y>10)\""
    << endl;
}
