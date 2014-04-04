
#include "config_gridfields.h"

#include "netcdfadaptor.h"
#include "util.h"
#include "cellarray.h"
#include "array.h"
using namespace std;

namespace GF {

Type NetCDFAdaptor::mapType(NcType t) {Type nty=INT;
  switch (t) {
    case ncFloat:
      nty= FLOAT;
    case ncInt:
      nty= INT;
    default:
      cout<<t<<endl;
      Fatal("Only ncFloat and ncInt are supported.");
      break;
      exit(1);
      nty=INT;
  }
return nty;
}

NcType NetCDFAdaptor::mapType(Type t) {NcType nty=ncInt;
  switch (t) {
    case FLOAT:
      nty= ncFloat;
      break;
    case INT:
      nty= ncInt;
      break;
    default:
      Fatal("Unknown Type encountered during netCDF emission");
      break;
      exit(1);
      nty= ncInt;
  }
return nty;
}

bool NetCDFAdaptor::HasVar(NcFile *ncdf, const string varname) {
  for (int i=0; i<ncdf->num_vars(); i++) {
    NcVar *var = ncdf->get_var(i);
    if (string(var->name()) == varname) {
      return 1;
    }
  }
  return 0;
}

bool NetCDFAdaptor::HasDim(NcFile *ncdf, const string dimname) {
  NcDim *nodedim;
  int i=0;
  DEBUG << "_" << endl;
  for (; i<ncdf->num_dims(); i++) {
    nodedim = ncdf->get_dim(i);
    DEBUG << "|" << nodedim->name() << endl;
    if (string(nodedim->name()) == dimname) break;
  }
  if (i==ncdf->num_dims()) {
    return 0;
  } else {
    return 1;
  }
}

bool NetCDFAdaptor::HasAttr(NcFile *ncdf, const string attr) {
  DEBUG << "Has attr " << attr << "? ";
  for (int i=0; i<ncdf->num_atts(); i++) {
    NcAtt *a = ncdf->get_att(i);
    if (string(a->name()) == (attr)) {
      DEBUG << "yes" << endl;
      return 1;
    }
  }
  DEBUG << "no" << endl;
  return 0;
}

void NetCDFAdaptor::Open(string mode) {
  this->Close();
  const char *fn = filename.c_str();
  NcFile::FileMode m=NcFile::Replace;
  if (mode == "r") {
    m = NcFile::ReadOnly;
  } else if (mode == "w+") {
    m = NcFile::Write;
  } else if (mode == "w") {
    m = NcFile::Replace;
  } else {
    Fatal("Unknown file mode %s", mode.c_str());
    exit(1);
  }
  ncdf = new NcFile(fn, m);
  if (!ncdf->is_valid()) {
    Fatal("Error opening netcdf file");
  }
}

void NetCDFAdaptor::Close() {
  if (ncdf!=NULL) {
    delete ncdf;
    ncdf = NULL;
  } 
}

void NetCDFAdaptor::NodesFromDim(string ncdim, Grid *G) {
   NcDim *ncd = ncdf->get_dim(ncdim.c_str());
   G->setImplicit0Cells(ncd->size());
}

// Read two arrays from the file one relating (cells and nodes), and the other 
// relating (cells and edges) 
// Use the information to construct 2-cells and 1-cells
void NetCDFAdaptor::WellSupportedPolygonsFromVars(string cellnode, string celledge, Grid *G) {
   NcVar *nccellnode = ncdf->get_var(cellnode.c_str());
   if (nccellnode->num_dims() != 2) {
     Fatal("To build cells from a netcdf variable, the variable must have two dimensions (number_of_cells, cell_count)");
   }
   NcVar *nccelledge = ncdf->get_var(celledge.c_str());
   if (nccelledge->num_dims() != 2) {
     Fatal("To build cells from a netcdf variable, the variable must have two dimensions (number_of_cells, cell_count)");
   }

   // Polygons have the same number of nodes and edges
   NcDim *ncells = nccellnode->get_dim(0);
   if (ncells == NULL) {
     Fatal("Error retrieving number of cells");
   }

   // Polygons have the same number of nodes and edges
   NcDim *nnodes = nccellnode->get_dim(1);
   if (nnodes == NULL) {
     Fatal("Error retrieving number of nodes per cell");
   }

   CellArray *twocells = new CellArray();
   CellArray *onecells = new CellArray();

   map<int, Cell> edgemap;

   Node *nodes = new Node[nnodes->size()+1];
   Node *edges = new Node[nnodes->size()];
   Cell edge(2);
   for (unsigned int i=0; i<(unsigned)ncells->size(); i++) {
     // add the cell
     nccellnode->get((int *) nodes, 1, nnodes->size());

     // complete the cycle: set last + 1 = first
     nodes[nnodes->size()] = nodes[0];

     // add the 2-cell as a list of nodes
     twocells->addCellNodes(nodes, nnodes->size());

     // gather the edges
     nccelledge->get((int *) edges, 1, nnodes->size());
     for (unsigned int j=0; j<(unsigned)nnodes->size(); j++) {
       // set the nodes array of the placeholder edge
       edge.setnodes(nodes+j);
       // insert the edge into the map
       //cout << edges[j] << ": " << edge.getnodes()[0] << ", " << edge.getnodes()[1]  << endl;
       edgemap[edges[j]] = edge;
     }
     // set the iterators
     nccellnode->set_cur(i+1,0);
     nccelledge->set_cur(i+1,0);
   }
 
   // insert the edges in sorted order
   for (unsigned int i=0; i<edgemap.size(); i++) {
     onecells->addCell(edgemap[i]);
   }

   twocells->cleanup_node_array = true;
   onecells->cleanup_node_array = true;
   // assumes contiguous nodes 
   // assumes cells span nodes
   G->setKCells(twocells, 2);
   G->setKCells(onecells, 1);
   delete nodes;
   delete edges;
}

void NetCDFAdaptor::HomogeneousCellsFromVar(Dim_t d, string ncvar, Grid *G) {
   NcVar *ncv = ncdf->get_var(ncvar.c_str());
   if (ncv->num_dims() != 2) {
     Fatal("To build cells from a netcdf variable, the variable must have two dimensions (number_of_cells, cell_count");
   }

   NcDim *d0 = ncv->get_dim(0);
   if (d0 == NULL) {
     Fatal("Error retrieving first dimension");
   }
 
   NcDim *d1 = ncv->get_dim(1);
   if (d1 == NULL) {
     Fatal("Error retrieving second dimension");
   }
  
   CellArray *cells = new CellArray();
   Node *nodes = new Node[d1->size()];
   for (unsigned int i=0; i<(unsigned)d0->size(); i++) {
     ncv->get((int *) nodes, 1, d1->size());
     cells->addCellNodes(nodes, d1->size());
     ncv->set_cur(i+1,0);
   } 
   cells->cleanup_node_array = true;
   // assumes contiguous nodes 
   // assumes cells span nodes
   G->setKCells(cells, d);
   delete nodes;
}

void NetCDFAdaptor::AttributeFromVar(Dim_t d, string ncvar, GridField *G) {
  NcVar *ncv = ncdf->get_var(ncvar.c_str());
  if (ncv == NULL) {
    Fatal("Error reading variable %s", ncvar.c_str());
  }

  Array *arr = new Array(ncv->name(), mapType(ncv->type()));
  arr->setVals((UnTypedPtr) ncv->values()->base(), ncv->num_vals());

  G->Bind(d, arr);
}


void NetCDFAdaptor::CreateDim(const string &name, unsigned int size) {
  this->ncdf->add_dim(name.c_str(), size);
}

void NetCDFAdaptor::DimFromDim(const string &name, GridField *gf, Dim_t d) {
  this->ncdf->add_dim(name.c_str(), gf->Card(d));
}

void NetCDFAdaptor::VarFromAttribute(const string &name, GridField *gf, Dim_t d, const vector<string> &dims) {
  const NcDim** ncdims = new const NcDim*[dims.size()];
  for (unsigned int i=0; i<dims.size(); i++) {
    ncdims[i] = ncdf->get_dim(dims[i].c_str());
  }
  Array *arr = gf->GetAttribute(d, name);
  NcVar *var = ncdf->add_var(name.c_str(), mapType(arr->gettype()), dims.size(),ncdims);
  switch (arr->gettype()) {
    case INT:
      var->put((int *) arr->getVals(), var->edges());
      break;
    case FLOAT:
      var->put((float *) arr->getVals(),var->edges());
      break;
    case OBJ:
      Fatal("Only floats and ints currently supported.");
      break;
    case TUPLE:
      Fatal("Only floats and ints currently supported.");
      break;
    case GRIDFIELD:
      Fatal("Only floats and ints currently supported.");
      break;
  }
  delete [] ncdims;
}

void NetCDFAdaptor::VarFromIncidence(const string &name, GridField *gf, Dim_t c, Dim_t d, string dim1, string dim2) {
  NcDim *d1 = ncdf->get_dim(dim1.c_str());
  NcDim *d2 = ncdf->get_dim(dim2.c_str());
  NcVar *var = ncdf->add_var(name.c_str(), ncInt, d1, d2);

  unsigned int cols = d2->size();

  int *ids = new int[var->num_vals()];

  Grid *G = gf->GetGrid();
  vector<CellId> out;
  //_OutputIterator<CellId> out;
  for (unsigned int i=0; i<gf->Card(c); i++) {
    //out = (OutputIterator<CellId>) &ids[i*col];
    G->IncidentTo(i, c, out, d);
    for (unsigned int j=0; j<out.size(); j++) {
      // Output CellIds not Cells.  "Automatic" normalization.
      ids[i*cols + j] = out[j];
    }
    out.clear();
  }


  var->put(ids, var->edges());
  delete [] ids;
}

} // namespace GF

