#ifndef _CATALOG_H
#define _CATALOG_H

#include <vector>
#include <string>
#include <map>

class ArrayReader;

class Catalog {
public:

 Catalog(std::string const &directory="/home/workspace/ccalmr/hindcasts/");
 ArrayReader *getArrayReader(std::vector<std::string> &access_vals, 
                             std::string &gridname, 
                             std::string &attr);
  
 ArrayReader *getElcircArrayReader(int yr, int wk, std::string const &db, 
                                   int dy, int ts, std::string const &var);


 private:
   std::string repository_dir;
   std::map<std::string, std::pair<std::string, int> > var_ext_map;
};

#endif
