#include "gridfield.h"
#include "restrict.h"
#include "scaninternal.h"
#include "output.h"
#include "gfops.h"

string filename("1_salt.63");
string opid;
string predicate;

int main( int argc, char *argv[] ) {
  
  cout << "gfRestrict..." << flush;
  checkopts( argc, argv );
  GridField *gf = ScanInternal::Scan(filename, 0);
  GridFieldOperator *r = 
        new RestrictOp::RestrictOp(predicate, gf);

  GridFieldOperator *output = new OutputOp::OutputOp(filename+opid, 0, r);
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
          predicate = cmdline.GetArgument("-p", 0);
        } catch (...) {
          showhelp();
          exit(-1);
        }
        
        from_string<string>(opid, cmdline.GetSafeArgument("-i", 0, "_R"));
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
