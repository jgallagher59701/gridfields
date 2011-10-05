#ifndef _BIND2OP_H
#define _BIND2OP_H

#include "catalog.h"
#include "cell.h"
#include "gridfieldoperator.h"

class Bind2Op : public UnaryGridFieldOperator {
public:
  Bind2Op(Catalog *catalog,
          std::string idx_attrs, 
          std::string attrs_to_bind, 
          Dim_t k,
          GridFieldOperator *GF,
          std::string addr="");
 
  void Execute();
  GridField *Bind(std::string idx_attrs, 
                  std::string attrs_to_bind, 
                  std::string addr, Dim_t k,
                  GridField *GF);
private:
  Dim_t _k;
  Catalog *catalog;
  std::string idx_str;
  std::string bind_str;
  std::string addr_attr;
};

#endif
