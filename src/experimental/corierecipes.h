#ifndef RECIPES_H
#define RECIPES_H

#include "type.h"
#include <string>

class vtkGridField;
class GridField;
class Array;

void computeColumnPositions(GridField *H,GridField*V); 
// Scan(addr=addr[-1]+b) 

vtkGridField *makeBathymetry(GridField *cutH, float scale);
// Client

void Zoom(GridField *&H, GridField *&V, std::string _region);
// Restrict(x) . Restrict(y) . Restrict(z)

void scaleZ(GridField *Gg, float scale);
// Apply(z=z*scale)

GridField *makeWetGrid(GridField *H, GridField *V);
// Restrict(b>z) . Cross(H,V)

GridField *cutSurface(GridField *Gg);
// Restrict(salt>-99)

void computeAddresses(GridField *wetgrid);
// Apply(addr + (zpos-b))

GridField *makeWetSlice(GridField *H, int depth);
//Restrict(b>depth)

vtkGridField *toVTK(GridField *G, std::string active);

void readArray(GridField *G, 
               int k, 
               std::string fn, 
               Type t,
               int offset, 
               std::string attrtobind,
               std::string addresses);
//Bind(attr)

GridField *readCleanStations(std::string fn, int size);
GridField *readStations(std::string fn, std::string attr);
//Bind(tup<id, x, y>);

void writeArray(GridField *Gg, string fn, int offset, string attr);
//IO operation, not included in the formal model

void readTextArray(GridField *G, 
               int k, 
               std::string fn, 
               Type t,
               int offset, 
               std::string attrtobind,
               std::string addresses);
//Bind(attr)


#endif
