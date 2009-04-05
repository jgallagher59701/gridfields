#include "bind2.h"
#include "array.h"
#include "cell.h"
#include "arrayreader.h"
#include <string>
#include <sstream>

Bind2Op::Bind2Op( Catalog *cat, 
                  std::string idx_str, 
                  std::string bind_str, 
                  Dim_t k,
                  GridFieldOperator *previous,
                  std::string addr_attr)                   
        : UnaryGridFieldOperator(previous), _k(k),
          idx_str(idx_str), bind_str(bind_str), 
          addr_attr(addr_attr), 
          catalog(cat)
{
  //cleanup = false;
}
 
void Bind2Op::Execute() {
  this->PrepareForExecution();
  Result =  Bind2Op::Bind(idx_str, bind_str, addr_attr, _k, GF);
}



GridField *Bind2Op::Bind(std::string access_str, 
                         std::string bind_str, 
                         std::string addr_attr, Dim_t k,
                         GridField *GF) {
  
  vector<std::string> access_attrs;
  vector<std::string> attrs_to_bind;
  
  split(access_str, ";", access_attrs);  
  split(bind_str, ";", attrs_to_bind);

  if (attrs_to_bind.size() != 1) {
    Fatal("Binding multiple attributes simultaneously is not yet supported.");
  }

  vector<std::string> access_vals;

  std::string attri;
  std::stringstream ss;
  
  for (int i=0; i< access_attrs.size(); i++) {
    attri = access_attrs[i];
    //cout << "Bind: " << attri << endl;
    if (GF->IsAttribute(k, attri)) {
      ss << * (int *) GF->GetAttributeValue(k, attri, 0);
      access_vals.push_back(ss.str());
      ss.clear();
    } else {
      // constant rather than an attribute
      access_vals.push_back(attri);
    }
    //cout << "val: " << access_vals[i] << endl;
  }

  std::string attr0 = attrs_to_bind[0];
  
  ArrayReader *ar;
  ar = catalog->getArrayReader(access_vals, GF->GetGrid()->name, attr0);
  ar->setPatternAttribute(addr_attr);

  
  Array *arr;
  /*
  if (GF->isAttribute(attr0.c_str())) {
    arr = GF->getAttribute(attr0.c_str());
  } else {
    */
    arr = new Array(attrs_to_bind[0], FLOAT);
//  }
  
  ar->Read(GF, k, arr);

  GF->Bind(k, arr);
    arr->unref();
  delete ar;
  GF->ref();
  return GF;
}

