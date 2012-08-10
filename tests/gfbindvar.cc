#include "gridfield.h"
#include "bind.h"
#include "scaninternal.h"
#include "output.h"
#include "elcircfile.h"
#include "gfops.h"

string filename("1_salt.63");
string opid;
string attr;
string posattr;
string sourcefilename;
int timestep;

using namespace GF;

int main( int argc, char *argv[] ) {
  
  cout << "gfBind..." << flush;
  checkopts( argc, argv );
  GridField *gf = ScanInternal::Scan(filename, 0);
  ElcircFile *f = new ElcircFile(sourcefilename);
  ArrayReader *ar = f->getVariableReader(timestep, posattr);
  GridFieldOperator *b = 
        new BindOp::BindOp(attr, FLOAT, ar, gf);

  GridFieldOperator *output = new OutputOp::OutputOp(filename+opid, 0, b);
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
        
        from_string<string>(opid, cmdline.GetSafeArgument("-i", 0, "_B"));
        from_string<string>(attr, cmdline.GetSafeArgument("-a", 0, "var"));
        from_string<string>(posattr, cmdline.GetSafeArgument("-p", 0, ""));
        from_string<int>(timestep, cmdline.GetSafeArgument("-t", 0, "0"));
        from_string<string>(sourcefilename, cmdline.GetSafeArgument("-s", 0, filename.c_str()));
}

void showhelp() {
  cout 
    << "Usage: "
    << endl << "gfbind -f <previous output filename> [-a <attribute name>]"
    << endl << "                                     [-p <position attribute>]"
    << endl << "                                     [-t <timestep>]"
    << endl << "                                     [-s <source filename>]"

    << endl << endl 
    << "Ex: gfbind -f 1_salt.63_S_A -a salt -p addr -t 34 -s 2_salt.63"
    << endl;
}
