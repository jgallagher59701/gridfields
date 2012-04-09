
#include "catalog.h"
#include <sstream>
#include "elcircfile.h"
#include "arrayreader.h"
//#include "boost/filesystem/operations.hpp"

using namespace std;

Catalog::Catalog(string const &dir) 
                  : repository_dir(dir)
{
  var_ext_map["elev"] = make_pair("elev", 61);
  var_ext_map["salt"] = make_pair("salt", 63);
  var_ext_map["temp"] = make_pair("temp", 63);
  var_ext_map["hvel"] = make_pair("hvel", 64);
  var_ext_map["v"] = make_pair("hvel", 64);
  var_ext_map["u"] = make_pair("hvel", 64);


}

ArrayReader *Catalog::getArrayReader(vector<string> &access_vals, 
                                     string &gridname, string &attr) {
 

  if (access_vals.size() < 5 || access_vals.size() > 5) {
    Fatal("wrong number of index values for binding array.");
  }

  return getElcircArrayReader(atoi(access_vals[0].c_str()), 
                              atoi(access_vals[1].c_str()), 
                              access_vals[2],
                              atoi(access_vals[3].c_str()), 
                              atoi(access_vals[4].c_str()), 
                              attr); 
}

ArrayReader *Catalog::getElcircArrayReader(int yr, int wk, 
                                           string const &db, int dy, 
                                           int ts, string const &var)
{
  cout << yr << "-" << wk << "-" << db << endl;
  cout << dy << ", " << ts << ", " << var << endl;
  
  stringstream year;
  year << yr;
 
  stringstream week;
  if (wk < 10) 
    week << "0" << wk;
  else 
    week << wk;
 
  map<string, pair<string, int> >::iterator mi;
  mi = var_ext_map.find(var);

  int ext = 63;
  string variable(var);
  
  if (mi != var_ext_map.end()) {
    ext = mi->second.second;
    variable = mi->second.first;
  }
  
  stringstream path;
  path << repository_dir << "/";
  path << year.str() << "-" << week.str() << "-" <<  db << "/run/";
  path << dy  << "_" << variable << "." << ext;

  cout << path << endl;

  ElcircFile ef(path.str());
  ArrayReader *arr = ef.getVariableReader(var, ts, "");

  return arr;
}

/*


*/
